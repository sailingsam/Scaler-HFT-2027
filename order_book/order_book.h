#pragma once
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

struct Order {
  uint64_t order_id;     // Unique order identifier
  bool is_buy;           // true = buy, false = sell
  double price;          // Limit price
  uint64_t quantity;     // Remaining quantity
  uint64_t timestamp_ns; // Order entry timestamp in nanoseconds

  // Default constructor for memory pool
  Order()
      : order_id(0), is_buy(false), price(0.0), quantity(0), timestamp_ns(0) {}

  Order(uint64_t id, bool buy, double p, uint64_t qty, uint64_t ts)
      : order_id(id), is_buy(buy), price(p), quantity(qty), timestamp_ns(ts) {}
};

struct PriceLevel {
  double price;
  uint64_t total_quantity;

  PriceLevel(double p, uint64_t qty) : price(p), total_quantity(qty) {}
};

// Memory pool for efficient order allocation
template <typename T> class MemoryPool {
private:
  std::vector<std::unique_ptr<T[]>> blocks;
  std::queue<T *> free_objects;
  size_t block_size;
  size_t current_block;
  size_t current_index;

public:
  MemoryPool(size_t block_sz = 1000)
      : block_size(block_sz), current_block(0), current_index(0) {
    allocate_block();
  }

  T *allocate() {
    if (free_objects.empty()) {
      if (current_index >= block_size) {
        allocate_block();
      }
      return &blocks[current_block][current_index++];
    }

    T *obj = free_objects.front();
    free_objects.pop();
    return obj;
  }

  void deallocate(T *obj) { free_objects.push(obj); }

private:
  void allocate_block() {
    blocks.emplace_back(std::make_unique<T[]>(block_size));
    current_block = blocks.size() - 1;
    current_index = 0;
  }
};

class OrderBook {
private:
  // Price level data structures
  std::map<double, std::queue<Order *>, std::greater<double>>
      bids;                                   // Highest price first
  std::map<double, std::queue<Order *>> asks; // Lowest price first

  // Order lookup for O(1) cancel/amend
  std::unordered_map<uint64_t, Order *> order_lookup;

  // Memory pool for efficient allocation
  MemoryPool<Order> order_pool;

  // Helper methods
  void remove_order_from_level(Order *order, double price, bool is_buy);
  void add_order_to_level(Order *order, double price, bool is_buy);
  void update_price_level_quantity(double price, bool is_buy,
                                   int64_t quantity_delta);

public:
  OrderBook() = default;
  ~OrderBook() = default;

  // Insert a new order into the book
  void add_order(const Order &order);

  // Cancel an existing order by its ID
  bool cancel_order(uint64_t order_id);

  // Amend an existing order's price or quantity
  bool amend_order(uint64_t order_id, double new_price, uint64_t new_quantity);

  // Get a snapshot of top N bid and ask levels (aggregated quantities)
  void get_snapshot(size_t depth, std::vector<PriceLevel> &bids,
                    std::vector<PriceLevel> &asks) const;

  // Print current state of the order book
  void print_book(size_t depth = 10) const;

  // Optional: Basic matching implementation
  void process_matching();

  // Utility methods
  bool is_empty() const;
  double get_best_bid() const;
  double get_best_ask() const;
};
