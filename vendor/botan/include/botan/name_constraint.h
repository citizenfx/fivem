/*
* X.509 Name Constraint
* (C) 2015 Kai Michaelis
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_NAME_CONSTRAINT_H__
#define BOTAN_NAME_CONSTRAINT_H__

#include <botan/asn1_obj.h>
#include <ostream>

namespace Botan {

class X509_Certificate;

/**
* @brief X.509 GeneralName Type
*
* Handles parsing GeneralName types in their BER and canonical string
* encoding. Allows matching GeneralNames against each other using
* the rules laid out in the RFC 5280, sec. 4.2.1.10 (Name Contraints).
*/
class BOTAN_DLL GeneralName : public ASN1_Object
   {
   public:
      enum MatchResult : int
            {
            All,
            Some,
            None,
            NotFound,
            UnknownType,
            };

      /**
      * Creates an empty GeneralName.
      */
      GeneralName() : m_type(), m_name() {}

      /**
      * Creates a new GeneralName for its string format.
      * @param str type and name, colon-separated, e.g., "DNS:google.com"
      */
      GeneralName(const std::string& str);

      void encode_into(class DER_Encoder&) const override;

      void decode_from(class BER_Decoder&) override;

      /**
      * @return Type of the name. Can be DN, DNS, IP, RFC822 or URI.
      */
      const std::string& type() const { return m_type; }

      /**
      * @return The name as string. Format depends on type.
      */
      const std::string& name() const { return m_name; }

      /**
      * Checks whether a given certificate (partially) matches this name.
      * @param cert certificate to be matched
      * @return the match result
      */
      MatchResult matches(const X509_Certificate& cert) const;

   private:
      std::string m_type;
      std::string m_name;

      bool matches_dns(const std::string&) const;
      bool matches_dn(const std::string&) const;
      bool matches_ip(const std::string&) const;
   };

std::ostream& operator<<(std::ostream& os, const GeneralName& gn);

/**
* @brief A single Name Constraint
*
* The Name Constraint extension adds a minimum and maximum path
* length to a GeneralName to form a constraint. The length limits
* are currently unused.
*/
class BOTAN_DLL GeneralSubtree : public ASN1_Object
   {
   public:
      /**
      * Creates an empty name constraint.
      */
      GeneralSubtree() : m_base(), m_minimum(0), m_maximum(std::numeric_limits<std::size_t>::max())
      {}

      /***
      * Creates a new name constraint.
      * @param base name
      * @param min minimum path length
      * @param max maximum path length
      */
      GeneralSubtree(GeneralName base, size_t min, size_t max)
      : m_base(base), m_minimum(min), m_maximum(max)
      {}

      /**
      * Creates a new name constraint for its string format.
      * @param str name constraint
      */
      GeneralSubtree(const std::string& str);

      void encode_into(class DER_Encoder&) const override;

      void decode_from(class BER_Decoder&) override;

      /**
      * @return name
      */
      GeneralName base() const { return m_base; }

      /**
      * @return minimum path length
      */
      size_t minimum() const { return m_minimum; }

      /**
      * @return maximum path length
      */
      size_t maximum() const { return m_maximum; }

   private:
      GeneralName m_base;
      size_t m_minimum;
      size_t m_maximum;
   };

std::ostream& operator<<(std::ostream& os, const GeneralSubtree& gs);

/**
* @brief Name Constraints
*
* Wraps the Name Constraints associated with a certificate.
*/
class BOTAN_DLL NameConstraints
   {
   public:
      /**
      * Creates an empty name NameConstraints.
      */
      NameConstraints() : m_permitted_subtrees(), m_excluded_subtrees() {}

      /**
      * Creates NameConstraints from a list of permitted and excluded subtrees.
      * @param permitted_subtrees names for which the certificate is permitted
      * @param excluded_subtrees names for which the certificate is not permitted
      */
      NameConstraints(std::vector<GeneralSubtree>&& permitted_subtrees,
                    std::vector<GeneralSubtree>&& excluded_subtrees)
      : m_permitted_subtrees(permitted_subtrees), m_excluded_subtrees(excluded_subtrees)
      {}

      /**
      * @return permitted names
      */
      const std::vector<GeneralSubtree>& permitted() const { return m_permitted_subtrees; }

      /**
      * @return excluded names
      */
      const std::vector<GeneralSubtree>& excluded() const { return m_excluded_subtrees; }

   private:
      std::vector<GeneralSubtree> m_permitted_subtrees;
      std::vector<GeneralSubtree> m_excluded_subtrees;
};

}

#endif
