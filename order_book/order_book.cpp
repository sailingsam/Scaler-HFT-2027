#include "order_book.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>

void OrderBook::add_order(const Order &order) {
  // Allocate order from memory pool
  Order *order_ptr = order_pool.allocate();
  *order_ptr = order;

  // Add to lookup table for O(1) access
  order_lookup[order.order_id] = order_ptr;

  // Add to appropriate side of the book
  if (order.is_buy) {
    bids[order.price].push(order_ptr);
  } else {
    asks[order.price].push(order_ptr);
  }
}

bool OrderBook::cancel_order(uint64_t order_id) {
  auto it = order_lookup.find(order_id);
  if (it == order_lookup.end()) {
    return false; // Order not found
  }

  Order *order = it->second;
  double price = order->price;
  bool is_buy = order->is_buy;

  // Remove from price level
  remove_order_from_level(order, price, is_buy);

  // Remove from lookup table
  order_lookup.erase(it);

  // Return to memory pool
  order_pool.deallocate(order);

  return true;
}

bool OrderBook::amend_order(uint64_t order_id, double new_price,
                            uint64_t new_quantity) {
  auto it = order_lookup.find(order_id);
  if (it == order_lookup.end()) {
    return false; // Order not found
  }

  Order *order = it->second;
  double old_price = order->price;
  bool is_buy = order->is_buy;

  // If price changed, treat as cancel + add
  if (std::abs(old_price - new_price) > 1e-9) {
    // Remove from old price level
    remove_order_from_level(order, old_price, is_buy);

    // Update order details
    order->price = new_price;
    order->quantity = new_quantity;

    // Add to new price level
    add_order_to_level(order, new_price, is_buy);
  } else {
    // Only quantity changed, update in place
    order->quantity = new_quantity;
  }

  return true;
}

void OrderBook::get_snapshot(size_t depth, std::vector<PriceLevel> &bid_levels,
                             std::vector<PriceLevel> &ask_levels) const {
  bid_levels.clear();
  ask_levels.clear();

  // Get top N bid levels (highest prices first)
  size_t bid_count = 0;
  for (const auto &pair : bids) {
    double price = pair.first;
    const auto &orders = pair.second;
    if (bid_count >= depth)
      break;

    uint64_t total_quantity = 0;
    std::queue<Order *> temp_queue = orders;
    while (!temp_queue.empty()) {
      total_quantity += temp_queue.front()->quantity;
      temp_queue.pop();
    }

    if (total_quantity > 0) {
      bid_levels.emplace_back(price, total_quantity);
      bid_count++;
    }
  }

  // Get top N ask levels (lowest prices first)
  size_t ask_count = 0;
  for (const auto &pair : asks) {
    double price = pair.first;
    const auto &orders = pair.second;
    if (ask_count >= depth)
      break;

    uint64_t total_quantity = 0;
    std::queue<Order *> temp_queue = orders;
    while (!temp_queue.empty()) {
      total_quantity += temp_queue.front()->quantity;
      temp_queue.pop();
    }

    if (total_quantity > 0) {
      ask_levels.emplace_back(price, total_quantity);
      ask_count++;
    }
  }
}

void OrderBook::print_book(size_t depth) const {
  std::cout << "\n=== ORDER BOOK SNAPSHOT (Top " << depth << " levels) ===\n";
  std::cout << std::setw(12) << "Price" << std::setw(15) << "Quantity"
            << std::setw(10) << "Side\n";
  std::cout << std::string(40, '-') << "\n";

  // Print asks (sell orders) - lowest price first
  std::vector<PriceLevel> ask_levels;
  size_t ask_count = 0;
  for (const auto &pair : asks) {
    double price = pair.first;
    const auto &orders = pair.second;
    if (ask_count >= depth)
      break;

    uint64_t total_quantity = 0;
    std::queue<Order *> temp_queue = orders;
    while (!temp_queue.empty()) {
      total_quantity += temp_queue.front()->quantity;
      temp_queue.pop();
    }

    if (total_quantity > 0) {
      ask_levels.emplace_back(price, total_quantity);
      ask_count++;
    }
  }

  // Print asks in reverse order (highest ask first)
  for (auto it = ask_levels.rbegin(); it != ask_levels.rend(); ++it) {
    std::cout << std::setw(12) << std::fixed << std::setprecision(2)
              << it->price << std::setw(15) << it->total_quantity
              << std::setw(10) << "ASK\n";
  }

  std::cout << std::string(40, '-') << "\n";

  // Print bids (buy orders) - highest price first
  size_t bid_count = 0;
  for (const auto &pair : bids) {
    double price = pair.first;
    const auto &orders = pair.second;
    if (bid_count >= depth)
      break;

    uint64_t total_quantity = 0;
    std::queue<Order *> temp_queue = orders;
    while (!temp_queue.empty()) {
      total_quantity += temp_queue.front()->quantity;
      temp_queue.pop();
    }

    if (total_quantity > 0) {
      std::cout << std::setw(12) << std::fixed << std::setprecision(2) << price
                << std::setw(15) << total_quantity << std::setw(10) << "BID\n";
      bid_count++;
    }
  }

  std::cout << std::string(40, '-') << "\n\n";
}

