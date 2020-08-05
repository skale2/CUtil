# CUtil

A collection of C utilities for high level, generic, and type-safe programming in C. This library is aimed at developers comfortable working in highly-abstracted C, rather than use-cases like those required by embedded system developers, who I'm sure would rather spend their time hardcoding a 4KB array because it's "better for the cache policy".

This library relies on a few compiler-specific extensions, including variadic macros, designated initializers, statement expresssions, and the `typeof` operator. For now, it's recommended that compilation is only done on GCC or Clang.

## Features

[Finished]
- **debug.c**: a a collection of functions and macros for debugging code, including type-generic prints, memory dumps, heap allocation tracking, and macro debugging.
- **functions.h**: some helpful functions and macros to allocate data types on the heap and assert type constraints, as well as some function signatures and implementations of useful behavior.
- **panic.h**: a runtime exception generator and handler. Uses long jumps to throw "panics" with helpful meta data, and macros to catch those panics during stack unrolling.
- **testing.h**: a native-C testing utility for creating test suites. Includes macros for creating tests and test suites, reading command line options, assert conditions in tests, and creating random memory buffer.
- **test_runner.c**: an independent utility for running tests created with testing.h from the command line.

[WIP]
- **data_struct.h**: implementations of generic, type-safe collections in native-C. Includes the following types:
    - *collection_t (type class)*  
    - array_t  
    - string_t  
    - *list_t (type class)*  
    - arraylist_t  
    - linkedlist_t  
    - *map_t (type class)*  
    - hashmap_t  
    - treemap_t  
    - *set_t (type class)*  
    - hashset_t  
    - treeset_t  
- **arg.h**: a utility to create command line interfaces and specify positional arguments, variadic arguments, and options. Confirms to the GNU CLI standards.

## Q&A

1. *Should I use this library in my new/existing project?*  
    No, because this library is still a work in progress and needs more testing and features before it's a viable dependency.
2. *Should I use this library after it's been fully developed and tested?*  
    No, because I'm not good at coding. Also C++ has been out for literal decades.
3. *Should I try it out in a project I don't care about?*  
    Knock yourself out.
