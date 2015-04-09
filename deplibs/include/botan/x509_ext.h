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
#include <botan/datastor.h>
#include <botan/crl_ent.h>

namespace Botan {

/**
* X.509 Certificate Extension
*/
class BOTAN_DLL Certificate_Extension
   {
   public:
      /**
      * @return OID representing this extension
      */
      OID oid_of() const;

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

      virtual ~Certificate_Extension() {}
   protected:
      friend class Extensions;
      virtual bool should_encode() const { return true; }
      virtual std::vector<byte> encode_inner() const = 0;
      virtual void decode_inner(const std::vector<byte>&) = 0;
   };

/**
* X.509 Certificate Extension List
*/
class BOTAN_DLL Extensions : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const;
      void decode_from(class BER_Decoder&);

      void contents_to(Data_Store&, Data_Store&) const;

      void add(Certificate_Extension* extn, bool critical = false);

      Extensions& operator=(const Extensions&);

      Extensions(const Extensions&);
      Extensions(bool st = true) : m_throw_on_unknown_critical(st) {}
      ~Extensions();
   private:
      static Certificate_Extension* get_extension(const OID&);

      std::vector<std::pair<Certificate_Extension*, bool> > extensions;
      bool m_throw_on_unknown_critical;
   };

namespace Cert_Extension {

static const size_t NO_CERT_PATH_LIMIT = 0xFFFFFFF0;

/**
* Basic Constraints Extension
*/
class BOTAN_DLL Basic_Constraints : public Certificate_Extension
   {
   public:
      Basic_Constraints* copy() const
         { return new Basic_Constraints(is_ca, path_limit); }

      Basic_Constraints(bool ca = false, size_t limit = 0) :
         is_ca(ca), path_limit(limit) {}

      bool get_is_ca() const { return is_ca; }
      size_t get_path_limit() const;
   private:
      std::string oid_name() const { return "X509v3.BasicConstraints"; }

      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      bool is_ca;
      size_t path_limit;
   };

/**
* Key Usage Constraints Extension
*/
class BOTAN_DLL Key_Usage : public Certificate_Extension
   {
   public:
      Key_Usage* copy() const { return new Key_Usage(constraints); }

      Key_Usage(Key_Constraints c = NO_CONSTRAINTS) : constraints(c) {}

      Key_Constraints get_constraints() const { return constraints; }
   private:
      std::string oid_name() const { return "X509v3.KeyUsage"; }

      bool should_encode() const { return (constraints != NO_CONSTRAINTS); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      Key_Constraints constraints;
   };

/**
* Subject Key Identifier Extension
*/
class BOTAN_DLL Subject_Key_ID : public Certificate_Extension
   {
   public:
      Subject_Key_ID* copy() const { return new Subject_Key_ID(key_id); }

      Subject_Key_ID() {}
      Subject_Key_ID(const std::vector<byte>&);

      std::vector<byte> get_key_id() const { return key_id; }
   private:
      std::string oid_name() const { return "X509v3.SubjectKeyIdentifier"; }

      bool should_encode() const { return (key_id.size() > 0); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<byte> key_id;
   };

/**
* Authority Key Identifier Extension
*/
class BOTAN_DLL Authority_Key_ID : public Certificate_Extension
   {
   public:
      Authority_Key_ID* copy() const { return new Authority_Key_ID(key_id); }

      Authority_Key_ID() {}
      Authority_Key_ID(const std::vector<byte>& k) : key_id(k) {}

      std::vector<byte> get_key_id() const { return key_id; }
   private:
      std::string oid_name() const { return "X509v3.AuthorityKeyIdentifier"; }

      bool should_encode() const { return (key_id.size() > 0); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<byte> key_id;
   };

/**
* Alternative Name Extension Base Class
*/
class BOTAN_DLL Alternative_Name : public Certificate_Extension
   {
   public:
      AlternativeName get_alt_name() const { return alt_name; }

   protected:
      Alternative_Name(const AlternativeName&, const std::string& oid_name);

      Alternative_Name(const std::string&, const std::string&);
   private:
      std::string oid_name() const { return oid_name_str; }

      bool should_encode() const { return alt_name.has_items(); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::string oid_name_str;
      AlternativeName alt_name;
   };

/**
* Subject Alternative Name Extension
*/
class BOTAN_DLL Subject_Alternative_Name : public Alternative_Name
   {
   public:
      Subject_Alternative_Name* copy() const
         { return new Subject_Alternative_Name(get_alt_name()); }

