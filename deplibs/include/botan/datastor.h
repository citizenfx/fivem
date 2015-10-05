/*
* Data Store
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_DATA_STORE_H__
#define BOTAN_DATA_STORE_H__

#include <botan/secmem.h>
#include <functional>
#include <utility>
#include <string>
#include <vector>
#include <map>

namespace Botan {

/**
* Data Store
*/
class Data_Store
   {
   public:
      /**
      * A search function
      */
      bool operator==(const Data_Store&) const;

      std::multimap<std::string, std::string> search_for(
         std::function<bool (std::string, std::string)> predicate) const;

      std::vector<std::string> get(const std::string&) const;

      std::string get1(const std::string& key) const;

      std::string get1(const std::string& key,
                       const std::string& default_value) const;

      std::vector<byte> get1_memvec(const std::string&) const;
      u32bit get1_u32bit(const std::string&, u32bit = 0) const;

      bool has_value(const std::string&) const;

      void add(const std::multimap<std::string, std::string>&);
      void add(const std::string&, const std::string&);
      void add(const std::string&, u32bit);
      void add(const std::string&, const secure_vector<byte>&);
      void add(const std::string&, const std::vector<byte>&);
   private:
      std::multimap<std::string, std::string> contents;
   };

}

#endif
