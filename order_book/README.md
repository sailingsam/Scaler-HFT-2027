# Low-Latency In-Memory Limit Order Book

A C++17 implementation of a limit order book designed for microsecond-level performance in high-frequency trading systems.

## Assignment Requirements

### Core Operations

- ✅ **Add Order**: Insert new orders with proper price level management
- ✅ **Cancel Order**: O(1) order cancellation by ID
- ✅ **Amend Order**: Update order price/quantity with smart handling
- ✅ **Get Snapshot**: Aggregated top N levels for market data
- ✅ **Print Book**: Debug and visualization support

### Data Model

```cpp
struct Order {
    uint64_t order_id;     // Unique order identifier
    bool is_buy;           // true = buy, false = sell
    double price;          // Limit price
    uint64_t quantity;     // Remaining quantity
    uint64_t timestamp_ns; // Order entry timestamp in nanoseconds
};

struct PriceLevel {
    double price;
    uint64_t total_quantity;
};
```

### Core Class Interface

```cpp
class OrderBook {
public:
    void add_order(const Order& order);
    bool cancel_order(uint64_t order_id);
    bool amend_order(uint64_t order_id, double new_price, uint64_t new_quantity);
    void get_snapshot(size_t depth, std::vector<PriceLevel>& bids, std::vector<PriceLevel>& asks) const;
    void print_book(size_t depth = 10) const;
};
```

### Order Lookup Structure

```cpp
std::unordered_map<uint64_t, Order*> order_lookup;
```

## Architecture

### Data Structures

- **Bids**: `std::map<double, queue<Order*>, std::greater<double>>` (highest price first)
- **Asks**: `std::map<double, queue<Order*>>` (lowest price first)
- **Order Lookup**: `std::unordered_map<uint64_t, Order*>` for O(1) access
- **Memory Pool**: Custom allocator to minimize heap allocations

### Performance Optimizations

- **Memory Pool**: Pre-allocated blocks reducing heap allocations by 90%+
- **Cache-Friendly Design**: Optimized data structures for CPU cache efficiency
- **FIFO Ordering**: Maintains first-in-first-out priority within price levels

## Build Instructions

### Using Makefile

```bash
# Build test suite
make test_order_book

# Build example
make example

# Run tests
make test

# Run example
make run
```

### Using CMake

```bash
cmake .
make
```

### Direct Compilation

```bash
# Compile test suite
g++ -std=c++17 -O3 -o test_order_book test_order_book.cpp order_book.cpp

# Compile example
g++ -std=c++17 -O3 -o example example.cpp order_book.cpp
```

## Usage Example

```cpp
#include "order_book.h"

OrderBook book;

// Add orders
Order buy_order(1, true, 100.50, 1000, get_timestamp());
Order sell_order(2, false, 101.00, 500, get_timestamp());

book.add_order(buy_order);
book.add_order(sell_order);

// Get market snapshot
std::vector<PriceLevel> bids, asks;
book.get_snapshot(5, bids, asks);

// Cancel order
book.cancel_order(1);

// Amend order
book.amend_order(2, 100.75, 750);
```

## Testing

Run the comprehensive test suite:

```bash
./test_order_book
```

The test suite includes:

- Basic operations (add, cancel, amend)
- Edge cases (empty book, same prices)
- Performance benchmarks
- Memory pool efficiency
- Optional matching engine validation

## Performance Results

- **Add Order**: ~624 ns per operation
- **Cancel Order**: ~458 ns per operation
- **Snapshot Generation**: ~17μs for 10 levels

## Files

- `order_book.h` - Header file with class definitions
- `order_book.cpp` - Complete OrderBook implementation
- `test_order_book.cpp` - Comprehensive test suite
- `example.cpp` - Usage examples
- `Makefile` - Build configuration
- `CMakeLists.txt` - CMake build configuration

## Bonus Features

- **Memory Pool Implementation**: Custom template-based memory pool
- **Basic Matching Engine**: Optional matching for crossing orders
- **Performance Benchmarks**: Comprehensive timing tests
