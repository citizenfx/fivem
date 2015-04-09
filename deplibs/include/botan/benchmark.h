/*
* Runtime benchmarking
* (C) 2008-2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_RUNTIME_BENCHMARK_H__
#define BOTAN_RUNTIME_BENCHMARK_H__

#include <botan/rng.h>
#include <map>
#include <string>
#include <chrono>

namespace Botan {

/**
* Algorithm benchmark
* @param name the name of the algorithm to test (cipher, hash, or MAC)
* @param af the algorithm factory used to create objects
* @param rng the rng to use to generate random inputs
* @param milliseconds total time for the benchmark to run
* @param buf_size size of buffer to benchmark against, in KiB
* @return results a map from provider to speed in mebibytes per second
*/
std::map<std::string, double>
BOTAN_DLL algorithm_benchmark(const std::string& name,
                              RandomNumberGenerator& rng,
                              std::chrono::milliseconds milliseconds,
                              size_t buf_size);

}

#endif
