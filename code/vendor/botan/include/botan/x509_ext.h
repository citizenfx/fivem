/*
* X.509 Certificate Extensions
* (C) 1999-2007,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_X509_EXTENSIONS_H_
#define BOTAN_X509_EXTENSIONS_H_

#include <botan/asn1_obj.h>
#include <botan/asn1_oid.h>
#include <botan/asn1_alt_name.h>
#include <botan/cert_status.h>
#include <botan/name_constraint.h>
#include <botan/key_constraint.h>
#include <botan/crl_ent.h>
#include <set>

namespace Botan {

class Data_Store;
class X509_Certificate;

/**
* X.509 Certificate Extension
*/
class BOTAN_PUBLIC_API(2,0) Certificate_Extension
   {
   public:
      /**
      * @return OID representing this extension
      */
      virtual OID oid_of() const = 0;

      /*
      * @return specific OID name
      * If possible OIDS table should match oid_name to OIDS, ie
      * OIDS::lookup(ext->oid_name()) == ext->oid_of()
      * Should return empty string if OID is not known
      */
      virtual std::string oid_name() const = 0;

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

      virtual ~Certificate_Extension() = default;
   protected:
      friend class Extensions;
      virtual bool should_encode() const { return true; }
      virtual std::vector<uint8_t> encode_inner() const = 0;
      virtual void decode_inner(const std::vector<uint8_t>&) = 0;
   };

/**
* X.509 Certificate Extension List
*/
class BOTAN_PUBLIC_API(2,0) Extensions final : public ASN1_Object
   {
   public:
      /**
      * Look up an object in the extensions, based on OID Returns
      * nullptr if not set, if the extension was either absent or not
      * handled. The pointer returned is owned by the Extensions
      * object.
      * This would be better with an optional<T> return value
      */
      const Certificate_Extension* get_extension_object(const OID& oid) const;

      template<typename T>
      const T* get_extension_object_as(const OID& oid = T::static_oid()) const
         {
         if(const Certificate_Extension* extn = get_extension_object(oid))
            {
            // Unknown_Extension oid_name is empty
            if(extn->oid_name().empty())
               {
               return nullptr;
               }
            else if(const T* extn_as_T = dynamic_cast<const T*>(extn))
               {
               return extn_as_T;
               }
            else
               {
               throw Decoding_Error("Exception::get_extension_object_as dynamic_cast failed");
               }
            }

         return nullptr;
         }

      /**
      * Return the set of extensions in the order they appeared in the certificate
      * (or as they were added, if constructed)
      */
      const std::vector<OID>& get_extension_oids() const
         {
         return m_extension_oids;
         }

      /**
      * Return true if an extension was set
      */
      bool extension_set(const OID& oid) const;

      /**
      * Return true if an extesion was set and marked critical
      */
      bool critical_extension_set(const OID& oid) const;

      /**
      * Return the raw bytes of the extension
      * Will throw if OID was not set as an extension.
      */
      std::vector<uint8_t> get_extension_bits(const OID& oid) const;

      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;
      void contents_to(Data_Store&, Data_Store&) const;

      /**
      * Adds a new extension to the list.
      * @param extn pointer to the certificate extension (Extensions takes ownership)
      * @param critical whether this extension should be marked as critical
      * @throw Invalid_Argument if the extension is already present in the list
      */
      void add(Certificate_Extension* extn, bool critical = false);

      /**
      * Adds a new extension to the list unless it already exists. If the extension
      * already exists within the Extensions object, the extn pointer will be deleted.
      *
      * @param extn pointer to the certificate extension (Extensions takes ownership)
      * @param critical whether this extension should be marked as critical
      * @return true if the object was added false if the extension was already used
      */
      bool add_new(Certificate_Extension* extn, bool critical = false);

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
      * @return Copy of extension with oid, nullptr if not found.
      * Can avoid creating a copy by using get_extension_object function
      */
      std::unique_ptr<Certificate_Extension> get(const OID& oid) const;

      /**
      * Searches for an extension by OID and returns the result decoding
      * it to some arbitrary extension type chosen by the application.
      *
      * Only the unknown extensions, that is, extensions types that
      * are not declared in this header, are searched for by this
      * function.
      *
      * @return Pointer to new extension with oid, nullptr if not found.
      */
      template<typename T>
      std::unique_ptr<T> get_raw(const OID& oid) const
         {
         auto extn_info = m_extension_info.find(oid);

         if(extn_info != m_extension_info.end())
            {
            // Unknown_Extension oid_name is empty
            if(extn_info->second.obj().oid_name() == "")
               {
               std::unique_ptr<T> ext(new T);
               ext->decode_inner(extn_info->second.bits());
               return std::move(ext);
               }
            }
         return nullptr;
         }

      /**
      * Returns a copy of the list of extensions together with the corresponding
      * criticality flag. All extensions are encoded as some object, falling back
      * to Unknown_Extension class which simply allows reading the bytes as well
      * as the criticality flag.
      */
      std::vector<std::pair<std::unique_ptr<Certificate_Extension>, bool>> extensions() const;

      /**
      * Returns the list of extensions as raw, encoded bytes
      * together with the corresponding criticality flag.
      * Contains all extensions, including any extensions encoded as Unknown_Extension
      */
      std::map<OID, std::pair<std::vector<uint8_t>, bool>> extensions_raw() const;

      Extensions() {}

      Extensions(const Extensions&) = default;
      Extensions& operator=(const Extensions&) = default;

      Extensions(Extensions&&) = default;
      Extensions& operator=(Extensions&&) = default;

   private:
      static std::unique_ptr<Certificate_Extension>
         create_extn_obj(const OID& oid,
                         bool critical,
                         const std::vector<uint8_t>& body);

      class Extensions_Info
         {
         public:
            Extensions_Info(bool critical,
                            Certificate_Extension* ext) :
               m_obj(ext),
               m_bits(m_obj->encode_inner()),
               m_critical(critical)
               {
               }

            Extensions_Info(bool critical,
                            const std::vector<uint8_t>& encoding,
                            Certificate_Extension* ext) :
               m_obj(ext),
               m_bits(encoding),
               m_critical(critical)
               {
               }

            bool is_critical() const { return m_critical; }
            const std::vector<uint8_t>& bits() const { return m_bits; }
            const Certificate_Extension& obj() const
               {
               BOTAN_ASSERT_NONNULL(m_obj.get());
               return *m_obj.get();
               }

         private:
            std::shared_ptr<Certificate_Extension> m_obj;
            std::vector<uint8_t> m_bits;
            bool m_critical = false;
         };

      std::vector<OID> m_extension_oids;
      std::map<OID, Extensions_Info> m_extension_info;
   };