void OrderBook::process_matching() {
  while (!bids.empty() && !asks.empty()) {
    auto best_bid_it = bids.begin();
    auto best_ask_it = asks.begin();

    double best_bid_price = best_bid_it->first;
    double best_ask_price = best_ask_it->first;

    if (best_bid_price < best_ask_price) {
      break; // No crossing
    }

    // Get the orders at best prices
    auto &bid_orders = best_bid_it->second;
    auto &ask_orders = best_ask_it->second;

    if (bid_orders.empty() || ask_orders.empty()) {
      break;
    }

    Order *bid_order = bid_orders.front();
    Order *ask_order = ask_orders.front();

    uint64_t match_quantity =
        std::min(bid_order->quantity, ask_order->quantity);

    // Execute the match
    bid_order->quantity -= match_quantity;
    ask_order->quantity -= match_quantity;

    std::cout << "MATCH: " << match_quantity << " @ " << best_bid_price << "\n";

    // Remove fully filled orders
    if (bid_order->quantity == 0) {
      bid_orders.pop();
      order_lookup.erase(bid_order->order_id);
      order_pool.deallocate(bid_order);
    }

    if (ask_order->quantity == 0) {
      ask_orders.pop();
      order_lookup.erase(ask_order->order_id);
      order_pool.deallocate(ask_order);
    }

    // Remove empty price levels
    if (bid_orders.empty()) {
      bids.erase(best_bid_it);
    }
    if (ask_orders.empty()) {
      asks.erase(best_ask_it);
    }
  }
}

bool OrderBook::is_empty() const { return bids.empty() && asks.empty(); }

double OrderBook::get_best_bid() const {
  if (bids.empty())
    return 0.0;
  return bids.begin()->first;
}

double OrderBook::get_best_ask() const {
  if (asks.empty())
    return 0.0;
  return asks.begin()->first;
}

// Private helper methods
void OrderBook::remove_order_from_level(Order *order, double price,
                                        bool is_buy) {
  if (is_buy) {
    auto it = bids.find(price);
    if (it != bids.end()) {
      std::queue<Order *> temp_queue;
      while (!it->second.empty()) {
        Order *front_order = it->second.front();
        it->second.pop();
        if (front_order != order) {
          temp_queue.push(front_order);
        }
      }
      it->second = temp_queue;

      // Remove empty price level
      if (it->second.empty()) {
        bids.erase(it);
      }
    }
  } else {
    auto it = asks.find(price);
    if (it != asks.end()) {
      std::queue<Order *> temp_queue;
      while (!it->second.empty()) {
        Order *front_order = it->second.front();
        it->second.pop();
        if (front_order != order) {
          temp_queue.push(front_order);
        }
      }
      it->second = temp_queue;

      // Remove empty price level
      if (it->second.empty()) {
        asks.erase(it);
      }
    }
  }
}

void OrderBook::add_order_to_level(Order *order, double price, bool is_buy) {
  if (is_buy) {
    bids[price].push(order);
  } else {
    asks[price].push(order);
  }
}

void OrderBook::update_price_level_quantity(double /*price*/, bool /*is_buy*/,
                                            int64_t /*quantity_delta*/) {
  // This method is kept for potential future use
  // Currently not used as we maintain FIFO queues
}
