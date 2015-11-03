/*
* TLS Extensions
* (C) 2011-2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_EXTENSIONS_H__
#define BOTAN_TLS_EXTENSIONS_H__

#include <botan/secmem.h>
#include <botan/tls_magic.h>
#include <vector>
#include <string>
#include <map>
#include <set>

namespace Botan {

namespace TLS {

class TLS_Data_Reader;

enum Handshake_Extension_Type {
   TLSEXT_SERVER_NAME_INDICATION = 0,
   TLSEXT_MAX_FRAGMENT_LENGTH    = 1,
   TLSEXT_CLIENT_CERT_URL        = 2,
   TLSEXT_TRUSTED_CA_KEYS        = 3,
   TLSEXT_TRUNCATED_HMAC         = 4,

   TLSEXT_CERTIFICATE_TYPES      = 9,
   TLSEXT_USABLE_ELLIPTIC_CURVES = 10,
   TLSEXT_EC_POINT_FORMATS       = 11,
   TLSEXT_SRP_IDENTIFIER         = 12,
   TLSEXT_SIGNATURE_ALGORITHMS   = 13,
   TLSEXT_USE_SRTP               = 14,
   TLSEXT_HEARTBEAT_SUPPORT      = 15,
   TLSEXT_ALPN                   = 16,

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
      virtual std::vector<byte> serialize() const = 0;

      /**
      * @return if we should encode this extension or not
      */
      virtual bool empty() const = 0;

      virtual ~Extension() {}
   };

/**
* Server Name Indicator extension (RFC 3546)
*/
class Server_Name_Indicator : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SERVER_NAME_INDICATION; }

      Handshake_Extension_Type type() const override { return static_type(); }

      Server_Name_Indicator(const std::string& host_name) :
         sni_host_name(host_name) {}

      Server_Name_Indicator(TLS_Data_Reader& reader,
                            u16bit extension_size);

      std::string host_name() const { return sni_host_name; }

      std::vector<byte> serialize() const override;

      bool empty() const override { return sni_host_name == ""; }
   private:
      std::string sni_host_name;
   };

/**
* SRP identifier extension (RFC 5054)
*/
class SRP_Identifier : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SRP_IDENTIFIER; }

      Handshake_Extension_Type type() const override { return static_type(); }

      SRP_Identifier(const std::string& identifier) :
         srp_identifier(identifier) {}

      SRP_Identifier(TLS_Data_Reader& reader,
                     u16bit extension_size);

      std::string identifier() const { return srp_identifier; }

      std::vector<byte> serialize() const override;

      bool empty() const override { return srp_identifier == ""; }
   private:
      std::string srp_identifier;
   };

/**
* Renegotiation Indication Extension (RFC 5746)
*/
class Renegotiation_Extension : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SAFE_RENEGOTIATION; }

      Handshake_Extension_Type type() const override { return static_type(); }

      Renegotiation_Extension() {}

      Renegotiation_Extension(const std::vector<byte>& bits) :
         reneg_data(bits) {}

      Renegotiation_Extension(TLS_Data_Reader& reader,
                             u16bit extension_size);

      const std::vector<byte>& renegotiation_info() const
         { return reneg_data; }

      std::vector<byte> serialize() const override;

      bool empty() const override { return false; } // always send this
   private:
      std::vector<byte> reneg_data;
   };

/**
* Maximum Fragment Length Negotiation Extension (RFC 4366 sec 3.2)
*/
class Maximum_Fragment_Length : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_MAX_FRAGMENT_LENGTH; }

      Handshake_Extension_Type type() const override { return static_type(); }

      bool empty() const override { return false; }

      size_t fragment_size() const { return m_max_fragment; }

      std::vector<byte> serialize() const override;

      /**
      * @param max_fragment specifies what maximum fragment size to
      *        advertise. Currently must be one of 512, 1024, 2048, or
      *        4096.
      */
      Maximum_Fragment_Length(size_t max_fragment) :
         m_max_fragment(max_fragment) {}

      Maximum_Fragment_Length(TLS_Data_Reader& reader,
                              u16bit extension_size);

   private:
      size_t m_max_fragment;
   };

/**
* ALPN (RFC 7301)
*/
class Application_Layer_Protocol_Notification : public Extension
   {
   public:
      static Handshake_Extension_Type static_type() { return TLSEXT_ALPN; }

      Handshake_Extension_Type type() const override { return static_type(); }

      const std::vector<std::string>& protocols() const { return m_protocols; }

      const std::string& single_protocol() const;

      /**
      * Single protocol, used by server
      */
      Application_Layer_Protocol_Notification(const std::string& protocol) :
         m_protocols(1, protocol) {}

      /**
      * List of protocols, used by client
      */
      Application_Layer_Protocol_Notification(const std::vector<std::string>& protocols) :
         m_protocols(protocols) {}

      Application_Layer_Protocol_Notification(TLS_Data_Reader& reader,
                                              u16bit extension_size);

      std::vector<byte> serialize() const override;

      bool empty() const override { return m_protocols.empty(); }
   private:
      std::vector<std::string> m_protocols;
   };

