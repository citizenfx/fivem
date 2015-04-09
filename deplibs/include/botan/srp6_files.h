/*
* SRP-6a File Handling
* (C) 2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SRP6A_FILES_H__
#define BOTAN_SRP6A_FILES_H__

#include <botan/bigint.h>
#include <string>
#include <map>

namespace Botan {

/**
* A GnuTLS compatible SRP6 authenticator file
*/
class BOTAN_DLL SRP6_Authenticator_File
   {
   public:
      /**
      * @param filename will be opened and processed as a SRP
      * authenticator file
      */
      SRP6_Authenticator_File(const std::string& filename);

      bool lookup_user(const std::string& username,
                       BigInt& v,
                       std::vector<byte>& salt,
                       std::string& group_id) const;
   private:
      struct SRP6_Data
         {
         SRP6_Data() {}

         SRP6_Data(const BigInt& v,
                   const std::vector<byte>& salt,
                   const std::string& group_id) :
            v(v), salt(salt), group_id(group_id) {}

         BigInt v;
         std::vector<byte> salt;
         std::string group_id;
         };

      std::map<std::string, SRP6_Data> entries;
   };

}

#endif
