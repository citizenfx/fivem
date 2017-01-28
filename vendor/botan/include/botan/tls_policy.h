/*
* Hooks for application level policies on TLS connections
* (C) 2004-2006,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_POLICY_H__
#define BOTAN_TLS_POLICY_H__

#include <botan/tls_version.h>
#include <botan/tls_ciphersuite.h>
#include <botan/x509cert.h>
#include <botan/dl_group.h>
#include <vector>
#include <sstream>

namespace Botan {

namespace TLS {

/**
* TLS Policy Base Class
* Inherit and overload as desired to suit local policy concerns
*/
class BOTAN_DLL Policy
   {
   public:

      /**
      * Returns a list of ciphers we are willing to negotiate, in
      * order of preference.
      */
      virtual std::vector<std::string> allowed_ciphers() const;

      /**
      * Returns a list of hash algorithms we are willing to use for
      * signatures, in order of preference.
      */
      virtual std::vector<std::string> allowed_signature_hashes() const;

      /**
      * Returns a list of MAC algorithms we are willing to use.
      */
      virtual std::vector<std::string> allowed_macs() const;

      /**
      * Returns a list of key exchange algorithms we are willing to
      * use, in order of preference. Allowed values: DH, empty string
      * (representing RSA using server certificate key)
      */
      virtual std::vector<std::string> allowed_key_exchange_methods() const;

      /**
      * Returns a list of signature algorithms we are willing to
      * use, in order of preference. Allowed values RSA and DSA.
      */
      virtual std::vector<std::string> allowed_signature_methods() const;

      /**
      * The minimum signature strength we will accept
      * Returning 80 allows RSA 1024 and SHA-1. Values larger than 80 disable SHA-1 support.
      * Returning 110 allows RSA 2048.
      * Return 128 to force ECC (P-256) or large (~3000 bit) RSA keys.
      * Default is 110
      */
      virtual size_t minimum_signature_strength() const;

      /**
      * Return if cert revocation info (CRL/OCSP) is required
      * If true, validation will fail unless a valid CRL or OCSP response
      * was examined.
      */
      virtual bool require_cert_revocation_info() const;

      bool allowed_signature_method(const std::string& sig_method) const;

      /**
      * Return list of ECC curves we are willing to use in order of preference
      */
      virtual std::vector<std::string> allowed_ecc_curves() const;

      bool allowed_ecc_curve(const std::string& curve) const;

      /**
      * Request that ECC curve points are sent compressed
      */
      virtual bool use_ecc_point_compression() const;

      /**
      * Returns a list of compression algorithms we are willing to use,
      * in order of preference. Allowed values any value of
      * Compression_Method.
      *
      * @note Compression is not currently supported
      */
      virtual std::vector<uint8_t> compression() const;

      /**
      * Choose an elliptic curve to use
      */
      virtual std::string choose_curve(const std::vector<std::string>& curve_names) const;

      /**
      * Allow renegotiation even if the counterparty doesn't
      * support the secure renegotiation extension.
      *
      * @warning Changing this to true exposes you to injected
      * plaintext attacks. Read RFC 5746 for background.
      */
      virtual bool allow_insecure_renegotiation() const;

      /**
      * The protocol dictates that the first 32 bits of the random
      * field are the current time in seconds. However this allows
      * client fingerprinting attacks. Set to false to disable, in
      * which case random bytes will be used instead.
      */
      virtual bool include_time_in_hello_random() const;

      /**
      * Allow servers to initiate a new handshake
      */
      virtual bool allow_server_initiated_renegotiation() const;
      
      /**
      * Allow TLS v1.0
      */
      virtual bool allow_tls10() const;
      
      /**
      * Allow TLS v1.1
      */
      virtual bool allow_tls11() const;
      
      /**
      * Allow TLS v1.2
      */
      virtual bool allow_tls12() const;
      
      /**
      * Allow DTLS v1.0
      */
      virtual bool allow_dtls10() const;
      
      /**
      * Allow DTLS v1.2
      */
      virtual bool allow_dtls12() const;

      virtual std::string dh_group() const;

      /**
      * Return the minimum DH group size we're willing to use
      * Default is currently 1024 (insecure), should be 2048
      */
      virtual size_t minimum_dh_group_size() const;
      
      /**
      * For ECDSA authenticated ciphersuites, the smallest key size the
      * client will accept.
      * This policy is currently only enforced on the server by the client.
      */
      virtual size_t minimum_ecdsa_group_size() const;
      
      /**
      * Return the minimum ECDH group size we're willing to use
      * for key exchange
      *
      * Default 255, allowing x25519 and larger
      * x25519 is the smallest curve we will negotiate
      * P-521 is the largest
      */
      virtual size_t minimum_ecdh_group_size() const;

      /**
      * Return the minimum bit size we're willing to accept for RSA
      * key exchange or server signatures.
      *
      * It does not place any requirements on the size of any RSA signature(s)
      * which were used to check the server certificate. This is only
      * concerned with the server's public key.
      *
      * Default is 2048 which is smallest RSA key size still secure
      * for medium term security.
      */
      virtual size_t minimum_rsa_bits() const;

      /**
      * Minimum DSA group size, default 2048 bits
      */
      virtual size_t minimum_dsa_group_size() const;

      /**
      * Throw an exception if you don't like the peer's key.
      * Default impl checks the key size against minimum_rsa_bits, minimum_ecdsa_group_size,
      * or minimum_ecdh_group_size depending on the key's type.
      * Override if you'd like to perform some other kind of test on
      * (or logging of) the peer's keys.
      */
      virtual void check_peer_key_acceptable(const Public_Key& public_key) const;
      
      /**
      * If this function returns false, unknown SRP/PSK identifiers
      * will be rejected with an unknown_psk_identifier alert as soon
      * as the non-existence is identified. Otherwise, a false
      * identifier value will be used and the protocol allowed to
      * proceed, causing the handshake to eventually fail without
      * revealing that the username does not exist on this system.
      */
      virtual bool hide_unknown_users() const;

      /**
      * Return the allowed lifetime of a session ticket. If 0, session
      * tickets do not expire until the session ticket key rolls over.
      * Expired session tickets cannot be used to resume a session.
      */
      virtual uint32_t session_ticket_lifetime() const;

      /**
      * If this returns a non-empty vector, and DTLS is negotiated,
      * then we will also attempt to negotiate the SRTP extension from
      * RFC 5764 using the returned values as the profile ids.
      */
      virtual std::vector<uint16_t> srtp_profiles() const;

      /**
      * @return true if and only if we are willing to accept this version
      * Default accepts TLS v1.0 and later or DTLS v1.2 or later.
      */
      virtual bool acceptable_protocol_version(Protocol_Version version) const;

      /**
      * Returns the more recent protocol version we are willing to
      * use, for either TLS or DTLS depending on datagram param.
      * Shouldn't ever need to override this unless you want to allow
      * a user to disable use of TLS v1.2 (which is *not recommended*)
      */
      virtual Protocol_Version latest_supported_version(bool datagram) const;

      /**
      * When offering this version, should we send a fallback SCSV?
      * Default returns true iff version is not the latest version the
      * policy allows, exists to allow override in case of interop problems.
      */
      virtual bool send_fallback_scsv(Protocol_Version version) const;

      /**
      * Allows policy to reject any ciphersuites which are undesirable
      * for whatever reason without having to reimplement ciphersuite_list
      */
      virtual bool acceptable_ciphersuite(const Ciphersuite& suite) const;

      /**
      * @return true if servers should choose the ciphersuite matching
      *         their highest preference, rather than the clients.
      *         Has no effect on client side.
      */
      virtual bool server_uses_own_ciphersuite_preferences() const;

      /**
      * Indicates whether the encrypt-then-MAC extension should be negotiated
      * (RFC 7366)
      */
      virtual bool negotiate_encrypt_then_mac() const;

      /**
      * Return allowed ciphersuites, in order of preference
      */
      virtual std::vector<uint16_t> ciphersuite_list(Protocol_Version version,
                                                   bool have_srp) const;

      /**
      * @return the default MTU for DTLS
      */
      virtual size_t dtls_default_mtu() const;

      /**
      * @return the initial timeout for DTLS
      */
      virtual size_t dtls_initial_timeout() const;

      /**
      * @return the maximum timeout for DTLS
      */
      virtual size_t dtls_maximum_timeout() const;

      /**
      * Convert this policy to a printable format.
      * @param o stream to be printed to
      */
      virtual void print(std::ostream& o) const;

      /**
      * Convert this policy to a printable format.
      * Same as calling `print` on a ostringstream and reading o.str()
      */
      std::string to_string() const;

      virtual ~Policy() {}
   };

