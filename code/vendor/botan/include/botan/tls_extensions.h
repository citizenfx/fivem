/*
* TLS Extensions
* (C) 2011,2012,2016 Jack Lloyd
*     2016 Juraj Somorovsky
*     2016 Matthias Gierlings
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_EXTENSIONS_H__
#define BOTAN_TLS_EXTENSIONS_H__

#include <botan/secmem.h>
#include <botan/tls_magic.h>
#include <botan/ocsp.h>
#include <vector>
#include <string>
#include <map>
#include <set>

namespace Botan {

namespace TLS {

class TLS_Data_Reader;

enum Handshake_Extension_Type {
   TLSEXT_SERVER_NAME_INDICATION = 0,
   TLSEXT_CERT_STATUS_REQUEST    = 5,

   TLSEXT_CERTIFICATE_TYPES      = 9,
   TLSEXT_USABLE_ELLIPTIC_CURVES = 10,
   TLSEXT_EC_POINT_FORMATS       = 11,
   TLSEXT_SRP_IDENTIFIER         = 12,
   TLSEXT_SIGNATURE_ALGORITHMS   = 13,
   TLSEXT_USE_SRTP               = 14,
   TLSEXT_ALPN                   = 16,

   TLSEXT_ENCRYPT_THEN_MAC       = 22,
   TLSEXT_EXTENDED_MASTER_SECRET = 23,

   TLSEXT_SESSION_TICKET         = 35,

   TLSEXT_SAFE_RENEGOTIATION     = 65281,
};

/**
* Base class representing a TLS extension of some kind
*/
class Extension
   {
   public:
      /**
      * @return code number of the extension
      */
      virtual Handshake_Extension_Type type() const = 0;

      /**
      * @return serialized binary for the extension
      */
      virtual std::vector<uint8_t> serialize() const = 0;

      /**
      * @return if we should encode this extension or not
      */
      virtual bool empty() const = 0;

      virtual ~Extension() {}
   };

/**
* Server Name Indicator extension (RFC 3546)
*/
class Server_Name_Indicator final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SERVER_NAME_INDICATION; }

      Handshake_Extension_Type type() const override { return static_type(); }

      explicit Server_Name_Indicator(const std::string& host_name) :
         m_sni_host_name(host_name) {}

      Server_Name_Indicator(TLS_Data_Reader& reader,
                            uint16_t extension_size);

      std::string host_name() const { return m_sni_host_name; }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return m_sni_host_name.empty(); }
   private:
      std::string m_sni_host_name;
   };

#if defined(BOTAN_HAS_SRP6)
/**
* SRP identifier extension (RFC 5054)
*/
class SRP_Identifier final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SRP_IDENTIFIER; }

      Handshake_Extension_Type type() const override { return static_type(); }

      explicit SRP_Identifier(const std::string& identifier) :
         m_srp_identifier(identifier) {}

      SRP_Identifier(TLS_Data_Reader& reader,
                     uint16_t extension_size);

      std::string identifier() const { return m_srp_identifier; }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return m_srp_identifier.empty(); }
   private:
      std::string m_srp_identifier;
   };
#endif

/**
* Renegotiation Indication Extension (RFC 5746)
*/
class Renegotiation_Extension final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SAFE_RENEGOTIATION; }

      Handshake_Extension_Type type() const override { return static_type(); }

      Renegotiation_Extension() {}

      explicit Renegotiation_Extension(const std::vector<uint8_t>& bits) :
         m_reneg_data(bits) {}

      Renegotiation_Extension(TLS_Data_Reader& reader,
                             uint16_t extension_size);

      const std::vector<uint8_t>& renegotiation_info() const
         { return m_reneg_data; }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; } // always send this
   private:
      std::vector<uint8_t> m_reneg_data;
   };

