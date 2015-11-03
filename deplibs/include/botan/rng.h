/*
* RandomNumberGenerator
* (C) 1999-2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_RANDOM_NUMBER_GENERATOR_H__
#define BOTAN_RANDOM_NUMBER_GENERATOR_H__

#include <botan/entropy_src.h>
#include <botan/exceptn.h>
#include <string>
#include <mutex>

namespace Botan {

/**
* This class represents a random number (RNG) generator object.
*/
class BOTAN_DLL RandomNumberGenerator
   {
   public:
      /**
      * Create a seeded and active RNG object for general application use
      * Added in 1.8.0
      */
      static RandomNumberGenerator* make_rng();

      /**
      * Randomize a byte array.
      * @param output the byte array to hold the random output.
      * @param length the length of the byte array output.
      */
      virtual void randomize(byte output[], size_t length) = 0;

      /**
      * Return a random vector
      * @param bytes number of bytes in the result
      * @return randomized vector of length bytes
      */
      virtual secure_vector<byte> random_vec(size_t bytes)
         {
         secure_vector<byte> output(bytes);
         randomize(output.data(), output.size());
         return output;
         }

      /**
      * Only usable with POD types, only useful with integers
      * get_random<u64bit>()
      */
      template<typename T> T get_random()
         {
         T r;
         this->randomize(reinterpret_cast<byte*>(&r), sizeof(r));
         return r;
         }

      /**
      * Return a value in range [0,2^bits)
      */
      u64bit gen_mask(size_t bits)
         {
         if(bits == 0 || bits > 64)
            throw std::invalid_argument("RandomNumberGenerator::gen_mask invalid argument");

         const u64bit mask = ((1 << bits) - 1);
         return this->get_random<u64bit>() & mask;
         }

      /**
      * Return a random byte
      * @return random byte
      */
      byte next_byte() { return get_random<byte>(); }

      byte next_nonzero_byte()
         {
         byte b = next_byte();
         while(b == 0)
            b = next_byte();
         return b;
         }

      /**
      * Check whether this RNG is seeded.
      * @return true if this RNG was already seeded, false otherwise.
      */
      virtual bool is_seeded() const = 0;

      /**
      * Clear all internally held values of this RNG.
      */
      virtual void clear() = 0;

      /**
      * Return the name of this object
      */
      virtual std::string name() const = 0;

      /**
      * Seed this RNG using the entropy sources it contains.
      * @param bits_to_collect is the number of bits of entropy to
               attempt to gather from the entropy sources
      */
      virtual void reseed(size_t bits_to_collect) = 0;

      /**
      * Add entropy to this RNG.
      * @param in a byte array containg the entropy to be added
      * @param length the length of the byte array in
      */
      virtual void add_entropy(const byte in[], size_t length) = 0;

      /*
      * Never copy a RNG, create a new one
      */
      RandomNumberGenerator(const RandomNumberGenerator& rng) = delete;
      RandomNumberGenerator& operator=(const RandomNumberGenerator& rng) = delete;

      RandomNumberGenerator() {}
      virtual ~RandomNumberGenerator() {}
   };

/**
* Null/stub RNG - fails if you try to use it for anything
*/
class BOTAN_DLL Null_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte[], size_t) override { throw PRNG_Unseeded("Null_RNG"); }

      void clear() override {}

      std::string name() const override { return "Null_RNG"; }

      void reseed(size_t) override {}
      bool is_seeded() const override { return false; }
      void add_entropy(const byte[], size_t) override {}
   };

/**
* Wraps access to a RNG in a mutex
*/
class BOTAN_DLL Serialized_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte out[], size_t len) override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->randomize(out, len);
         }

      bool is_seeded() const override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_rng->is_seeded();
         }

      void clear() override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->clear();
         }

      std::string name() const override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_rng->name();
         }

      void reseed(size_t poll_bits) override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->reseed(poll_bits);
         }

      void add_entropy(const byte in[], size_t len) override
         {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_rng->add_entropy(in, len);
         }

      Serialized_RNG() : m_rng(RandomNumberGenerator::make_rng()) {}
   private:
      mutable std::mutex m_mutex;
      std::unique_ptr<RandomNumberGenerator> m_rng;
   };

}

#endif
