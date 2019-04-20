/*
* Basic Filters
* (C) 1999-2007 Jack Lloyd
* (C) 2013 Joel Low
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_BASEFILT_H_
#define BOTAN_BASEFILT_H_

#include <botan/filter.h>

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
  #include <thread>
#endif

namespace Botan {

/**
* BitBucket is a filter which simply discards all inputs
*/
class BOTAN_PUBLIC_API(2,0) BitBucket final : public Filter
   {
   public:
      void write(const uint8_t[], size_t) override { /* discard */ }

      std::string name() const override { return "BitBucket"; }
   };

/**
* This class represents Filter chains. A Filter chain is an ordered
* concatenation of Filters, the input to a Chain sequentially passes
* through all the Filters contained in the Chain.
*/

class BOTAN_PUBLIC_API(2,0) Chain final : public Fanout_Filter
   {
   public:
      void write(const uint8_t input[], size_t length) override { send(input, length); }

      std::string name() const override;

      /**
      * Construct a chain of up to four filters. The filters are set
      * up in the same order as the arguments.
      */
      Chain(Filter* = nullptr, Filter* = nullptr,
            Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a chain from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Chain(Filter* filter_arr[], size_t length);
   };

/**
* This class represents a fork filter, whose purpose is to fork the
* flow of data. It causes an input message to result in n messages at
* the end of the filter, where n is the number of forks.
*/
class BOTAN_PUBLIC_API(2,0) Fork : public Fanout_Filter
   {
   public:
      void write(const uint8_t input[], size_t length) override { send(input, length); }
      void set_port(size_t n) { Fanout_Filter::set_port(n); }

      std::string name() const override;

      /**
      * Construct a Fork filter with up to four forks.
      */
      Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Fork(Filter* filter_arr[], size_t length);
   };

#if defined(BOTAN_HAS_THREAD_UTILS)

/**
* This class is a threaded version of the Fork filter. While this uses
* threads, the class itself is NOT thread-safe. This is meant as a drop-
* in replacement for Fork where performance gains are possible.
*/
class BOTAN_PUBLIC_API(2,0) Threaded_Fork final : public Fork
   {
   public:
      std::string name() const override;

      /**
      * Construct a Threaded_Fork filter with up to four forks.
      */
      Threaded_Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Threaded_Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Threaded_Fork(Filter* filter_arr[], size_t length);

      ~Threaded_Fork();

   private:
      void set_next(Filter* f[], size_t n);
      void send(const uint8_t in[], size_t length) override;
      void thread_delegate_work(const uint8_t input[], size_t length);
      void thread_entry(Filter* filter);

      std::vector<std::shared_ptr<std::thread>> m_threads;
      std::unique_ptr<struct Threaded_Fork_Data> m_thread_data;
   };
#endif

}

#endif
