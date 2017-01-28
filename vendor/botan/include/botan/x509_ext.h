/*
* X.509 Certificate Extensions
* (C) 1999-2007,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_X509_EXTENSIONS_H__
#define BOTAN_X509_EXTENSIONS_H__

#include <botan/asn1_obj.h>
#include <botan/asn1_oid.h>
#include <botan/asn1_alt_name.h>
#include <botan/cert_status.h>
#include <botan/datastor.h>
#include <botan/name_constraint.h>
#include <botan/key_constraint.h>
#include <botan/crl_ent.h>

namespace Botan {

class X509_Certificate;

/**
* X.509 Certificate Extension
*/
class BOTAN_DLL Certificate_Extension
   {
   public:
      /**
      * @return OID representing this extension
      */
      virtual OID oid_of() const;

      /**
      * Make a copy of this extension
      * @return copy of this
      */
      virtual Certificate_Extension* copy() const = 0;

      /*
      * Add the contents of this extension into the information
      * for the subject and/or issuer, as necessary.
      * @param subject the subject info
      * @param issuer the issuer info
      */
      virtual void contents_to(Data_Store& subject,
                               Data_Store& issuer) const = 0;

      /*
      * @return specific OID name
      */
      virtual std::string oid_name() const = 0;

      /*
      * Callback visited during path validation.
      *
      * An extension can implement this callback to inspect
      * the path during path validation.
      *
      * If an error occurs during validation of this extension,
      * an appropriate status code shall be added to cert_status.
      *
      * @param subject Subject certificate that contains this extension
      * @param issuer Issuer certificate
      * @param status Certificate validation status codes for subject certificate
      * @param cert_path Certificate path which is currently validated
      * @param pos Position of subject certificate in cert_path
      */
      virtual void validate(const X509_Certificate& subject, const X509_Certificate& issuer,
            const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
            std::vector<std::set<Certificate_Status_Code>>& cert_status,
            size_t pos);

      virtual ~Certificate_Extension() {}
   protected:
      friend class Extensions;
      virtual bool should_encode() const { return true; }
      virtual std::vector<uint8_t> encode_inner() const = 0;
      virtual void decode_inner(const std::vector<uint8_t>&) = 0;
   };

/**
* X.509 Certificate Extension List
*/
class BOTAN_DLL Extensions : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;
      void contents_to(Data_Store&, Data_Store&) const;

      /**
      * Adds a new extension to the list.
      * @param extn the certificate extension
      * @param critical whether this extension should be marked as critical
      * @throw Invalid_Argument if the extension is already present in the list
      */
      void add(Certificate_Extension* extn, bool critical = false);

      /**
      * Adds an extension to the list or replaces it.
      * @param extn the certificate extension
      * @param critical whether this extension should be marked as critical
      */
      void replace(Certificate_Extension* extn, bool critical = false);

      /**
      * Searches for an extension by OID and returns the result.
      * Only the known extensions types declared in this header
      * are searched for by this function.
      * @return Pointer to extension with oid, nullptr if not found.
      */
      std::unique_ptr<Certificate_Extension> get(const OID& oid) const;

      /**
      * Searches for an extension by OID and returns the result.
      * Only the unknown extensions, that is, extensions
      * types that are not declared in this header, are searched
      * for by this function.
      * @return Pointer to extension with oid, nullptr if not found.
      */
      template<typename T>
      std::unique_ptr<T> get_raw(const OID& oid)
      {
      try
         {
         if(m_extensions_raw.count(oid) > 0)
            {
            std::unique_ptr<T> ext(new T);
            ext->decode_inner(m_extensions_raw[oid].first);
            return std::move(ext);
            }
         }
      catch(std::exception& e)
         {
         throw Decoding_Error("Exception while decoding extension " +
                              oid.as_string() + ": " + e.what());
         }
      return nullptr;
      }

      /**
      * Returns the list of extensions together with the corresponding
      * criticality flag. Only contains the known extensions
      * types declared in this header.
      */
      std::vector<std::pair<std::unique_ptr<Certificate_Extension>, bool>> extensions() const;

      /**
      * Returns the list of extensions as raw, encoded bytes
      * together with the corresponding criticality flag.
      * Contains all extensions, known as well as unknown extensions.
      */
      std::map<OID, std::pair<std::vector<uint8_t>, bool>> extensions_raw() const;

      Extensions& operator=(const Extensions&);

      Extensions(const Extensions&);

      /**
      * @param st whether to throw an exception when encountering an unknown
      * extension type during decoding
      */
      explicit Extensions(bool st = true) : m_throw_on_unknown_critical(st) {}

   private:
      static Certificate_Extension* create_extension(const OID&, bool);

      std::vector<std::pair<std::unique_ptr<Certificate_Extension>, bool>> m_extensions;
      bool m_throw_on_unknown_critical;
      std::map<OID, std::pair<std::vector<uint8_t>, bool>> m_extensions_raw;
   };