/**
* ALPN (RFC 7301)
*/
class Application_Layer_Protocol_Notification final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type() { return TLSEXT_ALPN; }

      Handshake_Extension_Type type() const override { return static_type(); }

      const std::vector<std::string>& protocols() const { return m_protocols; }

      const std::string& single_protocol() const;

      /**
      * Single protocol, used by server
      */
      explicit Application_Layer_Protocol_Notification(const std::string& protocol) :
         m_protocols(1, protocol) {}

      /**
      * List of protocols, used by client
      */
      explicit Application_Layer_Protocol_Notification(const std::vector<std::string>& protocols) :
         m_protocols(protocols) {}

      Application_Layer_Protocol_Notification(TLS_Data_Reader& reader,
                                              uint16_t extension_size);

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return m_protocols.empty(); }
   private:
      std::vector<std::string> m_protocols;
   };

/**
* Session Ticket Extension (RFC 5077)
*/
class Session_Ticket final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SESSION_TICKET; }

      Handshake_Extension_Type type() const override { return static_type(); }

      /**
      * @return contents of the session ticket
      */
      const std::vector<uint8_t>& contents() const { return m_ticket; }

      /**
      * Create empty extension, used by both client and server
      */
      Session_Ticket() {}

      /**
      * Extension with ticket, used by client
      */
      explicit Session_Ticket(const std::vector<uint8_t>& session_ticket) :
         m_ticket(session_ticket) {}

      /**
      * Deserialize a session ticket
      */
      Session_Ticket(TLS_Data_Reader& reader, uint16_t extension_size);

      std::vector<uint8_t> serialize() const override { return m_ticket; }

      bool empty() const override { return false; }
   private:
      std::vector<uint8_t> m_ticket;
   };

/**
* Supported Elliptic Curves Extension (RFC 4492)
*/
class Supported_Elliptic_Curves final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_USABLE_ELLIPTIC_CURVES; }

      Handshake_Extension_Type type() const override { return static_type(); }

      static std::string curve_id_to_name(uint16_t id);
      static uint16_t name_to_curve_id(const std::string& name);

      const std::vector<std::string>& curves() const { return m_curves; }

      std::vector<uint8_t> serialize() const override;

      explicit Supported_Elliptic_Curves(const std::vector<std::string>& curves) :
         m_curves(curves) {}

      Supported_Elliptic_Curves(TLS_Data_Reader& reader,
                                uint16_t extension_size);

      bool empty() const override { return m_curves.empty(); }
   private:
      std::vector<std::string> m_curves;
   };

/**
* Supported Point Formats Extension (RFC 4492)
*/
class Supported_Point_Formats final : public Extension
   {
   public:
      enum ECPointFormat : uint8_t {
         UNCOMPRESSED = 0,
         ANSIX962_COMPRESSED_PRIME = 1,
         ANSIX962_COMPRESSED_CHAR2 = 2, // don't support these curves
      };

      static Handshake_Extension_Type static_type()
         { return TLSEXT_EC_POINT_FORMATS; }

      Handshake_Extension_Type type() const override { return static_type(); }

      std::vector<uint8_t> serialize() const override;

      explicit Supported_Point_Formats(bool prefer_compressed) :
         m_prefers_compressed(prefer_compressed) {}

      Supported_Point_Formats(TLS_Data_Reader& reader,
                              uint16_t extension_size);

      bool empty() const override { return false; }

      bool prefers_compressed() { return m_prefers_compressed; }

   private:
      bool m_prefers_compressed = false;
   };

