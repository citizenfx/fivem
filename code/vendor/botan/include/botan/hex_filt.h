/*
* Hex Encoder/Decoder
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_HEX_FILTER_H_
#define BOTAN_HEX_FILTER_H_

#include <botan/filter.h>

namespace Botan {

/**
* Converts arbitrary binary data to hex strings, optionally with
* newlines inserted
*/
class BOTAN_PUBLIC_API(2,0) Hex_Encoder final : public Filter
   {
   public:
      /**
      * Whether to use uppercase or lowercase letters for the encoded string.
      */
      enum Case { Uppercase, Lowercase };

      std::string name() const override { return "Hex_Encoder"; }

      void write(const uint8_t in[], size_t length) override;
      void end_msg() override;

      /**
      * Create a hex encoder.
      * @param the_case the case to use in the encoded strings.
      */
      explicit Hex_Encoder(Case the_case);

      /**
      * Create a hex encoder.
      * @param newlines should newlines be used
      * @param line_length if newlines are used, how long are lines
      * @param the_case the case to use in the encoded strings
      */
      Hex_Encoder(bool newlines = false,
                  size_t line_length = 72,
                  Case the_case = Uppercase);
   private:
      void encode_and_send(const uint8_t[], size_t);

      const Case m_casing;
      const size_t m_line_length;
      std::vector<uint8_t> m_in, m_out;
      size_t m_position, m_counter;
   };

/**
* Converts hex strings to bytes
*/
class BOTAN_PUBLIC_API(2,0) Hex_Decoder final : public Filter
   {
   public:
      std::string name() const override { return "Hex_Decoder"; }

      void write(const uint8_t[], size_t) override;
      void end_msg() override;

      /**
      * Construct a Hex Decoder using the specified
      * character checking.
      * @param checking the checking to use during decoding.
      */
      explicit Hex_Decoder(Decoder_Checking checking = NONE);
   private:
      const Decoder_Checking m_checking;
      std::vector<uint8_t> m_in, m_out;
      size_t m_position;
   };

}

#endif
