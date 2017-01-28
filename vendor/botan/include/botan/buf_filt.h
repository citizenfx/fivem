/*
* Buffered Filter
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_BUFFERED_FILTER_H__
#define BOTAN_BUFFERED_FILTER_H__

#include <botan/secmem.h>

namespace Botan {

/**
* Filter mixin that breaks input into blocks, useful for
* cipher modes
*/
class BOTAN_DLL Buffered_Filter
   {
   public:
      /**
      * Write bytes into the buffered filter, which will them emit them
      * in calls to buffered_block in the subclass
      * @param in the input bytes
      * @param length of in in bytes
      */
      void write(const uint8_t in[], size_t length);

      template<typename Alloc>
         void write(const std::vector<uint8_t, Alloc>& in, size_t length)
         {
         write(in.data(), length);
         }

      /**
      * Finish a message, emitting to buffered_block and buffered_final
      * Will throw an exception if less than final_minimum bytes were
      * written into the filter.
      */
      void end_msg();

      /**
      * Initialize a Buffered_Filter
      * @param block_size the function buffered_block will be called
      *        with inputs which are a multiple of this size
      * @param final_minimum the function buffered_final will be called
      *        with at least this many bytes.
      */
      Buffered_Filter(size_t block_size, size_t final_minimum);

      virtual ~Buffered_Filter() {}
   protected:
      /**
      * The block processor, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be a multiple
      *        of block_size
      */
      virtual void buffered_block(const uint8_t input[], size_t length) = 0;

      /**
      * The final block, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be at least
      *        final_minimum bytes
      */
      virtual void buffered_final(const uint8_t input[], size_t length) = 0;

      /**
      * @return block size of inputs
      */
      size_t buffered_block_size() const { return m_main_block_mod; }

      /**
      * @return current position in the buffer
      */
      size_t current_position() const { return m_buffer_pos; }

      /**
      * Reset the buffer position
      */
      void buffer_reset() { m_buffer_pos = 0; }
   private:
      size_t m_main_block_mod, m_final_minimum;

      secure_vector<uint8_t> m_buffer;
      size_t m_buffer_pos;
   };

}

#endif