      Subject_Alternative_Name(const AlternativeName& = AlternativeName());
   };

/**
* Issuer Alternative Name Extension
*/
class BOTAN_DLL Issuer_Alternative_Name : public Alternative_Name
   {
   public:
      Issuer_Alternative_Name* copy() const
         { return new Issuer_Alternative_Name(get_alt_name()); }

      Issuer_Alternative_Name(const AlternativeName& = AlternativeName());
   };

/**
* Extended Key Usage Extension
*/
class BOTAN_DLL Extended_Key_Usage : public Certificate_Extension
   {
   public:
      Extended_Key_Usage* copy() const { return new Extended_Key_Usage(oids); }

      Extended_Key_Usage() {}
      Extended_Key_Usage(const std::vector<OID>& o) : oids(o) {}

      std::vector<OID> get_oids() const { return oids; }
   private:
      std::string oid_name() const { return "X509v3.ExtendedKeyUsage"; }

      bool should_encode() const { return (oids.size() > 0); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<OID> oids;
   };

/**
* Certificate Policies Extension
*/
class BOTAN_DLL Certificate_Policies : public Certificate_Extension
   {
   public:
      Certificate_Policies* copy() const
         { return new Certificate_Policies(oids); }

      Certificate_Policies() {}
      Certificate_Policies(const std::vector<OID>& o) : oids(o) {}

      std::vector<OID> get_oids() const { return oids; }
   private:
      std::string oid_name() const { return "X509v3.CertificatePolicies"; }

      bool should_encode() const { return (oids.size() > 0); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<OID> oids;
   };

class BOTAN_DLL Authority_Information_Access : public Certificate_Extension
   {
   public:
      Authority_Information_Access* copy() const
         { return new Authority_Information_Access(m_ocsp_responder); }

      Authority_Information_Access() {}

      Authority_Information_Access(const std::string& ocsp) :
         m_ocsp_responder(ocsp) {}

   private:
      std::string oid_name() const { return "PKIX.AuthorityInformationAccess"; }

      bool should_encode() const { return (m_ocsp_responder != ""); }

      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);

      void contents_to(Data_Store&, Data_Store&) const;

      std::string m_ocsp_responder;
   };

/**
* CRL Number Extension
*/
class BOTAN_DLL CRL_Number : public Certificate_Extension
   {
   public:
      CRL_Number* copy() const;

      CRL_Number() : has_value(false), crl_number(0) {}
      CRL_Number(size_t n) : has_value(true), crl_number(n) {}

      size_t get_crl_number() const;
   private:
      std::string oid_name() const { return "X509v3.CRLNumber"; }

      bool should_encode() const { return has_value; }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      bool has_value;
      size_t crl_number;
   };

/**
* CRL Entry Reason Code Extension
*/
class BOTAN_DLL CRL_ReasonCode : public Certificate_Extension
   {
   public:
      CRL_ReasonCode* copy() const { return new CRL_ReasonCode(reason); }

      CRL_ReasonCode(CRL_Code r = UNSPECIFIED) : reason(r) {}

      CRL_Code get_reason() const { return reason; }
   private:
      std::string oid_name() const { return "X509v3.ReasonCode"; }

      bool should_encode() const { return (reason != UNSPECIFIED); }
      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      CRL_Code reason;
   };

/**
* CRL Distribution Points Extension
*/
class BOTAN_DLL CRL_Distribution_Points : public Certificate_Extension
   {
   public:
      class BOTAN_DLL Distribution_Point : public ASN1_Object
         {
         public:
            void encode_into(class DER_Encoder&) const;
            void decode_from(class BER_Decoder&);

            const AlternativeName& point() const { return m_point; }
         private:
            AlternativeName m_point;
         };

      CRL_Distribution_Points* copy() const
         { return new CRL_Distribution_Points(m_distribution_points); }

      CRL_Distribution_Points() {}

      CRL_Distribution_Points(const std::vector<Distribution_Point>& points) :
         m_distribution_points(points) {}

      std::vector<Distribution_Point> distribution_points() const
         { return m_distribution_points; }

   private:
      std::string oid_name() const { return "X509v3.CRLDistributionPoints"; }

      bool should_encode() const { return !m_distribution_points.empty(); }

      std::vector<byte> encode_inner() const;
      void decode_inner(const std::vector<byte>&);
      void contents_to(Data_Store&, Data_Store&) const;

      std::vector<Distribution_Point> m_distribution_points;
   };

}

}

#endif