/**
* Signature Algorithms Extension for TLS 1.2 (RFC 5246)
*/
class Signature_Algorithms final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SIGNATURE_ALGORITHMS; }

      Handshake_Extension_Type type() const override { return static_type(); }

      static std::string hash_algo_name(uint8_t code);
      static uint8_t hash_algo_code(const std::string& name);

      static std::string sig_algo_name(uint8_t code);
      static uint8_t sig_algo_code(const std::string& name);

      // [(hash,sig),(hash,sig),...]
      const std::vector<std::pair<std::string, std::string>>&
      supported_signature_algorthms() const
         {
         return m_supported_algos;
         }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; }

      Signature_Algorithms(const std::vector<std::string>& hashes,
                           const std::vector<std::string>& sig_algos);

      explicit Signature_Algorithms(const std::vector<std::pair<std::string, std::string>>& algos) :
         m_supported_algos(algos) {}

      Signature_Algorithms(TLS_Data_Reader& reader,
                           uint16_t extension_size);
   private:
      std::vector<std::pair<std::string, std::string>> m_supported_algos;
   };

/**
* Used to indicate SRTP algorithms for DTLS (RFC 5764)
*/
class SRTP_Protection_Profiles final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_USE_SRTP; }

      Handshake_Extension_Type type() const override { return static_type(); }

      const std::vector<uint16_t>& profiles() const { return m_pp; }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return m_pp.empty(); }

      explicit SRTP_Protection_Profiles(const std::vector<uint16_t>& pp) : m_pp(pp) {}

      explicit SRTP_Protection_Profiles(uint16_t pp) : m_pp(1, pp) {}

      SRTP_Protection_Profiles(TLS_Data_Reader& reader, uint16_t extension_size);
   private:
      std::vector<uint16_t> m_pp;
   };

/**
* Extended Master Secret Extension (RFC 7627)
*/
class Extended_Master_Secret final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_EXTENDED_MASTER_SECRET; }

      Handshake_Extension_Type type() const override { return static_type(); }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; }

      Extended_Master_Secret() {}

      Extended_Master_Secret(TLS_Data_Reader& reader, uint16_t extension_size);
   };

/**
* Encrypt-then-MAC Extension (RFC 7366)
*/
class Encrypt_then_MAC final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_ENCRYPT_THEN_MAC; }

      Handshake_Extension_Type type() const override { return static_type(); }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; }

      Encrypt_then_MAC() {}

      Encrypt_then_MAC(TLS_Data_Reader& reader, uint16_t extension_size);
   };

/**
* Certificate Status Request (RFC 6066)
*/
class Certificate_Status_Request final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_CERT_STATUS_REQUEST; }

      Handshake_Extension_Type type() const override { return static_type(); }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; }

      // Server generated version: empty
      Certificate_Status_Request();

      // Client version, both lists can be empty
      Certificate_Status_Request(const std::vector<X509_DN>& ocsp_responder_ids,
                                 const std::vector<std::vector<uint8_t>>& ocsp_key_ids);

      Certificate_Status_Request(TLS_Data_Reader& reader, uint16_t extension_size);
   private:
      std::vector<X509_DN> m_ocsp_names;
      std::vector<std::vector<uint8_t>> m_ocsp_keys;
      std::vector<uint8_t> m_extension_bytes;
      bool m_server_side;
   };

/**
* Represents a block of extensions in a hello message
*/
class BOTAN_DLL Extensions
   {
   public:
      std::set<Handshake_Extension_Type> extension_types() const;

      template<typename T>
      T* get() const
         {
         Handshake_Extension_Type type = T::static_type();

         auto i = m_extensions.find(type);

         if(i != m_extensions.end())
            return dynamic_cast<T*>(i->second.get());
         return nullptr;
         }

      template<typename T>
      bool has() const
         {
         return get<T>() != nullptr;
         }

      void add(Extension* extn)
         {
         m_extensions[extn->type()].reset(extn);
         }

      std::vector<uint8_t> serialize() const;

      void deserialize(TLS_Data_Reader& reader);

      Extensions() {}

      explicit Extensions(TLS_Data_Reader& reader) { deserialize(reader); }

   private:
      Extensions(const Extensions&) {}
      Extensions& operator=(const Extensions&) { return (*this); }

      std::map<Handshake_Extension_Type, std::unique_ptr<Extension>> m_extensions;
   };

}

}

#endif