/**
* Session Ticket Extension (RFC 5077)
*/
class Session_Ticket : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SESSION_TICKET; }

      Handshake_Extension_Type type() const override { return static_type(); }

      /**
      * @return contents of the session ticket
      */
      const std::vector<byte>& contents() const { return m_ticket; }

      /**
      * Create empty extension, used by both client and server
      */
      Session_Ticket() {}

      /**
      * Extension with ticket, used by client
      */
      Session_Ticket(const std::vector<byte>& session_ticket) :
         m_ticket(session_ticket) {}

      /**
      * Deserialize a session ticket
      */
      Session_Ticket(TLS_Data_Reader& reader, u16bit extension_size);

      std::vector<byte> serialize() const override { return m_ticket; }

      bool empty() const override { return false; }
   private:
      std::vector<byte> m_ticket;
   };

/**
* Supported Elliptic Curves Extension (RFC 4492)
*/
class Supported_Elliptic_Curves : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_USABLE_ELLIPTIC_CURVES; }

      Handshake_Extension_Type type() const override { return static_type(); }

      static std::string curve_id_to_name(u16bit id);
      static u16bit name_to_curve_id(const std::string& name);

      const std::vector<std::string>& curves() const { return m_curves; }

      std::vector<byte> serialize() const override;

      Supported_Elliptic_Curves(const std::vector<std::string>& curves) :
         m_curves(curves) {}

      Supported_Elliptic_Curves(TLS_Data_Reader& reader,
                                u16bit extension_size);

      bool empty() const override { return m_curves.empty(); }
   private:
      std::vector<std::string> m_curves;
   };

/**
* Signature Algorithms Extension for TLS 1.2 (RFC 5246)
*/
class Signature_Algorithms : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SIGNATURE_ALGORITHMS; }

      Handshake_Extension_Type type() const override { return static_type(); }

      static std::string hash_algo_name(byte code);
      static byte hash_algo_code(const std::string& name);

      static std::string sig_algo_name(byte code);
      static byte sig_algo_code(const std::string& name);

      std::vector<std::pair<std::string, std::string> >
         supported_signature_algorthms() const
         {
         return m_supported_algos;
         }

      std::vector<byte> serialize() const override;

      bool empty() const override { return false; }

      Signature_Algorithms(const std::vector<std::string>& hashes,
                           const std::vector<std::string>& sig_algos);

      Signature_Algorithms(const std::vector<std::pair<std::string, std::string> >& algos) :
         m_supported_algos(algos) {}

      Signature_Algorithms(TLS_Data_Reader& reader,
                           u16bit extension_size);
   private:
      std::vector<std::pair<std::string, std::string> > m_supported_algos;
   };

/**
* Heartbeat Extension (RFC 6520)
*/
class Heartbeat_Support_Indicator : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_HEARTBEAT_SUPPORT; }

      Handshake_Extension_Type type() const override { return static_type(); }

      bool peer_allowed_to_send() const { return m_peer_allowed_to_send; }

      std::vector<byte> serialize() const override;

      bool empty() const override { return false; }

      Heartbeat_Support_Indicator(bool peer_allowed_to_send) :
         m_peer_allowed_to_send(peer_allowed_to_send) {}

      Heartbeat_Support_Indicator(TLS_Data_Reader& reader, u16bit extension_size);

   private:
      bool m_peer_allowed_to_send;
   };

/**
* Used to indicate SRTP algorithms for DTLS (RFC 5764)
*/
class SRTP_Protection_Profiles : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_USE_SRTP; }

      Handshake_Extension_Type type() const override { return static_type(); }

      const std::vector<u16bit>& profiles() const { return m_pp; }

      std::vector<byte> serialize() const override;

      bool empty() const override { return m_pp.empty(); }

      SRTP_Protection_Profiles(const std::vector<u16bit>& pp) : m_pp(pp) {}

      SRTP_Protection_Profiles(u16bit pp) : m_pp(1, pp) {}

      SRTP_Protection_Profiles(TLS_Data_Reader& reader, u16bit extension_size);
   private:
      std::vector<u16bit> m_pp;
   };

/**
* Represents a block of extensions in a hello message
*/
class Extensions
   {
   public:
      std::set<Handshake_Extension_Type> extension_types() const;

      template<typename T>
      T* get() const
         {
         Handshake_Extension_Type type = T::static_type();

         auto i = extensions.find(type);

         if(i != extensions.end())
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
         extensions[extn->type()].reset(extn);
         }

      std::vector<byte> serialize() const;

      void deserialize(TLS_Data_Reader& reader);

      Extensions() {}

      Extensions(TLS_Data_Reader& reader) { deserialize(reader); }

   private:
      Extensions(const Extensions&) {}
      Extensions& operator=(const Extensions&) { return (*this); }

      std::map<Handshake_Extension_Type, std::unique_ptr<Extension>> extensions;
   };

}

}

#endif
