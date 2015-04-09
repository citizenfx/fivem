/**
 * (C) Copyright Projet SECRET, INRIA, Rocquencourt
 * (C) Bhaskar Biswas and  Nicolas Sendrier
 *
 * (C) 2014 cryptosource GmbH
 * (C) 2014 Falko Strenzke fstrenzke@cryptosource.de
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 *
 */

#ifndef BOTAN_MCELIECE_H__
#define BOTAN_MCELIECE_H__

#include <botan/secmem.h>
#include <botan/types.h>
#include <botan/pk_ops.h>
#include <botan/mceliece_key.h>

#define MASK_LOG2_BYTE ((1 << 3) - 1)
#define _BITP_TO_BYTEP(__bit_pos) (__bit_pos >> 3)
#define _BITP_TO_BYTEOFFS(__bit_pos) (__bit_pos & MASK_LOG2_BYTE)

namespace Botan {

secure_vector<gf2m> BOTAN_DLL create_random_error_positions(unsigned code_length, unsigned error_weight, RandomNumberGenerator& rng);

class mceliece_message_parts
   {
   public:

      mceliece_message_parts(const secure_vector<gf2m>& err_pos, const byte* message, u32bit message_length, u32bit code_length) :
         m_error_vector(error_vector_from_error_positions(&err_pos[0], err_pos.size(), code_length)),
         m_code_length(code_length)
         {
         m_message_word.resize(message_length);
         copy_mem(&m_message_word[0], message, message_length);
         }

      mceliece_message_parts(const secure_vector<gf2m>& err_pos, const secure_vector<byte>& message, unsigned code_length) :
         m_error_vector(error_vector_from_error_positions(&err_pos[0], err_pos.size(), code_length)),
         m_message_word(message),
         m_code_length(code_length)
         {}

      static secure_vector<byte> error_vector_from_error_positions(const gf2m* err_pos, size_t err_pos_len, size_t code_length)
         {
         secure_vector<byte> result((code_length+7)/8);
         for(unsigned i = 0; i < err_pos_len; i++)
            {
            u16bit pos = err_pos[i];
            u32bit byte_pos = _BITP_TO_BYTEP(pos);
            if(byte_pos > result.size())
               {
               throw Invalid_Argument("error position larger than code size");
               }
            result[byte_pos] |= (1 << _BITP_TO_BYTEOFFS(pos));
            }
         return result;
         }

      mceliece_message_parts(const byte* message_concat_errors, size_t message_concat_errors_len, unsigned code_length) :
         m_code_length(code_length)
         {
         size_t err_vec_len = (code_length+7)/8;
         if(message_concat_errors_len < err_vec_len )
            {
            throw Invalid_Argument("cannot split McEliece message parts");
            }
         size_t err_vec_start_pos = message_concat_errors_len - err_vec_len;
         m_message_word = secure_vector<byte>(err_vec_start_pos );
         copy_mem(&m_message_word[0], &message_concat_errors[0], err_vec_start_pos);
         m_error_vector = secure_vector<byte>(err_vec_len );
         copy_mem(&m_error_vector[0],  &message_concat_errors[err_vec_start_pos], err_vec_len);
         }

      secure_vector<byte> get_concat() const
         {
         secure_vector<byte> result(m_error_vector.size() + m_message_word.size());
         copy_mem(&result[0], &m_message_word[0], m_message_word.size());
         copy_mem(&result[m_message_word.size()], &m_error_vector[0], m_error_vector.size());
         return result;
         }

      secure_vector<gf2m> get_error_positions() const
         {
         secure_vector<gf2m> result;
         for(unsigned i = 0; i < m_code_length; i++)
            {
            if(i >= m_code_length)
               {
               throw Invalid_Argument("index out of range in get_error_positions()");
               }
            if((m_error_vector[_BITP_TO_BYTEP(i)] >> _BITP_TO_BYTEOFFS(i)) & 1)
               {
               result.push_back(i);
               }
            }
         return result;
         }

      secure_vector<byte> get_error_vector() const { return m_error_vector; }
      secure_vector<byte> get_message_word() const { return m_message_word; }
   private:
      secure_vector<byte> m_error_vector;
      secure_vector<byte> m_message_word;
      unsigned m_code_length;
   };

class BOTAN_DLL McEliece_Private_Operation : public PK_Ops::Decryption
   {
   public:
      McEliece_Private_Operation(const McEliece_PrivateKey& mce_key);

      size_t max_input_bits() const { return m_priv_key.max_input_bits();  }

      secure_vector<byte> decrypt(const byte msg[], size_t msg_len);

      McEliece_PrivateKey const& get_key() const { return m_priv_key; }

   private:
      const McEliece_PrivateKey m_priv_key;
   };

class BOTAN_DLL McEliece_Public_Operation : public PK_Ops::Encryption
   {
   public:
      McEliece_Public_Operation(const McEliece_PublicKey& public_key, u32bit code_length);

      size_t max_input_bits() const { return m_pub_key.max_input_bits(); }
      secure_vector<byte> encrypt(const byte msg[], size_t msg_len, RandomNumberGenerator&);

      McEliece_PublicKey const& get_key() const { return m_pub_key; }

   private:
      McEliece_PublicKey m_pub_key;
      u32bit m_code_length;
   };

/**
* Estimate work factor for McEliece
* @return estimated security level for these key parameters
*/
BOTAN_DLL size_t mceliece_work_factor(size_t code_size, size_t k, size_t t);

}


#endif