/**
* NSA Suite B 128-bit security level (RFC 6460)
*/
class BOTAN_DLL NSA_Suite_B_128 : public Policy
   {
   public:
      std::vector<std::string> allowed_ciphers() const override
         { return std::vector<std::string>({"AES-128/GCM"}); }

      std::vector<std::string> allowed_signature_hashes() const override
         { return std::vector<std::string>({"SHA-256"}); }

      std::vector<std::string> allowed_macs() const override
         { return std::vector<std::string>({"AEAD"}); }

      std::vector<std::string> allowed_key_exchange_methods() const override
         { return std::vector<std::string>({"ECDH"}); }

      std::vector<std::string> allowed_signature_methods() const override
         { return std::vector<std::string>({"ECDSA"}); }

      std::vector<std::string> allowed_ecc_curves() const override
         { return std::vector<std::string>({"secp256r1"}); }

      size_t minimum_signature_strength() const override { return 128; }

      bool allow_tls10()  const override { return false; }
      bool allow_tls11()  const override { return false; }
      bool allow_tls12()  const override { return true;  }
      bool allow_dtls10() const override { return false; }
      bool allow_dtls12() const override { return false; }
   };

/**
* BSI TR-02102-2 Policy
*/
class BOTAN_DLL BSI_TR_02102_2 : public Policy
   {
   public:
      std::vector<std::string> allowed_ciphers() const override
         {
         return std::vector<std::string>({"AES-256/GCM", "AES-128/GCM", "AES-256", "AES-128" });
         }

      std::vector<std::string> allowed_signature_hashes() const override
         {
         return std::vector<std::string>({"SHA-384", "SHA-256"});
         }

      std::vector<std::string> allowed_macs() const override
         {
         return std::vector<std::string>({"AEAD", "SHA-384", "SHA-256"});
         }

      std::vector<std::string> allowed_key_exchange_methods() const override
         {
         return std::vector<std::string>({"ECDH", "DH", "PSK", "ECDHE_PSK", "DHE_PSK"});
         }

      std::vector<std::string> allowed_signature_methods() const override
         {
         return std::vector<std::string>({"ECDSA", "RSA", "DSA"});
         }

      std::vector<std::string> allowed_ecc_curves() const override
         {
         return std::vector<std::string>({"brainpool512r1", "brainpool384r1", "brainpool256r1", "secp384r1", "secp256r1"});
         }

      bool allow_insecure_renegotiation() const override { return false; }
      bool allow_server_initiated_renegotiation() const override { return true; }
      bool server_uses_own_ciphersuite_preferences() const override { return true; }
      bool negotiate_encrypt_then_mac() const override { return true; }

      size_t minimum_rsa_bits() const override { return 2000; }
      size_t minimum_dh_group_size() const override { return 2000; }
      size_t minimum_dsa_group_size() const override { return 2000; }

      size_t minimum_ecdh_group_size() const override { return 250; }
      size_t minimum_ecdsa_group_size() const override { return 250; }

      bool allow_tls10()  const override { return false; }
      bool allow_tls11()  const override { return false; }
      bool allow_tls12()  const override { return true;  }
      bool allow_dtls10() const override { return false; }
      bool allow_dtls12() const override { return false; }
   };

