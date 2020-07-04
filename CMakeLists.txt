cmake_minimum_required(VERSION 3.14)
project(bench)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2 -mbmi")

include_directories(.)
include_directories(common)
include_directories(FPI)
include_directories(PE)

add_executable(bench
        common/Abstract.h
        common/bitset.h
        common/hash.h
        FPI/OO_FPI.h
        FPI/OO_SIMD.h
        FPI/PIE.h
        FPI/SS.h
        PE/CM_BF.h
        PE/CM_HT.h
        PE/OO_PE.h
        benchmark.h
        main.cpp)
