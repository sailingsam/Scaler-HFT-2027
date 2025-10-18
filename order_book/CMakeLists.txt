cmake_minimum_required(VERSION 3.16)
project(LowLatencyOrderBook)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set optimization flags for performance
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

# Set default build type to Release for performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create the main library
add_library(order_book_lib
    order_book.cpp
)

# Set compile options for the library
target_compile_options(order_book_lib PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native>
    $<$<CONFIG:Debug>:-g -O0>
)

# Create the test executable
add_executable(test_order_book
    test_order_book.cpp
)

# Link the library to the test
target_link_libraries(test_order_book order_book_lib)

# Create the example executable
add_executable(example
    example.cpp
)

# Link the library to the example
target_link_libraries(example order_book_lib)

# Enable testing
enable_testing()

# Add test
add_test(NAME OrderBookTests COMMAND test_order_book)

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")

# Performance optimization settings
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release build - optimizations enabled")
    target_compile_definitions(order_book_lib PRIVATE NDEBUG)
endif()

# Add custom targets for convenience
add_custom_target(run_tests
    COMMAND test_order_book
    DEPENDS test_order_book
    COMMENT "Running Order Book tests"
)

add_custom_target(run_example
    COMMAND example
    DEPENDS example
    COMMENT "Running Order Book example"
)

# Add install targets
install(TARGETS order_book_lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(TARGETS test_order_book example
    RUNTIME DESTINATION bin
)

# Print usage information
message(STATUS "")
message(STATUS "=== Low-Latency Order Book Build Configuration ===")
message(STATUS "To build: cmake . && make")
message(STATUS "To run tests: make run_tests")
message(STATUS "To run example: make run_example")
message(STATUS "To install: make install")
message(STATUS "")
