/*
* Base64 Encoder/Decoder
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_BASE64_FILTER_H__
#define BOTAN_BASE64_FILTER_H__

#include <botan/filter.h>

namespace Botan {

/**
* This class represents a Base64 encoder.
*/
class BOTAN_DLL Base64_Encoder : public Filter
   {
   public:
      std::string name() const { return "Base64_Encoder"; }

      /**
      * Input a part of a message to the encoder.
      * @param input the message to input as a byte array
      * @param length the length of the byte array input
      */
      void write(const byte input[], size_t length);

      /**
      * Inform the Encoder that the current message shall be closed.
      */
      void end_msg();

      /**
      * Create a base64 encoder.
      * @param breaks whether to use line breaks in the output
      * @param length the length of the lines of the output
      * @param t_n whether to use a trailing newline
      */
      Base64_Encoder(bool breaks = false, size_t length = 72,
                     bool t_n = false);
   private:
      void encode_and_send(const byte input[], size_t length,
                           bool final_inputs = false);
      void do_output(const byte output[], size_t length);

      const size_t line_length;
      const bool trailing_newline;
      std::vector<byte> in, out;
      size_t position, out_position;
   };

/**
* This object represents a Base64 decoder.
*/
class BOTAN_DLL Base64_Decoder : public Filter
   {
   public:
      std::string name() const { return "Base64_Decoder"; }

      /**
      * Input a part of a message to the decoder.
      * @param input the message to input as a byte array
      * @param length the length of the byte array input
      */
      void write(const byte input[], size_t length);

      /**
      * Finish up the current message
      */
      void end_msg();

      /**
      * Create a base64 decoder.
      * @param checking the type of checking that shall be performed by
      * the decoder
      */
      Base64_Decoder(Decoder_Checking checking = NONE);
   private:
      const Decoder_Checking checking;
      std::vector<byte> in, out;
      size_t position;
   };

}

#endif
