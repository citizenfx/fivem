/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PSK_DB_SQL_H_
#define BOTAN_PSK_DB_SQL_H_

#include <botan/psk_db.h>
#include <botan/database.h>

namespace Botan {

class BOTAN_PUBLIC_API(2,4) Encrypted_PSK_Database_SQL : public Encrypted_PSK_Database
   {
   public:
      Encrypted_PSK_Database_SQL(const secure_vector<uint8_t>& master_key,
                                 std::shared_ptr<SQL_Database> db,
                                 const std::string& table_name);

   private:
      void kv_set(const std::string& index, const std::string& value) override;
      std::string kv_get(const std::string& index) const override;
      void kv_del(const std::string& index) override;
      std::set<std::string> kv_get_all() const override;

      std::shared_ptr<SQL_Database> m_db;
      const std::string m_table_name;
   };

}

#endif