namespace Cert_Extension {

static const size_t NO_CERT_PATH_LIMIT = 0xFFFFFFF0;

/**
* Basic Constraints Extension
*/
class BOTAN_PUBLIC_API(2,0) Basic_Constraints final : public Certificate_Extension
   {
   public:
      Basic_Constraints* copy() const override
         { return new Basic_Constraints(m_is_ca, m_path_limit); }

      Basic_Constraints(bool ca = false, size_t limit = 0) :
         m_is_ca(ca), m_path_limit(limit) {}

      bool get_is_ca() const { return m_is_ca; }
      size_t get_path_limit() const;

      static OID static_oid() { return OID("2.5.29.19"); }
      OID oid_of() const override { return static_oid(); }

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
class BOTAN_PUBLIC_API(2,0) Key_Usage final : public Certificate_Extension
   {
   public:
      Key_Usage* copy() const override { return new Key_Usage(m_constraints); }

      explicit Key_Usage(Key_Constraints c = NO_CONSTRAINTS) : m_constraints(c) {}

      Key_Constraints get_constraints() const { return m_constraints; }

      static OID static_oid() { return OID("2.5.29.15"); }
      OID oid_of() const override { return static_oid(); }

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
class BOTAN_PUBLIC_API(2,0) Subject_Key_ID final : public Certificate_Extension
   {
   public:
      Subject_Key_ID() = default;

      explicit Subject_Key_ID(const std::vector<uint8_t>& k) : m_key_id(k) {}

      Subject_Key_ID(const std::vector<uint8_t>& public_key,
                     const std::string& hash_fn);

      Subject_Key_ID* copy() const override
         { return new Subject_Key_ID(m_key_id); }

      const std::vector<uint8_t>& get_key_id() const { return m_key_id; }

      static OID static_oid() { return OID("2.5.29.14"); }
      OID oid_of() const override { return static_oid(); }

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
class BOTAN_PUBLIC_API(2,0) Authority_Key_ID final : public Certificate_Extension
   {
   public:
      Authority_Key_ID* copy() const override
         { return new Authority_Key_ID(m_key_id); }

      Authority_Key_ID() = default;
      explicit Authority_Key_ID(const std::vector<uint8_t>& k) : m_key_id(k) {}

      const std::vector<uint8_t>& get_key_id() const { return m_key_id; }

      static OID static_oid() { return OID("2.5.29.35"); }
      OID oid_of() const override { return static_oid(); }

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
* Subject Alternative Name Extension
*/
class BOTAN_PUBLIC_API(2,4) Subject_Alternative_Name final : public Certificate_Extension
   {
   public:
      const AlternativeName& get_alt_name() const { return m_alt_name; }

      static OID static_oid() { return OID("2.5.29.17"); }
      OID oid_of() const override { return static_oid(); }