/**
* Policy for DTLS. We require DTLS v1.2 and an AEAD mode.
*/
class BOTAN_DLL Datagram_Policy : public Policy
   {
   public:
      std::vector<std::string> allowed_macs() const override
         { return std::vector<std::string>({"AEAD"}); }
            
      bool allow_tls10()  const override { return false; }
      bool allow_tls11()  const override { return false; }
      bool allow_tls12()  const override { return false; }
      bool allow_dtls10() const override { return false; }
      bool allow_dtls12() const override { return true;  }
   };

/*
* This policy requires a secure version of TLS and disables all insecure
* algorithms. It is compatible with other botan TLSes (including those using the
* default policy) and with many other recent implementations. It is a great idea
* to use if you control both sides of the protocol and don't have to worry
* about ancient and/or bizarre TLS implementations.
*/
class BOTAN_DLL Strict_Policy : public Policy
   {
   public:
      std::vector<std::string> allowed_ciphers() const override;

      std::vector<std::string> allowed_signature_hashes() const override;

      std::vector<std::string> allowed_macs() const override;

      std::vector<std::string> allowed_key_exchange_methods() const override;

      bool allow_tls10()  const override;
      bool allow_tls11()  const override;
      bool allow_tls12()  const override;
      bool allow_dtls10() const override;
      bool allow_dtls12() const override;
   };

