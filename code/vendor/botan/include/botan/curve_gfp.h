/*
* Elliptic curves over GF(p)
*
* (C) 2007 Martin Doering, Christoph Ludwig, Falko Strenzke
*     2010-2011,2012,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_GFP_CURVE_H__
#define BOTAN_GFP_CURVE_H__

#include <botan/numthry.h>
#include <memory>

namespace Botan {

class CurveGFp_Repr
   {
   public:
      virtual ~CurveGFp_Repr() {}

      virtual const BigInt& get_p() const = 0;
      virtual const BigInt& get_a() const = 0;
      virtual const BigInt& get_b() const = 0;

      virtual size_t get_p_words() const = 0;

      /*
      * Returns to_curve_rep(get_a())
      */
      virtual const BigInt& get_a_rep() const = 0;

      /*
      * Returns to_curve_rep(get_b())
      */
      virtual const BigInt& get_b_rep() const = 0;

      virtual void to_curve_rep(BigInt& x, secure_vector<word>& ws) const = 0;

      virtual void from_curve_rep(BigInt& x, secure_vector<word>& ws) const = 0;

      virtual void curve_mul(BigInt& z, const BigInt& x, const BigInt& y,
                             secure_vector<word>& ws) const = 0;

      virtual void curve_sqr(BigInt& z, const BigInt& x,
                             secure_vector<word>& ws) const = 0;
   };

/**
* This class represents an elliptic curve over GF(p)
*/
class BOTAN_DLL CurveGFp
   {
   public:

      /**
      * Create an uninitialized CurveGFp
      */
      CurveGFp() {}

      /**
      * Construct the elliptic curve E: y^2 = x^3 + ax + b over GF(p)
      * @param p prime number of the field
      * @param a first coefficient
      * @param b second coefficient
      */
      CurveGFp(const BigInt& p, const BigInt& a, const BigInt& b) :
         m_repr(choose_repr(p, a, b))
         {
         }

      CurveGFp(const CurveGFp&) = default;

      CurveGFp& operator=(const CurveGFp&) = default;

      /**
      * @return curve coefficient a
      */
      const BigInt& get_a() const { return m_repr->get_a(); }

      /**
      * @return curve coefficient b
      */
      const BigInt& get_b() const { return m_repr->get_b(); }

      /**
      * Get prime modulus of the field of the curve
      * @return prime modulus of the field of the curve
      */
      const BigInt& get_p() const { return m_repr->get_p(); }

      const BigInt& get_a_rep() const { return m_repr->get_a_rep(); }

      const BigInt& get_b_rep() const { return m_repr->get_b_rep(); }

      void to_rep(BigInt& x, secure_vector<word>& ws) const
         {
         m_repr->to_curve_rep(x, ws);
         }

      void from_rep(BigInt& x, secure_vector<word>& ws) const
         {
         m_repr->from_curve_rep(x, ws);
         }

      BigInt from_rep(const BigInt& x, secure_vector<word>& ws) const
         {
         BigInt xt(x);
         m_repr->from_curve_rep(xt, ws);
         return xt;
         }

      // TODO: from_rep taking && ref

      void mul(BigInt& z, const BigInt& x, const BigInt& y, secure_vector<word>& ws) const
         {
         m_repr->curve_mul(z, x, y, ws);
         }

      BigInt mul(const BigInt& x, const BigInt& y, secure_vector<word>& ws) const
         {
         BigInt z;
         m_repr->curve_mul(z, x, y, ws);
         return z;
         }

      void sqr(BigInt& z, const BigInt& x, secure_vector<word>& ws) const
         {
         m_repr->curve_sqr(z, x, ws);
         }

      BigInt sqr(const BigInt& x, secure_vector<word>& ws) const
         {
         BigInt z;
         m_repr->curve_sqr(z, x, ws);
         return z;
         }

      void swap(CurveGFp& other)
         {
         std::swap(m_repr, other.m_repr);
         }

   private:
      static std::shared_ptr<CurveGFp_Repr>
         choose_repr(const BigInt& p, const BigInt& a, const BigInt& b);

      std::shared_ptr<CurveGFp_Repr> m_repr;
   };

/**
* Equality operator
* @param lhs a curve
* @param rhs a curve
* @return true iff lhs is the same as rhs
*/
inline bool operator==(const CurveGFp& lhs, const CurveGFp& rhs)
   {
   return (lhs.get_p() == rhs.get_p()) &&
          (lhs.get_a() == rhs.get_a()) &&
          (lhs.get_b() == rhs.get_b());
   }

inline bool operator!=(const CurveGFp& lhs, const CurveGFp& rhs)
   {
   return !(lhs == rhs);
   }

}

namespace std {

template<> inline
void swap<Botan::CurveGFp>(Botan::CurveGFp& curve1,
                           Botan::CurveGFp& curve2) BOTAN_NOEXCEPT
   {
   curve1.swap(curve2);
   }

} // namespace std

#endif
