/*
* EntropySource
* (C) 2008-2009,2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_ENTROPY_SOURCE_BASE_H__
#define BOTAN_ENTROPY_SOURCE_BASE_H__

#include <botan/secmem.h>
#include <string>
#include <functional>

namespace Botan {

/**
* Class used to accumulate the poll results of EntropySources
*/
class BOTAN_DLL Entropy_Accumulator
   {
   public:
      /**
      * Initialize an Entropy_Accumulator
      * @param goal is how many bits we would like to collect
      */
      Entropy_Accumulator(std::function<bool (const byte[], size_t, double)> accum) :
         m_accum_fn(accum), m_done(false) {}

      virtual ~Entropy_Accumulator() {}

      /**
      * Get a cached I/O buffer (purely for minimizing allocation
      * overhead to polls)
      *
      * @param size requested size for the I/O buffer
      * @return cached I/O buffer for repeated polls
      */
      secure_vector<byte>& get_io_buffer(size_t size)
         {
         m_io_buffer.clear();
         m_io_buffer.resize(size);
         return m_io_buffer;
         }

      /**
      * @return if our polling goal has been achieved
      */
      bool polling_goal_achieved() const { return m_done; }

      /**
      * Add entropy to the accumulator
      * @param bytes the input bytes
      * @param length specifies how many bytes the input is
      * @param entropy_bits_per_byte is a best guess at how much
      * entropy per byte is in this input
      */
      void add(const void* bytes, size_t length, double entropy_bits_per_byte)
         {
         m_done = m_accum_fn(reinterpret_cast<const byte*>(bytes),
                             length, entropy_bits_per_byte * length);
         }

      /**
      * Add entropy to the accumulator
      * @param v is some value
      * @param entropy_bits_per_byte is a best guess at how much
      * entropy per byte is in this input
      */
      template<typename T>
      void add(const T& v, double entropy_bits_per_byte)
         {
         add(&v, sizeof(T), entropy_bits_per_byte);
         }
   private:
      std::function<bool (const byte[], size_t, double)> m_accum_fn;
      bool m_done;
      secure_vector<byte> m_io_buffer;
   };

/**
* Abstract interface to a source of entropy
*/
class BOTAN_DLL EntropySource
   {
   public:
      /**
      * @return name identifying this entropy source
      */
      virtual std::string name() const = 0;

      /**
      * Perform an entropy gathering poll
      * @param accum is an accumulator object that will be given entropy
      */
      virtual void poll(Entropy_Accumulator& accum) = 0;

      virtual ~EntropySource() {}
   };

}

#endif
