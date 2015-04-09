/*
* Compression Transform
* (C) 2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_COMPRESSION_TRANSFORM_H__
#define BOTAN_COMPRESSION_TRANSFORM_H__

#include <botan/transform.h>

namespace Botan {

class BOTAN_DLL Compressor_Transform : public Transform
   {
   public:
      size_t update_granularity() const override { return 1; }

      size_t minimum_final_size() const override { return 0; }

      size_t default_nonce_length() const override { return 0; }

      bool valid_nonce_length(size_t nonce_len) const override
         { return nonce_len == 0; }

      virtual void flush(secure_vector<byte>& buf, size_t offset = 0) { update(buf, offset); }

      size_t output_length(size_t) const override
         {
         throw std::runtime_error(name() + " output length indeterminate");
         }
   };

BOTAN_DLL Transform* make_compressor(const std::string& type, size_t level);
BOTAN_DLL Transform* make_decompressor(const std::string& type);

class Compression_Stream
   {
   public:
      virtual ~Compression_Stream() {}

      virtual void next_in(byte* b, size_t len) = 0;

      virtual void next_out(byte* b, size_t len) = 0;

      virtual size_t avail_in() const = 0;

      virtual size_t avail_out() const = 0;

      virtual u32bit run_flag() const = 0;
      virtual u32bit flush_flag() const = 0;
      virtual u32bit finish_flag() const = 0;

      virtual bool run(u32bit flags) = 0;
   };

class BOTAN_DLL Stream_Compression : public Compressor_Transform
   {
   public:
      void update(secure_vector<byte>& buf, size_t offset = 0) override;

      void flush(secure_vector<byte>& buf, size_t offset = 0) override;

      void finish(secure_vector<byte>& buf, size_t offset = 0) override;

      void clear() override;
   private:
      secure_vector<byte> start_raw(const byte[], size_t) override;

      void process(secure_vector<byte>& buf, size_t offset, u32bit flags);

      virtual Compression_Stream* make_stream() const = 0;

      secure_vector<byte> m_buffer;
      std::unique_ptr<Compression_Stream> m_stream;
   };

class BOTAN_DLL Stream_Decompression : public Compressor_Transform
   {
   public:
      void update(secure_vector<byte>& buf, size_t offset = 0) override;

      void finish(secure_vector<byte>& buf, size_t offset = 0) override;

      void clear() override;

   private:
      secure_vector<byte> start_raw(const byte[], size_t) override;

      void process(secure_vector<byte>& buf, size_t offset, u32bit flags);

      virtual Compression_Stream* make_stream() const = 0;

      secure_vector<byte> m_buffer;
      std::unique_ptr<Compression_Stream> m_stream;
   };

}

#endif