      Subject_Alternative_Name* copy() const override
         { return new Subject_Alternative_Name(get_alt_name()); }

      explicit Subject_Alternative_Name(const AlternativeName& name = AlternativeName()) :
         m_alt_name(name) {}

   private:
      std::string oid_name() const override { return "X509v3.SubjectAlternativeName"; }

      bool should_encode() const override { return m_alt_name.has_items(); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      AlternativeName m_alt_name;
   };

/**
* Issuer Alternative Name Extension
*/
class BOTAN_PUBLIC_API(2,0) Issuer_Alternative_Name final : public Certificate_Extension
   {
   public:
      const AlternativeName& get_alt_name() const { return m_alt_name; }

      static OID static_oid() { return OID("2.5.29.18"); }
      OID oid_of() const override { return static_oid(); }

      Issuer_Alternative_Name* copy() const override
         { return new Issuer_Alternative_Name(get_alt_name()); }

      explicit Issuer_Alternative_Name(const AlternativeName& name = AlternativeName()) :
         m_alt_name(name) {}

   private:
      std::string oid_name() const override { return "X509v3.IssuerAlternativeName"; }

      bool should_encode() const override { return m_alt_name.has_items(); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      AlternativeName m_alt_name;
   };

/**
* Extended Key Usage Extension
*/
class BOTAN_PUBLIC_API(2,0) Extended_Key_Usage final : public Certificate_Extension
   {
   public:
      Extended_Key_Usage* copy() const override
         { return new Extended_Key_Usage(m_oids); }

      Extended_Key_Usage() = default;
      explicit Extended_Key_Usage(const std::vector<OID>& o) : m_oids(o) {}

      const std::vector<OID>& get_oids() const { return m_oids; }

      static OID static_oid() { return OID("2.5.29.37"); }
      OID oid_of() const override { return static_oid(); }

   private:
      std::string oid_name() const override { return "X509v3.ExtendedKeyUsage"; }