namespace Cert_Extension {

static const size_t NO_CERT_PATH_LIMIT = 0xFFFFFFF0;

/**
* Basic Constraints Extension
*/
class BOTAN_DLL Basic_Constraints final : public Certificate_Extension
   {
   public:
      Basic_Constraints* copy() const override
         { return new Basic_Constraints(m_is_ca, m_path_limit); }

      Basic_Constraints(bool ca = false, size_t limit = 0) :
         m_is_ca(ca), m_path_limit(limit) {}

      bool get_is_ca() const { return m_is_ca; }
      size_t get_path_limit() const;

   private:
      std::string oid_name() const override
         { return "X509v3.BasicConstraints"; }

      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      bool m_is_ca;
      size_t m_path_limit;
   };

/**
* Key Usage Constraints Extension
*/
class BOTAN_DLL Key_Usage final : public Certificate_Extension
   {
   public:
      Key_Usage* copy() const override { return new Key_Usage(m_constraints); }

      explicit Key_Usage(Key_Constraints c = NO_CONSTRAINTS) : m_constraints(c) {}

      Key_Constraints get_constraints() const { return m_constraints; }

   private:
      std::string oid_name() const override { return "X509v3.KeyUsage"; }

      bool should_encode() const override
         { return (m_constraints != NO_CONSTRAINTS); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      Key_Constraints m_constraints;
   };

/**
* Subject Key Identifier Extension
*/
class BOTAN_DLL Subject_Key_ID final : public Certificate_Extension
   {
   public:
      Subject_Key_ID* copy() const override
         { return new Subject_Key_ID(m_key_id); }

      Subject_Key_ID() {}
      explicit Subject_Key_ID(const std::vector<uint8_t>&);

      std::vector<uint8_t> get_key_id() const { return m_key_id; }
   private:
      std::string oid_name() const override
         { return "X509v3.SubjectKeyIdentifier"; }

