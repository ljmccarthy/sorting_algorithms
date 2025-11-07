# sorting_algorithms

This repository contains a collection of sorting algorithms and a test program.
At the moment it only tests a few basic patterns but I plan to expand this to include adversarial inputs and add benchmarking features.

The program test_sort tests the following sort functions:

- System-provided `qsort`, `mergesort`, `heapsort` and `psort` functions (where available)
- Some of my own implementations of:
    - Merge sort (including indirect pointer and indexed variants)
    - Insertion sort
- Third-party sort functions included in this repository:
    - Bentley & McIlroy's classic quicksort
    - Lynn Och's implementation of Knuth's smoothsort (which is used as qsort in musl libc)
    - Patrick Perry's port of Tim Peter's Timsort to C from the Java port (which was ported from Python's list sort function)
    - BSD heapsort function (copied from OpenBSD)
    - BSD mergesort function by Peter McIlroy (copied from FreeBSD)

The project is released in to the public domain with the Unlicense. Third-party code included in the `third_party` directory are licensed under their own licenses (see the source code for details).

## Build and run

On POSIX platforms (Linux and macOS), run the `build.sh` Bash script:

    $ ./build.sh Release
    $ ./build/Release/test_sort

On Windows, open a Visual Studio PowerShell prompt and run the `build.ps1` script:

    > .\build.ps1
    > .\build\Release\test_sort.exe

## test_sort usage

    test_sort [-f <function>] [-n <array-size>] [-s <elem-size>] [-r <seed>]

    -h
    --help
        Display usage help and list available sort functions.
    -f <function>
        Specify the name of the function to test. If omitted, all functions not
        classified as slow will be tested.
    -n <array-size>
        Specify the number of array elements (default: 1000000).
    -s <elem-size>
        Specify the size in bytes of each array element (default: 64).
    -r <seed>
        Specify a random seed to use (32-bit integer).

## References

- Musl qsort - https://git.musl-libc.org/cgit/musl/tree/src/stdlib/qsort.c
- Timsort - https://github.com/patperry/timsort
