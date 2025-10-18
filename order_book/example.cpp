#include "order_book.h"
#include <chrono>
#include <iostream>

int main() {
  std::cout << "=== Order Book Example Usage ===\n\n";

  OrderBook book;

  // Create some sample orders
  auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
                 std::chrono::high_resolution_clock::now().time_since_epoch())
                 .count();

  // Add buy orders (bids)
  Order bid1(1, true, 100.50, 1000, now);
  Order bid2(2, true, 100.25, 500, now + 1);
  Order bid3(3, true, 100.75, 750, now + 2);

  // Add sell orders (asks)
  Order ask1(4, false, 101.00, 600, now + 3);
  Order ask2(5, false, 101.25, 300, now + 4);
  Order ask3(6, false, 100.90, 400, now + 5);

  std::cout << "Adding orders to the book...\n";
  book.add_order(bid1);
  book.add_order(bid2);
  book.add_order(bid3);
  book.add_order(ask1);
  book.add_order(ask2);
  book.add_order(ask3);

  // Display the order book
  book.print_book(5);

  // Get a snapshot
  std::vector<PriceLevel> bids, asks;
  book.get_snapshot(3, bids, asks);

  std::cout << "Top 3 Bid Levels:\n";
  for (const auto &level : bids) {
    std::cout << "  $" << level.price << " - " << level.total_quantity
              << " shares\n";
  }

  std::cout << "\nTop 3 Ask Levels:\n";
  for (const auto &level : asks) {
    std::cout << "  $" << level.price << " - " << level.total_quantity
              << " shares\n";
  }

  // Demonstrate order cancellation
  std::cout << "\nCancelling order #2 (100.25 bid)...\n";
  book.cancel_order(2);
  book.print_book(5);

  // Demonstrate order amendment
  std::cout
      << "\nAmending order #3 (100.75 -> 100.80, quantity 750 -> 1000)...\n";
  book.amend_order(3, 100.80, 1000);
  book.print_book(5);

  // Add a crossing order to demonstrate matching
  std::cout << "\nAdding crossing order (buy at 101.10)...\n";
  Order crossing_bid(7, true, 101.10, 200, now + 6);
  book.add_order(crossing_bid);

  std::cout << "Before matching:\n";
  book.print_book(5);

  std::cout << "Processing matches...\n";
  book.process_matching();

  std::cout << "After matching:\n";
  book.print_book(5);

  // Show best bid/ask
  std::cout << "Current best bid: $" << book.get_best_bid() << "\n";
  std::cout << "Current best ask: $" << book.get_best_ask() << "\n";
  std::cout << "Spread: $" << (book.get_best_ask() - book.get_best_bid())
            << "\n";

  return 0;
}
