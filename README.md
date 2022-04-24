# Quadtree

[![Build Status](https://travis-ci.org/pvigier/Quadtree.svg?branch=master)](https://travis-ci.org/pvigier/Quadtree)
[![codecov](https://codecov.io/gh/pvigier/Quadtree/branch/master/graph/badge.svg)](https://codecov.io/gh/pvigier/Quadtree)

`Quadtree` is a C++ implementation of a [quadtree](https://en.wikipedia.org/wiki/Quadtree).

`Quadtree` aims to be:

* versatile (can be used in dynamic and static contexts)
* simple
* lightweight
* easy to use
* fast
* header only
* implemented with modern C++ features (C++17)

[Google Benchmark](https://github.com/google/benchmark) is required to build the benchmarks and [GoogleTest](https://github.com/google/googletest) is required for the tests.

## Documentation

I have written an article on my blog describing the design and the code of the library. It is available [here](https://pvigier.github.io/2019/07/28/quadtree-collision-detection.html).

Otherwise, just look at the [Quadtree.h](https://github.com/pvigier/Quadtree/blob/master/include/Quadtree.h) file, the interface is easy to understand.

## License

Distributed under the MIT License.
