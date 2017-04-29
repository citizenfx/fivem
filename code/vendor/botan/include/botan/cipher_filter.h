/*
* Filter interface for ciphers
* (C) 2013,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TRANSFORM_FILTER_H__
#define BOTAN_TRANSFORM_FILTER_H__

#include <botan/cipher_mode.h>
#include <botan/key_filt.h>
#include <botan/buf_filt.h>

namespace Botan {

/**
* Filter interface for cipher modes
*/
class BOTAN_DLL Cipher_Mode_Filter : public Keyed_Filter,
                                     private Buffered_Filter
   {
   public:
      explicit Cipher_Mode_Filter(Cipher_Mode* t);

      void set_iv(const InitializationVector& iv) override;

      void set_key(const SymmetricKey& key) override;

      Key_Length_Specification key_spec() const override;

      bool valid_iv_length(size_t length) const override;

      std::string name() const override;

   protected:
      const Cipher_Mode& get_mode() const { return *m_mode; }

      Cipher_Mode& get_mode() { return *m_mode; }

   private:
      void write(const uint8_t input[], size_t input_length) override;
      void start_msg() override;
      void end_msg() override;

      void buffered_block(const uint8_t input[], size_t input_length) override;
      void buffered_final(const uint8_t input[], size_t input_length) override;

      class Nonce_State
         {
         public:
            explicit Nonce_State(bool allow_null_nonce) : m_fresh_nonce(allow_null_nonce) {}

            void update(const InitializationVector& iv);
            std::vector<uint8_t> get();
         private:
            bool m_fresh_nonce;
            std::vector<uint8_t> m_nonce;
         };

      Nonce_State m_nonce;
      std::unique_ptr<Cipher_Mode> m_mode;
      secure_vector<uint8_t> m_buffer;
   };

// deprecated aliases, will be removed before 2.0
typedef Cipher_Mode_Filter Transform_Filter;
typedef Transform_Filter Transformation_Filter;

}

#endif
