/*
* EntropySource
* (C) 2008,2009,2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ENTROPY_H__
#define BOTAN_ENTROPY_H__

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
      *
      * @param accum will be called with poll results, first params the data and
      * length, the second a best estimate of min-entropy for the entire buffer;
      * out of an abundance of caution this will be zero for many sources.
      * accum should return true if it wants the polling to stop, though it may
      * still be called again a few more times, and should be careful to return
      * true then as well.
      */
      Entropy_Accumulator(std::function<bool (const byte[], size_t, double)> accum) :
         m_accum_fn(accum) {}

      virtual ~Entropy_Accumulator() {}

      /**
      * @return if our polling goal has been achieved
      */
      bool polling_goal_achieved() const { return m_done; }

      bool polling_finished() const { return m_done; }

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
                             length, entropy_bits_per_byte * length) || m_done;
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
      bool m_done = false;
   };

/**
* Abstract interface to a source of entropy
*/
class BOTAN_DLL EntropySource
   {
   public:
      static void poll_available_sources(class Entropy_Accumulator& accum);

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