      bool should_encode() const override { return (m_key_id.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<uint8_t> m_key_id;
   };

/**
* Authority Key Identifier Extension
*/
class BOTAN_DLL Authority_Key_ID final : public Certificate_Extension
   {
   public:
      Authority_Key_ID* copy() const override
         { return new Authority_Key_ID(m_key_id); }

      Authority_Key_ID() {}
      explicit Authority_Key_ID(const std::vector<uint8_t>& k) : m_key_id(k) {}

      std::vector<uint8_t> get_key_id() const { return m_key_id; }

   private:
      std::string oid_name() const override
         { return "X509v3.AuthorityKeyIdentifier"; }

      bool should_encode() const override { return (m_key_id.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<uint8_t> m_key_id;
   };

/**
* Alternative Name Extension Base Class
*/
class BOTAN_DLL Alternative_Name : public Certificate_Extension
   {
   public:
      AlternativeName get_alt_name() const { return m_alt_name; }

   protected:
      Alternative_Name(const AlternativeName&, const std::string& oid_name);

      Alternative_Name(const std::string&, const std::string&);

   private:
      std::string oid_name() const override { return m_oid_name_str; }

      bool should_encode() const override { return m_alt_name.has_items(); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::string m_oid_name_str;
      AlternativeName m_alt_name;
   };

/**
* Subject Alternative Name Extension
*/
class BOTAN_DLL Subject_Alternative_Name : public Alternative_Name
   {
   public:
      Subject_Alternative_Name* copy() const override
         { return new Subject_Alternative_Name(get_alt_name()); }

      explicit Subject_Alternative_Name(const AlternativeName& = AlternativeName());
   };

/**
* Issuer Alternative Name Extension
*/
class BOTAN_DLL Issuer_Alternative_Name : public Alternative_Name
   {
   public:
      Issuer_Alternative_Name* copy() const override
         { return new Issuer_Alternative_Name(get_alt_name()); }

      explicit Issuer_Alternative_Name(const AlternativeName& = AlternativeName());
   };

/**
* Extended Key Usage Extension
*/
class BOTAN_DLL Extended_Key_Usage final : public Certificate_Extension
   {
   public:
      Extended_Key_Usage* copy() const override
         { return new Extended_Key_Usage(m_oids); }

      Extended_Key_Usage() {}
      explicit Extended_Key_Usage(const std::vector<OID>& o) : m_oids(o) {}

      std::vector<OID> get_oids() const { return m_oids; }

   private:
      std::string oid_name() const override
         { return "X509v3.ExtendedKeyUsage"; }

      bool should_encode() const override { return (m_oids.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<OID> m_oids;
   };

/**
* Name Constraints
*/
class BOTAN_DLL Name_Constraints : public Certificate_Extension
   {
   public:
      Name_Constraints* copy() const override
         { return new Name_Constraints(m_name_constraints); }

      Name_Constraints() {}
      Name_Constraints(const NameConstraints &nc) : m_name_constraints(nc) {}

      void validate(const X509_Certificate& subject, const X509_Certificate& issuer,
            const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
            std::vector<std::set<Certificate_Status_Code>>& cert_status,
            size_t pos) override;

   private:
      std::string oid_name() const override
         { return "X509v3.NameConstraints"; }

      bool should_encode() const override { return true; }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      NameConstraints m_name_constraints;
   };

/**
* Certificate Policies Extension
*/
class BOTAN_DLL Certificate_Policies final : public Certificate_Extension
   {
   public:
      Certificate_Policies* copy() const override
         { return new Certificate_Policies(m_oids); }

      Certificate_Policies() {}
      explicit Certificate_Policies(const std::vector<OID>& o) : m_oids(o) {}

      std::vector<OID> get_oids() const { return m_oids; }

   private:
      std::string oid_name() const override
         { return "X509v3.CertificatePolicies"; }

      bool should_encode() const override { return (m_oids.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<OID> m_oids;
   };

class BOTAN_DLL Authority_Information_Access final : public Certificate_Extension
   {
   public:
      Authority_Information_Access* copy() const override
         { return new Authority_Information_Access(m_ocsp_responder); }

      Authority_Information_Access() {}

      explicit Authority_Information_Access(const std::string& ocsp) :
         m_ocsp_responder(ocsp) {}

   private:
      std::string oid_name() const override
         { return "PKIX.AuthorityInformationAccess"; }

      bool should_encode() const override { return (!m_ocsp_responder.empty()); }

      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;

      void contents_to(Data_Store&, Data_Store&) const override;

      std::string m_ocsp_responder;
   };

/**
* CRL Number Extension
*/
class BOTAN_DLL CRL_Number final : public Certificate_Extension
   {
   public:
      CRL_Number* copy() const override;

      CRL_Number() : m_has_value(false), m_crl_number(0) {}
      CRL_Number(size_t n) : m_has_value(true), m_crl_number(n) {}

      size_t get_crl_number() const;

   private:
      std::string oid_name() const override { return "X509v3.CRLNumber"; }

      bool should_encode() const override { return m_has_value; }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      bool m_has_value;
      size_t m_crl_number;
   };

/**
* CRL Entry Reason Code Extension
*/
class BOTAN_DLL CRL_ReasonCode final : public Certificate_Extension
   {
   public:
      CRL_ReasonCode* copy() const override
         { return new CRL_ReasonCode(m_reason); }

      explicit CRL_ReasonCode(CRL_Code r = UNSPECIFIED) : m_reason(r) {}

      CRL_Code get_reason() const { return m_reason; }

   private:
      std::string oid_name() const override { return "X509v3.ReasonCode"; }

      bool should_encode() const override { return (m_reason != UNSPECIFIED); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      CRL_Code m_reason;
   };

/**
* CRL Distribution Points Extension
*/
class BOTAN_DLL CRL_Distribution_Points final : public Certificate_Extension
   {
   public:
      class BOTAN_DLL Distribution_Point final : public ASN1_Object
         {
         public:
            void encode_into(class DER_Encoder&) const override;
            void decode_from(class BER_Decoder&) override;

            const AlternativeName& point() const { return m_point; }
         private:
            AlternativeName m_point;
         };

      CRL_Distribution_Points* copy() const override
         { return new CRL_Distribution_Points(m_distribution_points); }

      CRL_Distribution_Points() {}

      explicit CRL_Distribution_Points(const std::vector<Distribution_Point>& points) :
         m_distribution_points(points) {}

      std::vector<Distribution_Point> distribution_points() const
         { return m_distribution_points; }

   private:
      std::string oid_name() const override
         { return "X509v3.CRLDistributionPoints"; }

      bool should_encode() const override
         { return !m_distribution_points.empty(); }

      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<Distribution_Point> m_distribution_points;
   };

/**
* An unknown X.509 extension marked as critical
* Will always add a failure to the path validation result.
*/
class BOTAN_DLL Unknown_Critical_Extension final : public Certificate_Extension
   {
   public:
      explicit Unknown_Critical_Extension(OID oid) : m_oid(oid) {}

      Unknown_Critical_Extension* copy() const override
         { return new Unknown_Critical_Extension(m_oid); }

      OID oid_of() const override
         { return m_oid; };

      void validate(const X509_Certificate&, const X509_Certificate&,
      		const std::vector<std::shared_ptr<const X509_Certificate>>&,
      		std::vector<std::set<Certificate_Status_Code>>& cert_status,
      		size_t pos) override
         {
         cert_status.at(pos).insert(Certificate_Status_Code::UNKNOWN_CRITICAL_EXTENSION);
         }

   private:
      std::string oid_name() const override
         { return "Unknown OID name"; }

      bool should_encode() const override { return false; }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      OID m_oid;
   };

}

}

#endif
