/*
* NIST elliptic curves over GF(p)
* (C) 2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_GFP_CURVE_NIST_H__
#define BOTAN_GFP_CURVE_NIST_H__

#include <botan/curve_gfp.h>
#include <memory>

namespace Botan {

class CurveGFp_NIST : public CurveGFp_Repr
   {
   public:
      CurveGFp_NIST(size_t p_bits, const BigInt& a, const BigInt& b) :
         m_a(a), m_b(b), m_p_words((p_bits + BOTAN_MP_WORD_BITS - 1) / BOTAN_MP_WORD_BITS)
         {
         }

      size_t get_p_words() const override { return m_p_words; }

      const BigInt& get_a() const override { return m_a; }

      const BigInt& get_b() const override { return m_b; }

      const BigInt& get_a_rep() const override { return m_a; }

      const BigInt& get_b_rep() const override { return m_b; }

      void to_curve_rep(BigInt& x, secure_vector<word>& ws) const override
         { redc(x, ws); }

      void from_curve_rep(BigInt& x, secure_vector<word>& ws) const override
         { redc(x, ws); }

      void curve_mul(BigInt& z, const BigInt& x, const BigInt& y,
                     secure_vector<word>& ws) const override;

      void curve_sqr(BigInt& z, const BigInt& x,
                     secure_vector<word>& ws) const override;
   private:
      virtual void redc(BigInt& x, secure_vector<word>& ws) const = 0;

      virtual size_t max_redc_subtractions() const = 0;

      // Curve parameters
      BigInt m_a, m_b;
      size_t m_p_words; // cache of m_p.sig_words()
   };

#if (BOTAN_MP_WORD_BITS == 32) || (BOTAN_MP_WORD_BITS == 64)

#define BOTAN_HAS_CURVEGFP_NISTP_M32

/**
* The NIST P-192 curve
*/
class CurveGFp_P192 : public CurveGFp_NIST
   {
   public:
      CurveGFp_P192(const BigInt& a, const BigInt& b) : CurveGFp_NIST(192, a, b) {}

      static const BigInt& prime();

      const BigInt& get_p() const override { return CurveGFp_P192::prime(); }

   private:
      void redc(BigInt& x, secure_vector<word>& ws) const override;

      size_t max_redc_subtractions() const override { return 3; }
   };

/**
* The NIST P-224 curve
*/
class CurveGFp_P224 : public CurveGFp_NIST
   {
   public:
      CurveGFp_P224(const BigInt& a, const BigInt& b) : CurveGFp_NIST(224, a, b) {}

      static const BigInt& prime();

      const BigInt& get_p() const override { return CurveGFp_P224::prime(); }
   private:
      void redc(BigInt& x, secure_vector<word>& ws) const override;

      size_t max_redc_subtractions() const override { return 3; }
   };

/**
* The NIST P-256 curve
*/
class CurveGFp_P256 : public CurveGFp_NIST
   {
   public:
      CurveGFp_P256(const BigInt& a, const BigInt& b) : CurveGFp_NIST(256, a, b) {}

      static const BigInt& prime();

      const BigInt& get_p() const override { return CurveGFp_P256::prime(); }

   private:
      void redc(BigInt& x, secure_vector<word>& ws) const override;

      size_t max_redc_subtractions() const override { return 10; }
   };

/**
* The NIST P-384 curve
*/
class CurveGFp_P384 : public CurveGFp_NIST
   {
   public:
      CurveGFp_P384(const BigInt& a, const BigInt& b) : CurveGFp_NIST(384, a, b) {}

      static const BigInt& prime();

      const BigInt& get_p() const override { return CurveGFp_P384::prime(); }

   private:
      void redc(BigInt& x, secure_vector<word>& ws) const override;

      size_t max_redc_subtractions() const override { return 4; }
   };

#endif

/**
* The NIST P-521 curve
*/
class CurveGFp_P521 : public CurveGFp_NIST
   {
   public:
      CurveGFp_P521(const BigInt& a, const BigInt& b) : CurveGFp_NIST(521, a, b) {}

      static const BigInt& prime();

      const BigInt& get_p() const override { return CurveGFp_P521::prime(); }

   private:
      void redc(BigInt& x, secure_vector<word>& ws) const override;

      size_t max_redc_subtractions() const override { return 1; }
   };

}

#endif