class BOTAN_DLL Text_Policy : public Policy
   {
   public:

      std::vector<std::string> allowed_ciphers() const override
         { return get_list("ciphers", Policy::allowed_ciphers()); }

      std::vector<std::string> allowed_signature_hashes() const override
         { return get_list("signature_hashes", Policy::allowed_signature_hashes()); }

      std::vector<std::string> allowed_macs() const override
         { return get_list("macs", Policy::allowed_macs()); }

      std::vector<std::string> allowed_key_exchange_methods() const override
         { return get_list("key_exchange_methods", Policy::allowed_key_exchange_methods()); }

      std::vector<std::string> allowed_signature_methods() const override
         { return get_list("signature_methods", Policy::allowed_signature_methods()); }

      std::vector<std::string> allowed_ecc_curves() const override
         { return get_list("ecc_curves", Policy::allowed_ecc_curves()); }
      
      bool use_ecc_point_compression() const override
         { return get_bool("use_ecc_point_compression", Policy::use_ecc_point_compression()); }

      bool allow_tls10() const override
         { return get_bool("allow_tls10", Policy::allow_tls10()); }
      
      bool allow_tls11() const override
         { return get_bool("allow_tls11", Policy::allow_tls11()); }
      
      bool allow_tls12() const override
         { return get_bool("allow_tls12", Policy::allow_tls12()); }
      
      bool allow_dtls10() const override
         { return get_bool("allow_dtls10", Policy::allow_dtls10()); }
      
      bool allow_dtls12() const override
         { return get_bool("allow_dtls12", Policy::allow_dtls12()); }

      bool allow_insecure_renegotiation() const override
         { return get_bool("allow_insecure_renegotiation", Policy::allow_insecure_renegotiation()); }

      bool include_time_in_hello_random() const override
         { return get_bool("include_time_in_hello_random", Policy::include_time_in_hello_random()); }

      bool allow_server_initiated_renegotiation() const override
         { return get_bool("allow_server_initiated_renegotiation", Policy::allow_server_initiated_renegotiation()); }

      bool server_uses_own_ciphersuite_preferences() const override
         { return get_bool("server_uses_own_ciphersuite_preferences", Policy::server_uses_own_ciphersuite_preferences()); }

      bool negotiate_encrypt_then_mac() const override
         { return get_bool("negotiate_encrypt_then_mac", Policy::negotiate_encrypt_then_mac()); }

      std::string dh_group() const override
         { return get_str("dh_group", Policy::dh_group()); }

      size_t minimum_ecdh_group_size() const override
         { return get_len("minimum_ecdh_group_size", Policy::minimum_ecdh_group_size()); }

      size_t minimum_ecdsa_group_size() const override
         { return get_len("minimum_ecdsa_group_size", Policy::minimum_ecdsa_group_size()); }

      size_t minimum_dh_group_size() const override
         { return get_len("minimum_dh_group_size", Policy::minimum_dh_group_size()); }

      size_t minimum_rsa_bits() const override
         { return get_len("minimum_rsa_bits", Policy::minimum_rsa_bits()); }

      size_t minimum_signature_strength() const override
         { return get_len("minimum_signature_strength", Policy::minimum_signature_strength()); }

      bool hide_unknown_users() const override
         { return get_bool("hide_unknown_users", Policy::hide_unknown_users()); }

      uint32_t session_ticket_lifetime() const override
         { return static_cast<uint32_t>(get_len("session_ticket_lifetime", Policy::session_ticket_lifetime())); }

      bool send_fallback_scsv(Protocol_Version version) const override
         { return get_bool("send_fallback_scsv", false) ? Policy::send_fallback_scsv(version) : false; }

      std::vector<uint16_t> srtp_profiles() const override
         {
         std::vector<uint16_t> r;
         for(auto&& p : get_list("srtp_profiles", std::vector<std::string>()))
            {
            r.push_back(to_u32bit(p));
            }
         return r;
         }

      void set(const std::string& k, const std::string& v) { m_kv[k] = v; }

      explicit Text_Policy(const std::string& s)
         {
         std::istringstream iss(s);
         m_kv = read_cfg(iss);
         }

      explicit Text_Policy(std::istream& in) : m_kv(read_cfg(in))
         {}

   private:

      std::vector<std::string> get_list(const std::string& key,
                                        const std::vector<std::string>& def) const
         {
         const std::string v = get_str(key);

         if(v.empty())
            return def;

         return split_on(v, ' ');
         }

      size_t get_len(const std::string& key, size_t def) const
         {
         const std::string v = get_str(key);

         if(v.empty())
            return def;

         return to_u32bit(v);
         }

      bool get_bool(const std::string& key, bool def) const
         {
         const std::string v = get_str(key);

         if(v.empty())
            return def;

         if(v == "true" || v == "True")
            return true;
         else if(v == "false" || v == "False")
            return false;
         else
            throw Exception("Invalid boolean '" + v + "'");
         }

      std::string get_str(const std::string& key, const std::string& def = "") const
         {
         auto i = m_kv.find(key);
         if(i == m_kv.end())
            return def;

         return i->second;
         }

      std::map<std::string, std::string> m_kv;
   };

}

}

#endif
