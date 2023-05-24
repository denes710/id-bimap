[![Build Status](https://github.com/denes710/id-bimap/actions/workflows/tests.yml/badge.svg)](https://github.com/denes710/id-bimap/actions/workflows/tests.yml)

# C++ Bidirectional Map Library
This C++ library provides an implementation of a two-way associative data structure, also known as a bidirectional map. It is designed as a template class following the principles of the C++ Standard Template Library (STL). The data structure allows for efficient mapping between keys and values, supporting both forward and reverse lookups. This library was developed as part of the "Modern C++" course at ELTE university.

## Key Requirements
- The key type must be incrementable (++) and support comparison and sorting (==, !=, <).
- The key type is expected to be of integer type. A compile-time error will be generated if any other types, such as floats, are used as keys.
- The key type parameter has a default value. If not explicitly specified by the user, the library will default to using the platform's default indexing type.

## Value Type
The mapped value type can be any user-defined type. Similar to the key type, it should support comparison and sorting. However, it is essential to note that the key and value types must be distinct and cannot be the same.

## Memory Usage
The bidirectional map optimizes memory usage by storing values only one time. To implement the bidirectional mapping, references are utilized, resulting in an efficient utilization of memory resources.

## Running the tests
```bash
mkdir build
cd build
cmake ..
cmake --build .
ctest
```
