/*
* Filter interface for ciphers
* (C) 2013,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TRANSFORM_FILTER_H_
#define BOTAN_TRANSFORM_FILTER_H_

#include <botan/cipher_mode.h>
#include <botan/key_filt.h>
#include <botan/buf_filt.h>

namespace Botan {

/**
* Filter interface for cipher modes
*/
class BOTAN_PUBLIC_API(2,0) Cipher_Mode_Filter final : public Keyed_Filter,
                                     private Buffered_Filter
   {
   public:
      explicit Cipher_Mode_Filter(Cipher_Mode* t);

      explicit Cipher_Mode_Filter(std::unique_ptr<Cipher_Mode> t) :
         Cipher_Mode_Filter(t.release()) {}

      void set_iv(const InitializationVector& iv) override;

      void set_key(const SymmetricKey& key) override;

      Key_Length_Specification key_spec() const override;

      bool valid_iv_length(size_t length) const override;

      std::string name() const override;

   private:
      void write(const uint8_t input[], size_t input_length) override;
      void start_msg() override;
      void end_msg() override;

      void buffered_block(const uint8_t input[], size_t input_length) override;
      void buffered_final(const uint8_t input[], size_t input_length) override;

      std::unique_ptr<Cipher_Mode> m_mode;
      std::vector<uint8_t> m_nonce;
      secure_vector<uint8_t> m_buffer;
   };

// deprecated aliases, will be removed before 2.0
typedef Cipher_Mode_Filter Transform_Filter;
typedef Transform_Filter Transformation_Filter;

}

#endif
