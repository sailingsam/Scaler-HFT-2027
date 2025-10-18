# Makefile for Low-Latency Order Book
# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -march=native -DNDEBUG
DEBUGFLAGS = -std=c++17 -Wall -Wextra -g -O0 -DDEBUG

# Source files
SOURCES = order_book.cpp
HEADERS = order_book.h

# Targets
TARGETS = test_order_book example

# Default target
all: $(TARGETS)

# Test executable
test_order_book: test_order_book.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o test_order_book test_order_book.cpp $(SOURCES)

# Example executable  
example: example.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o example example.cpp $(SOURCES)

# Debug versions
debug_test: test_order_book.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(DEBUGFLAGS) -o test_order_book_debug test_order_book.cpp $(SOURCES)

debug_example: example.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(DEBUGFLAGS) -o example_debug example.cpp $(SOURCES)

# Run tests
test: test_order_book
	./test_order_book

# Run example
run: example
	./example

# Clean
clean:
	rm -f $(TARGETS) *_debug

# Install (if needed)
install: $(TARGETS)
	@echo "Installing executables..."
	@mkdir -p bin
	cp test_order_book example bin/

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build all executables (default)"
	@echo "  test_order_book - Build test executable"
	@echo "  example      - Build example executable"
	@echo "  debug_test   - Build debug version of test"
	@echo "  debug_example - Build debug version of example"
	@echo "  test         - Build and run tests"
	@echo "  run          - Build and run example"
	@echo "  clean        - Remove all executables"
	@echo "  install      - Install to bin/ directory"
	@echo "  help         - Show this help"

.PHONY: all test run clean install help debug_test debug_example