      bool should_encode() const override { return (m_oids.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<OID> m_oids;
   };

/**
* Name Constraints
*/
class BOTAN_PUBLIC_API(2,0) Name_Constraints final : public Certificate_Extension
   {
   public:
      Name_Constraints* copy() const override
         { return new Name_Constraints(m_name_constraints); }

      Name_Constraints() = default;
      Name_Constraints(const NameConstraints &nc) : m_name_constraints(nc) {}

      void validate(const X509_Certificate& subject, const X509_Certificate& issuer,
            const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
            std::vector<std::set<Certificate_Status_Code>>& cert_status,
            size_t pos) override;

      const NameConstraints& get_name_constraints() const { return m_name_constraints; }

      static OID static_oid() { return OID("2.5.29.30"); }
      OID oid_of() const override { return static_oid(); }

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
class BOTAN_PUBLIC_API(2,0) Certificate_Policies final : public Certificate_Extension
   {
   public:
      Certificate_Policies* copy() const override
         { return new Certificate_Policies(m_oids); }

      Certificate_Policies() = default;
      explicit Certificate_Policies(const std::vector<OID>& o) : m_oids(o) {}

      BOTAN_DEPRECATED("Use get_policy_oids")
      std::vector<OID> get_oids() const { return m_oids; }

      const std::vector<OID>& get_policy_oids() const { return m_oids; }

      static OID static_oid() { return OID("2.5.29.32"); }
      OID oid_of() const override { return static_oid(); }

      void validate(const X509_Certificate& subject, const X509_Certificate& issuer,
            const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
            std::vector<std::set<Certificate_Status_Code>>& cert_status,
            size_t pos) override;
   private:
      std::string oid_name() const override
         { return "X509v3.CertificatePolicies"; }

      bool should_encode() const override { return (m_oids.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<OID> m_oids;
   };

/**
* Authority Information Access Extension
*/
class BOTAN_PUBLIC_API(2,0) Authority_Information_Access final : public Certificate_Extension
   {
   public:
      Authority_Information_Access* copy() const override
         { return new Authority_Information_Access(m_ocsp_responder, m_ca_issuers); }

      Authority_Information_Access() = default;

      explicit Authority_Information_Access(const std::string& ocsp, const std::vector<std::string>& ca_issuers = std::vector<std::string>()) :
         m_ocsp_responder(ocsp), m_ca_issuers(ca_issuers) {}

      std::string ocsp_responder() const { return m_ocsp_responder; }

      static OID static_oid() { return OID("1.3.6.1.5.5.7.1.1"); }
      OID oid_of() const override { return static_oid(); }
      const std::vector<std::string> ca_issuers() const { return m_ca_issuers; }

   private:
      std::string oid_name() const override
         { return "PKIX.AuthorityInformationAccess"; }

      bool should_encode() const override { return (!m_ocsp_responder.empty()); }

      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;

      void contents_to(Data_Store&, Data_Store&) const override;

      std::string m_ocsp_responder;
      std::vector<std::string> m_ca_issuers;
   };

/**
* CRL Number Extension
*/
class BOTAN_PUBLIC_API(2,0) CRL_Number final : public Certificate_Extension
   {
   public:
      CRL_Number* copy() const override;

      CRL_Number() : m_has_value(false), m_crl_number(0) {}
      CRL_Number(size_t n) : m_has_value(true), m_crl_number(n) {}

      size_t get_crl_number() const;

      static OID static_oid() { return OID("2.5.29.20"); }
      OID oid_of() const override { return static_oid(); }

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
class BOTAN_PUBLIC_API(2,0) CRL_ReasonCode final : public Certificate_Extension
   {
   public:
      CRL_ReasonCode* copy() const override
         { return new CRL_ReasonCode(m_reason); }

      explicit CRL_ReasonCode(CRL_Code r = UNSPECIFIED) : m_reason(r) {}

      CRL_Code get_reason() const { return m_reason; }

      static OID static_oid() { return OID("2.5.29.21"); }
      OID oid_of() const override { return static_oid(); }

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
* todo enforce restrictions from RFC 5280 4.2.1.13
*/
class BOTAN_PUBLIC_API(2,0) CRL_Distribution_Points final : public Certificate_Extension
   {
   public:
      class BOTAN_PUBLIC_API(2,0) Distribution_Point final : public ASN1_Object
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

      CRL_Distribution_Points() = default;

      explicit CRL_Distribution_Points(const std::vector<Distribution_Point>& points) :
         m_distribution_points(points) {}

      const std::vector<Distribution_Point>& distribution_points() const
         { return m_distribution_points; }

      const std::vector<std::string>& crl_distribution_urls() const
         { return m_crl_distribution_urls; }

      static OID static_oid() { return OID("2.5.29.31"); }
      OID oid_of() const override { return static_oid(); }

   private:
      std::string oid_name() const override
         { return "X509v3.CRLDistributionPoints"; }

      bool should_encode() const override
         { return !m_distribution_points.empty(); }

      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<Distribution_Point> m_distribution_points;
      std::vector<std::string> m_crl_distribution_urls;
   };

/**
* CRL Issuing Distribution Point Extension
* todo enforce restrictions from RFC 5280 5.2.5
*/
class CRL_Issuing_Distribution_Point final : public Certificate_Extension
   {
   public:
      CRL_Issuing_Distribution_Point() = default;

      explicit CRL_Issuing_Distribution_Point(const CRL_Distribution_Points::Distribution_Point& distribution_point) :
         m_distribution_point(distribution_point) {}

      CRL_Issuing_Distribution_Point* copy() const override
         { return new CRL_Issuing_Distribution_Point(m_distribution_point); }

      const AlternativeName& get_point() const
         { return m_distribution_point.point(); }

      static OID static_oid() { return OID("2.5.29.28"); }
      OID oid_of() const override { return static_oid(); }

   private:
      std::string oid_name() const override
         { return "X509v3.CRLIssuingDistributionPoint"; }

      bool should_encode() const override { return true; }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      CRL_Distribution_Points::Distribution_Point m_distribution_point;
   };

/**
* An unknown X.509 extension
* Will add a failure to the path validation result, if critical
*/
class BOTAN_PUBLIC_API(2,4) Unknown_Extension final : public Certificate_Extension
   {
   public:
      Unknown_Extension(const OID& oid, bool critical) :
         m_oid(oid), m_critical(critical) {}

      Unknown_Extension* copy() const override
         { return new Unknown_Extension(m_oid, m_critical); }

      /**
      * Return the OID of this unknown extension
      */
      OID oid_of() const override
         { return m_oid; }

      //static_oid not defined for Unknown_Extension

      /**
      * Return the extension contents
      */
      const std::vector<uint8_t>& extension_contents() const { return m_bytes; }

      /**
      * Return if this extension was marked critical
      */
      bool is_critical_extension() const { return m_critical; }

      void validate(const X509_Certificate&, const X509_Certificate&,
            const std::vector<std::shared_ptr<const X509_Certificate>>&,
            std::vector<std::set<Certificate_Status_Code>>& cert_status,
            size_t pos) override
         {
         if(m_critical)
            {
            cert_status.at(pos).insert(Certificate_Status_Code::UNKNOWN_CRITICAL_EXTENSION);
            }
         }

   private:
      std::string oid_name() const override { return ""; }

      bool should_encode() const override { return true; }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      OID m_oid;
      bool m_critical;
      std::vector<uint8_t> m_bytes;
   };

   }

}

#endif
