/*
* Filter interface for compression
* (C) 2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_COMPRESSION_FILTER_H__
#define BOTAN_COMPRESSION_FILTER_H__

#include <botan/filter.h>

namespace Botan {

class Transform;
class Compressor_Transform;

/**
* Filter interface for compression/decompression
*/
class BOTAN_DLL Compression_Decompression_Filter : public Filter
   {
   public:
      void start_msg() override;
      void write(const byte input[], size_t input_length) override;
      void end_msg() override;

      std::string name() const override;

   protected:
      Compression_Decompression_Filter(Transform* t, size_t bs);

      void flush();
   private:
      std::unique_ptr<Compressor_Transform> m_transform;
      secure_vector<byte> m_buffer;
   };

class BOTAN_DLL Compression_Filter : public Compression_Decompression_Filter
   {
   public:
      Compression_Filter(const std::string& type,
                         size_t compression_level,
                         size_t buffer_size = 4096);

      using Compression_Decompression_Filter::flush;
   };

class Decompression_Filter : public Compression_Decompression_Filter
   {
   public:
      Decompression_Filter(const std::string& type,
                           size_t buffer_size = 4096);
   };

}

#endif
