/*
* Filter interface for Transformations
* (C) 2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_TRANSFORMATION_FILTER_H__
#define BOTAN_TRANSFORMATION_FILTER_H__

#include <botan/transform.h>
#include <botan/key_filt.h>
#include <botan/buf_filt.h>

namespace Botan {

/**
* Filter interface for Transformations
*/
class BOTAN_DLL Transformation_Filter : public Keyed_Filter,
                                        private Buffered_Filter
   {
   public:
      Transformation_Filter(Transformation* t);

      void set_iv(const InitializationVector& iv) override;

      void set_key(const SymmetricKey& key) override;

      Key_Length_Specification key_spec() const override;

      bool valid_iv_length(size_t length) const override;

      std::string name() const override;

   protected:
      const Transformation& get_transform() const { return *m_transform; }

      Transformation& get_transform() { return *m_transform; }

   private:
      void write(const byte input[], size_t input_length) override;
      void start_msg() override;
      void end_msg() override;

      void buffered_block(const byte input[], size_t input_length) override;
      void buffered_final(const byte input[], size_t input_length) override;

      class Nonce_State
         {
         public:
            Nonce_State(bool allow_null_nonce) : m_fresh_nonce(allow_null_nonce) {}

            void update(const InitializationVector& iv);
            std::vector<byte> get();
         private:
            bool m_fresh_nonce;
            std::vector<byte> m_nonce;
         };

      Nonce_State m_nonce;
      std::unique_ptr<Transformation> m_transform;
      secure_vector<byte> m_buffer;
   };

}

#endif
