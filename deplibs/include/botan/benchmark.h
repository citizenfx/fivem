/*
* Runtime benchmarking
* (C) 2008-2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_RUNTIME_BENCHMARK_H__
#define BOTAN_RUNTIME_BENCHMARK_H__

#include <botan/algo_factory.h>
#include <botan/rng.h>
#include <map>
#include <string>
#include <chrono>

namespace Botan {

/**
* Time aspects of an algorithm/provider
* @param name the name of the algorithm to test
* @param af the algorithm factory used to create objects
* @param provider the provider to use
* @param rng the rng to use to generate random inputs
* @param runtime total time for the benchmark to run
* @param buf_size size of buffer to benchmark against, in KiB
* @return results a map from op type to operations per second
*/
std::map<std::string, double>
BOTAN_DLL time_algorithm_ops(const std::string& name,
                             Algorithm_Factory& af,
                             const std::string& provider,
                             RandomNumberGenerator& rng,
                             std::chrono::nanoseconds runtime,
                             size_t buf_size);

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
                              Algorithm_Factory& af,
                              RandomNumberGenerator& rng,
                              std::chrono::milliseconds milliseconds,
                              size_t buf_size);

double BOTAN_DLL
time_op(std::chrono::nanoseconds runtime, std::function<void ()> op);

}

#endif
