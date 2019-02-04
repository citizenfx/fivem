/*
* Botan 2.10.0 Amalgamation
* (C) 1999-2018 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AMALGAMATION_INTERNAL_H_
#define BOTAN_AMALGAMATION_INTERNAL_H_

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


namespace Botan {

/**
Barrier implements a barrier synchronization primitive. wait() will
indicate how many threads to synchronize; each thread needing
synchronization should call sync(). When sync() returns, the barrier
is reset to zero, and the m_syncs counter is incremented. m_syncs is a
counter to ensure that wait() can be called after a sync() even if the
previously sleeping threads have not awoken.)
*/
class Barrier final
   {
   public:
      explicit Barrier(int value = 0) : m_value(value), m_syncs(0) {}

      void wait(size_t delta);

      void sync();

   private:
      int m_value;
      size_t m_syncs;
      mutex_type m_mutex;
      std::condition_variable m_cond;
   };

}

namespace Botan {

/**
* If top bit of arg is set, return ~0. Otherwise return 0.
*/
template<typename T>
inline T expand_top_bit(T a)
   {
   return static_cast<T>(0) - (a >> (sizeof(T)*8-1));
   }

/**
* If arg is zero, return ~0. Otherwise return 0
*/
template<typename T>
inline T ct_is_zero(T x)
   {
   return expand_top_bit<T>(~x & (x - 1));
   }

/**
* Power of 2 test. T should be an unsigned integer type
* @param arg an integer value
* @return true iff arg is 2^n for some n > 0
*/
template<typename T>
inline constexpr bool is_power_of_2(T arg)
   {
   return (arg != 0) && (arg != 1) && ((arg & static_cast<T>(arg-1)) == 0);
   }

/**
* Return the index of the highest set bit
* T is an unsigned integer type
* @param n an integer value
* @return index of the highest set bit in n
*/
template<typename T>
inline size_t high_bit(T n)
   {
   size_t hb = 0;

   for(size_t s = 8*sizeof(T) / 2; s > 0; s /= 2)
      {
      const size_t z = s * ((~ct_is_zero(n >> s)) & 1);
      hb += z;
      n >>= z;
      }

   hb += n;

   return hb;
   }

/**
* Return the number of significant bytes in n
* @param n an integer value
* @return number of significant bytes in n
*/
template<typename T>
inline size_t significant_bytes(T n)
   {
   size_t b = 0;

   for(size_t s = 8*sizeof(n) / 2; s >= 8; s /= 2)
      {
      const size_t z = s * (~ct_is_zero(n >> s) & 1);
      b += z/8;
      n >>= z;
      }

   b += (n != 0);

   return b;
   }

/**
* Count the trailing zero bits in n
* @param n an integer value
* @return maximum x st 2^x divides n
*/
template<typename T>
inline size_t ctz(T n)
   {
   /*
   * If n == 0 then this function will compute 8*sizeof(T)-1, so
   * initialize lb to 1 if n == 0 to produce the expected result.
   */
   size_t lb = ct_is_zero(n) & 1;

   for(size_t s = 8*sizeof(T) / 2; s > 0; s /= 2)
      {
      const T mask = (static_cast<T>(1) << s) - 1;
      const size_t z = s * (ct_is_zero(n & mask) & 1);
      lb += z;
      n >>= z;
      }

   return lb;
   }

template<typename T>
size_t ceil_log2(T x)
   {
   if(x >> (sizeof(T)*8-1))
      return sizeof(T)*8;

   size_t result = 0;
   T compare = 1;

   while(compare < x)
      {
      compare <<= 1;
      result++;
      }

   return result;
   }

// Potentially variable time ctz used for OCB
inline size_t var_ctz32(uint32_t n)
   {
#if defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG)
   if(n == 0)
      return 32;
   return __builtin_ctz(n);
#else
   return ctz<uint32_t>(n);
#endif
   }

}

namespace Botan {

const uint32_t CAST_SBOX1[256] = {
   0x30FB40D4, 0x9FA0FF0B, 0x6BECCD2F, 0x3F258C7A, 0x1E213F2F, 0x9C004DD3,
   0x6003E540, 0xCF9FC949, 0xBFD4AF27, 0x88BBBDB5, 0xE2034090, 0x98D09675,
   0x6E63A0E0, 0x15C361D2, 0xC2E7661D, 0x22D4FF8E, 0x28683B6F, 0xC07FD059,
   0xFF2379C8, 0x775F50E2, 0x43C340D3, 0xDF2F8656, 0x887CA41A, 0xA2D2BD2D,
   0xA1C9E0D6, 0x346C4819, 0x61B76D87, 0x22540F2F, 0x2ABE32E1, 0xAA54166B,
   0x22568E3A, 0xA2D341D0, 0x66DB40C8, 0xA784392F, 0x004DFF2F, 0x2DB9D2DE,
   0x97943FAC, 0x4A97C1D8, 0x527644B7, 0xB5F437A7, 0xB82CBAEF, 0xD751D159,
   0x6FF7F0ED, 0x5A097A1F, 0x827B68D0, 0x90ECF52E, 0x22B0C054, 0xBC8E5935,
   0x4B6D2F7F, 0x50BB64A2, 0xD2664910, 0xBEE5812D, 0xB7332290, 0xE93B159F,
   0xB48EE411, 0x4BFF345D, 0xFD45C240, 0xAD31973F, 0xC4F6D02E, 0x55FC8165,
   0xD5B1CAAD, 0xA1AC2DAE, 0xA2D4B76D, 0xC19B0C50, 0x882240F2, 0x0C6E4F38,
   0xA4E4BFD7, 0x4F5BA272, 0x564C1D2F, 0xC59C5319, 0xB949E354, 0xB04669FE,
   0xB1B6AB8A, 0xC71358DD, 0x6385C545, 0x110F935D, 0x57538AD5, 0x6A390493,
   0xE63D37E0, 0x2A54F6B3, 0x3A787D5F, 0x6276A0B5, 0x19A6FCDF, 0x7A42206A,
   0x29F9D4D5, 0xF61B1891, 0xBB72275E, 0xAA508167, 0x38901091, 0xC6B505EB,
   0x84C7CB8C, 0x2AD75A0F, 0x874A1427, 0xA2D1936B, 0x2AD286AF, 0xAA56D291,
   0xD7894360, 0x425C750D, 0x93B39E26, 0x187184C9, 0x6C00B32D, 0x73E2BB14,
   0xA0BEBC3C, 0x54623779, 0x64459EAB, 0x3F328B82, 0x7718CF82, 0x59A2CEA6,
   0x04EE002E, 0x89FE78E6, 0x3FAB0950, 0x325FF6C2, 0x81383F05, 0x6963C5C8,
   0x76CB5AD6, 0xD49974C9, 0xCA180DCF, 0x380782D5, 0xC7FA5CF6, 0x8AC31511,
   0x35E79E13, 0x47DA91D0, 0xF40F9086, 0xA7E2419E, 0x31366241, 0x051EF495,
   0xAA573B04, 0x4A805D8D, 0x548300D0, 0x00322A3C, 0xBF64CDDF, 0xBA57A68E,
   0x75C6372B, 0x50AFD341, 0xA7C13275, 0x915A0BF5, 0x6B54BFAB, 0x2B0B1426,
   0xAB4CC9D7, 0x449CCD82, 0xF7FBF265, 0xAB85C5F3, 0x1B55DB94, 0xAAD4E324,
   0xCFA4BD3F, 0x2DEAA3E2, 0x9E204D02, 0xC8BD25AC, 0xEADF55B3, 0xD5BD9E98,
   0xE31231B2, 0x2AD5AD6C, 0x954329DE, 0xADBE4528, 0xD8710F69, 0xAA51C90F,
   0xAA786BF6, 0x22513F1E, 0xAA51A79B, 0x2AD344CC, 0x7B5A41F0, 0xD37CFBAD,
   0x1B069505, 0x41ECE491, 0xB4C332E6, 0x032268D4, 0xC9600ACC, 0xCE387E6D,
   0xBF6BB16C, 0x6A70FB78, 0x0D03D9C9, 0xD4DF39DE, 0xE01063DA, 0x4736F464,
   0x5AD328D8, 0xB347CC96, 0x75BB0FC3, 0x98511BFB, 0x4FFBCC35, 0xB58BCF6A,
   0xE11F0ABC, 0xBFC5FE4A, 0xA70AEC10, 0xAC39570A, 0x3F04442F, 0x6188B153,
   0xE0397A2E, 0x5727CB79, 0x9CEB418F, 0x1CACD68D, 0x2AD37C96, 0x0175CB9D,
   0xC69DFF09, 0xC75B65F0, 0xD9DB40D8, 0xEC0E7779, 0x4744EAD4, 0xB11C3274,
   0xDD24CB9E, 0x7E1C54BD, 0xF01144F9, 0xD2240EB1, 0x9675B3FD, 0xA3AC3755,
   0xD47C27AF, 0x51C85F4D, 0x56907596, 0xA5BB15E6, 0x580304F0, 0xCA042CF1,
   0x011A37EA, 0x8DBFAADB, 0x35BA3E4A, 0x3526FFA0, 0xC37B4D09, 0xBC306ED9,
   0x98A52666, 0x5648F725, 0xFF5E569D, 0x0CED63D0, 0x7C63B2CF, 0x700B45E1,
   0xD5EA50F1, 0x85A92872, 0xAF1FBDA7, 0xD4234870, 0xA7870BF3, 0x2D3B4D79,
   0x42E04198, 0x0CD0EDE7, 0x26470DB8, 0xF881814C, 0x474D6AD7, 0x7C0C5E5C,
   0xD1231959, 0x381B7298, 0xF5D2F4DB, 0xAB838653, 0x6E2F1E23, 0x83719C9E,
   0xBD91E046, 0x9A56456E, 0xDC39200C, 0x20C8C571, 0x962BDA1C, 0xE1E696FF,
   0xB141AB08, 0x7CCA89B9, 0x1A69E783, 0x02CC4843, 0xA2F7C579, 0x429EF47D,
   0x427B169C, 0x5AC9F049, 0xDD8F0F00, 0x5C8165BF };

const uint32_t CAST_SBOX2[256] = {
   0x1F201094, 0xEF0BA75B, 0x69E3CF7E, 0x393F4380, 0xFE61CF7A, 0xEEC5207A,
   0x55889C94, 0x72FC0651, 0xADA7EF79, 0x4E1D7235, 0xD55A63CE, 0xDE0436BA,
   0x99C430EF, 0x5F0C0794, 0x18DCDB7D, 0xA1D6EFF3, 0xA0B52F7B, 0x59E83605,
   0xEE15B094, 0xE9FFD909, 0xDC440086, 0xEF944459, 0xBA83CCB3, 0xE0C3CDFB,
   0xD1DA4181, 0x3B092AB1, 0xF997F1C1, 0xA5E6CF7B, 0x01420DDB, 0xE4E7EF5B,
   0x25A1FF41, 0xE180F806, 0x1FC41080, 0x179BEE7A, 0xD37AC6A9, 0xFE5830A4,
   0x98DE8B7F, 0x77E83F4E, 0x79929269, 0x24FA9F7B, 0xE113C85B, 0xACC40083,
   0xD7503525, 0xF7EA615F, 0x62143154, 0x0D554B63, 0x5D681121, 0xC866C359,
   0x3D63CF73, 0xCEE234C0, 0xD4D87E87, 0x5C672B21, 0x071F6181, 0x39F7627F,
   0x361E3084, 0xE4EB573B, 0x602F64A4, 0xD63ACD9C, 0x1BBC4635, 0x9E81032D,
   0x2701F50C, 0x99847AB4, 0xA0E3DF79, 0xBA6CF38C, 0x10843094, 0x2537A95E,
   0xF46F6FFE, 0xA1FF3B1F, 0x208CFB6A, 0x8F458C74, 0xD9E0A227, 0x4EC73A34,
   0xFC884F69, 0x3E4DE8DF, 0xEF0E0088, 0x3559648D, 0x8A45388C, 0x1D804366,
   0x721D9BFD, 0xA58684BB, 0xE8256333, 0x844E8212, 0x128D8098, 0xFED33FB4,
   0xCE280AE1, 0x27E19BA5, 0xD5A6C252, 0xE49754BD, 0xC5D655DD, 0xEB667064,
   0x77840B4D, 0xA1B6A801, 0x84DB26A9, 0xE0B56714, 0x21F043B7, 0xE5D05860,
   0x54F03084, 0x066FF472, 0xA31AA153, 0xDADC4755, 0xB5625DBF, 0x68561BE6,
   0x83CA6B94, 0x2D6ED23B, 0xECCF01DB, 0xA6D3D0BA, 0xB6803D5C, 0xAF77A709,
   0x33B4A34C, 0x397BC8D6, 0x5EE22B95, 0x5F0E5304, 0x81ED6F61, 0x20E74364,
   0xB45E1378, 0xDE18639B, 0x881CA122, 0xB96726D1, 0x8049A7E8, 0x22B7DA7B,
   0x5E552D25, 0x5272D237, 0x79D2951C, 0xC60D894C, 0x488CB402, 0x1BA4FE5B,
   0xA4B09F6B, 0x1CA815CF, 0xA20C3005, 0x8871DF63, 0xB9DE2FCB, 0x0CC6C9E9,
   0x0BEEFF53, 0xE3214517, 0xB4542835, 0x9F63293C, 0xEE41E729, 0x6E1D2D7C,
   0x50045286, 0x1E6685F3, 0xF33401C6, 0x30A22C95, 0x31A70850, 0x60930F13,
   0x73F98417, 0xA1269859, 0xEC645C44, 0x52C877A9, 0xCDFF33A6, 0xA02B1741,
   0x7CBAD9A2, 0x2180036F, 0x50D99C08, 0xCB3F4861, 0xC26BD765, 0x64A3F6AB,
   0x80342676, 0x25A75E7B, 0xE4E6D1FC, 0x20C710E6, 0xCDF0B680, 0x17844D3B,
   0x31EEF84D, 0x7E0824E4, 0x2CCB49EB, 0x846A3BAE, 0x8FF77888, 0xEE5D60F6,
   0x7AF75673, 0x2FDD5CDB, 0xA11631C1, 0x30F66F43, 0xB3FAEC54, 0x157FD7FA,
   0xEF8579CC, 0xD152DE58, 0xDB2FFD5E, 0x8F32CE19, 0x306AF97A, 0x02F03EF8,
   0x99319AD5, 0xC242FA0F, 0xA7E3EBB0, 0xC68E4906, 0xB8DA230C, 0x80823028,
   0xDCDEF3C8, 0xD35FB171, 0x088A1BC8, 0xBEC0C560, 0x61A3C9E8, 0xBCA8F54D,
   0xC72FEFFA, 0x22822E99, 0x82C570B4, 0xD8D94E89, 0x8B1C34BC, 0x301E16E6,
   0x273BE979, 0xB0FFEAA6, 0x61D9B8C6, 0x00B24869, 0xB7FFCE3F, 0x08DC283B,
   0x43DAF65A, 0xF7E19798, 0x7619B72F, 0x8F1C9BA4, 0xDC8637A0, 0x16A7D3B1,
   0x9FC393B7, 0xA7136EEB, 0xC6BCC63E, 0x1A513742, 0xEF6828BC, 0x520365D6,
   0x2D6A77AB, 0x3527ED4B, 0x821FD216, 0x095C6E2E, 0xDB92F2FB, 0x5EEA29CB,
   0x145892F5, 0x91584F7F, 0x5483697B, 0x2667A8CC, 0x85196048, 0x8C4BACEA,
   0x833860D4, 0x0D23E0F9, 0x6C387E8A, 0x0AE6D249, 0xB284600C, 0xD835731D,
   0xDCB1C647, 0xAC4C56EA, 0x3EBD81B3, 0x230EABB0, 0x6438BC87, 0xF0B5B1FA,
   0x8F5EA2B3, 0xFC184642, 0x0A036B7A, 0x4FB089BD, 0x649DA589, 0xA345415E,
   0x5C038323, 0x3E5D3BB9, 0x43D79572, 0x7E6DD07C, 0x06DFDF1E, 0x6C6CC4EF,
   0x7160A539, 0x73BFBE70, 0x83877605, 0x4523ECF1 };

const uint32_t CAST_SBOX3[256] = {
   0x8DEFC240, 0x25FA5D9F, 0xEB903DBF, 0xE810C907, 0x47607FFF, 0x369FE44B,
   0x8C1FC644, 0xAECECA90, 0xBEB1F9BF, 0xEEFBCAEA, 0xE8CF1950, 0x51DF07AE,
   0x920E8806, 0xF0AD0548, 0xE13C8D83, 0x927010D5, 0x11107D9F, 0x07647DB9,
   0xB2E3E4D4, 0x3D4F285E, 0xB9AFA820, 0xFADE82E0, 0xA067268B, 0x8272792E,
   0x553FB2C0, 0x489AE22B, 0xD4EF9794, 0x125E3FBC, 0x21FFFCEE, 0x825B1BFD,
   0x9255C5ED, 0x1257A240, 0x4E1A8302, 0xBAE07FFF, 0x528246E7, 0x8E57140E,
   0x3373F7BF, 0x8C9F8188, 0xA6FC4EE8, 0xC982B5A5, 0xA8C01DB7, 0x579FC264,
   0x67094F31, 0xF2BD3F5F, 0x40FFF7C1, 0x1FB78DFC, 0x8E6BD2C1, 0x437BE59B,
   0x99B03DBF, 0xB5DBC64B, 0x638DC0E6, 0x55819D99, 0xA197C81C, 0x4A012D6E,
   0xC5884A28, 0xCCC36F71, 0xB843C213, 0x6C0743F1, 0x8309893C, 0x0FEDDD5F,
   0x2F7FE850, 0xD7C07F7E, 0x02507FBF, 0x5AFB9A04, 0xA747D2D0, 0x1651192E,
   0xAF70BF3E, 0x58C31380, 0x5F98302E, 0x727CC3C4, 0x0A0FB402, 0x0F7FEF82,
   0x8C96FDAD, 0x5D2C2AAE, 0x8EE99A49, 0x50DA88B8, 0x8427F4A0, 0x1EAC5790,
   0x796FB449, 0x8252DC15, 0xEFBD7D9B, 0xA672597D, 0xADA840D8, 0x45F54504,
   0xFA5D7403, 0xE83EC305, 0x4F91751A, 0x925669C2, 0x23EFE941, 0xA903F12E,
   0x60270DF2, 0x0276E4B6, 0x94FD6574, 0x927985B2, 0x8276DBCB, 0x02778176,
   0xF8AF918D, 0x4E48F79E, 0x8F616DDF, 0xE29D840E, 0x842F7D83, 0x340CE5C8,
   0x96BBB682, 0x93B4B148, 0xEF303CAB, 0x984FAF28, 0x779FAF9B, 0x92DC560D,
   0x224D1E20, 0x8437AA88, 0x7D29DC96, 0x2756D3DC, 0x8B907CEE, 0xB51FD240,
   0xE7C07CE3, 0xE566B4A1, 0xC3E9615E, 0x3CF8209D, 0x6094D1E3, 0xCD9CA341,
   0x5C76460E, 0x00EA983B, 0xD4D67881, 0xFD47572C, 0xF76CEDD9, 0xBDA8229C,
   0x127DADAA, 0x438A074E, 0x1F97C090, 0x081BDB8A, 0x93A07EBE, 0xB938CA15,
   0x97B03CFF, 0x3DC2C0F8, 0x8D1AB2EC, 0x64380E51, 0x68CC7BFB, 0xD90F2788,
   0x12490181, 0x5DE5FFD4, 0xDD7EF86A, 0x76A2E214, 0xB9A40368, 0x925D958F,
   0x4B39FFFA, 0xBA39AEE9, 0xA4FFD30B, 0xFAF7933B, 0x6D498623, 0x193CBCFA,
   0x27627545, 0x825CF47A, 0x61BD8BA0, 0xD11E42D1, 0xCEAD04F4, 0x127EA392,
   0x10428DB7, 0x8272A972, 0x9270C4A8, 0x127DE50B, 0x285BA1C8, 0x3C62F44F,
   0x35C0EAA5, 0xE805D231, 0x428929FB, 0xB4FCDF82, 0x4FB66A53, 0x0E7DC15B,
   0x1F081FAB, 0x108618AE, 0xFCFD086D, 0xF9FF2889, 0x694BCC11, 0x236A5CAE,
   0x12DECA4D, 0x2C3F8CC5, 0xD2D02DFE, 0xF8EF5896, 0xE4CF52DA, 0x95155B67,
   0x494A488C, 0xB9B6A80C, 0x5C8F82BC, 0x89D36B45, 0x3A609437, 0xEC00C9A9,
   0x44715253, 0x0A874B49, 0xD773BC40, 0x7C34671C, 0x02717EF6, 0x4FEB5536,
   0xA2D02FFF, 0xD2BF60C4, 0xD43F03C0, 0x50B4EF6D, 0x07478CD1, 0x006E1888,
   0xA2E53F55, 0xB9E6D4BC, 0xA2048016, 0x97573833, 0xD7207D67, 0xDE0F8F3D,
   0x72F87B33, 0xABCC4F33, 0x7688C55D, 0x7B00A6B0, 0x947B0001, 0x570075D2,
   0xF9BB88F8, 0x8942019E, 0x4264A5FF, 0x856302E0, 0x72DBD92B, 0xEE971B69,
   0x6EA22FDE, 0x5F08AE2B, 0xAF7A616D, 0xE5C98767, 0xCF1FEBD2, 0x61EFC8C2,
   0xF1AC2571, 0xCC8239C2, 0x67214CB8, 0xB1E583D1, 0xB7DC3E62, 0x7F10BDCE,
   0xF90A5C38, 0x0FF0443D, 0x606E6DC6, 0x60543A49, 0x5727C148, 0x2BE98A1D,
   0x8AB41738, 0x20E1BE24, 0xAF96DA0F, 0x68458425, 0x99833BE5, 0x600D457D,
   0x282F9350, 0x8334B362, 0xD91D1120, 0x2B6D8DA0, 0x642B1E31, 0x9C305A00,
   0x52BCE688, 0x1B03588A, 0xF7BAEFD5, 0x4142ED9C, 0xA4315C11, 0x83323EC5,
   0xDFEF4636, 0xA133C501, 0xE9D3531C, 0xEE353783 };

const uint32_t CAST_SBOX4[256] = {
   0x9DB30420, 0x1FB6E9DE, 0xA7BE7BEF, 0xD273A298, 0x4A4F7BDB, 0x64AD8C57,
   0x85510443, 0xFA020ED1, 0x7E287AFF, 0xE60FB663, 0x095F35A1, 0x79EBF120,
   0xFD059D43, 0x6497B7B1, 0xF3641F63, 0x241E4ADF, 0x28147F5F, 0x4FA2B8CD,
   0xC9430040, 0x0CC32220, 0xFDD30B30, 0xC0A5374F, 0x1D2D00D9, 0x24147B15,
   0xEE4D111A, 0x0FCA5167, 0x71FF904C, 0x2D195FFE, 0x1A05645F, 0x0C13FEFE,
   0x081B08CA, 0x05170121, 0x80530100, 0xE83E5EFE, 0xAC9AF4F8, 0x7FE72701,
   0xD2B8EE5F, 0x06DF4261, 0xBB9E9B8A, 0x7293EA25, 0xCE84FFDF, 0xF5718801,
   0x3DD64B04, 0xA26F263B, 0x7ED48400, 0x547EEBE6, 0x446D4CA0, 0x6CF3D6F5,
   0x2649ABDF, 0xAEA0C7F5, 0x36338CC1, 0x503F7E93, 0xD3772061, 0x11B638E1,
   0x72500E03, 0xF80EB2BB, 0xABE0502E, 0xEC8D77DE, 0x57971E81, 0xE14F6746,
   0xC9335400, 0x6920318F, 0x081DBB99, 0xFFC304A5, 0x4D351805, 0x7F3D5CE3,
   0xA6C866C6, 0x5D5BCCA9, 0xDAEC6FEA, 0x9F926F91, 0x9F46222F, 0x3991467D,
   0xA5BF6D8E, 0x1143C44F, 0x43958302, 0xD0214EEB, 0x022083B8, 0x3FB6180C,
   0x18F8931E, 0x281658E6, 0x26486E3E, 0x8BD78A70, 0x7477E4C1, 0xB506E07C,
   0xF32D0A25, 0x79098B02, 0xE4EABB81, 0x28123B23, 0x69DEAD38, 0x1574CA16,
   0xDF871B62, 0x211C40B7, 0xA51A9EF9, 0x0014377B, 0x041E8AC8, 0x09114003,
   0xBD59E4D2, 0xE3D156D5, 0x4FE876D5, 0x2F91A340, 0x557BE8DE, 0x00EAE4A7,
   0x0CE5C2EC, 0x4DB4BBA6, 0xE756BDFF, 0xDD3369AC, 0xEC17B035, 0x06572327,
   0x99AFC8B0, 0x56C8C391, 0x6B65811C, 0x5E146119, 0x6E85CB75, 0xBE07C002,
   0xC2325577, 0x893FF4EC, 0x5BBFC92D, 0xD0EC3B25, 0xB7801AB7, 0x8D6D3B24,
   0x20C763EF, 0xC366A5FC, 0x9C382880, 0x0ACE3205, 0xAAC9548A, 0xECA1D7C7,
   0x041AFA32, 0x1D16625A, 0x6701902C, 0x9B757A54, 0x31D477F7, 0x9126B031,
   0x36CC6FDB, 0xC70B8B46, 0xD9E66A48, 0x56E55A79, 0x026A4CEB, 0x52437EFF,
   0x2F8F76B4, 0x0DF980A5, 0x8674CDE3, 0xEDDA04EB, 0x17A9BE04, 0x2C18F4DF,
   0xB7747F9D, 0xAB2AF7B4, 0xEFC34D20, 0x2E096B7C, 0x1741A254, 0xE5B6A035,
   0x213D42F6, 0x2C1C7C26, 0x61C2F50F, 0x6552DAF9, 0xD2C231F8, 0x25130F69,
   0xD8167FA2, 0x0418F2C8, 0x001A96A6, 0x0D1526AB, 0x63315C21, 0x5E0A72EC,
   0x49BAFEFD, 0x187908D9, 0x8D0DBD86, 0x311170A7, 0x3E9B640C, 0xCC3E10D7,
   0xD5CAD3B6, 0x0CAEC388, 0xF73001E1, 0x6C728AFF, 0x71EAE2A1, 0x1F9AF36E,
   0xCFCBD12F, 0xC1DE8417, 0xAC07BE6B, 0xCB44A1D8, 0x8B9B0F56, 0x013988C3,
   0xB1C52FCA, 0xB4BE31CD, 0xD8782806, 0x12A3A4E2, 0x6F7DE532, 0x58FD7EB6,
   0xD01EE900, 0x24ADFFC2, 0xF4990FC5, 0x9711AAC5, 0x001D7B95, 0x82E5E7D2,
   0x109873F6, 0x00613096, 0xC32D9521, 0xADA121FF, 0x29908415, 0x7FBB977F,
   0xAF9EB3DB, 0x29C9ED2A, 0x5CE2A465, 0xA730F32C, 0xD0AA3FE8, 0x8A5CC091,
   0xD49E2CE7, 0x0CE454A9, 0xD60ACD86, 0x015F1919, 0x77079103, 0xDEA03AF6,
   0x78A8565E, 0xDEE356DF, 0x21F05CBE, 0x8B75E387, 0xB3C50651, 0xB8A5C3EF,
   0xD8EEB6D2, 0xE523BE77, 0xC2154529, 0x2F69EFDF, 0xAFE67AFB, 0xF470C4B2,
   0xF3E0EB5B, 0xD6CC9876, 0x39E4460C, 0x1FDA8538, 0x1987832F, 0xCA007367,
   0xA99144F8, 0x296B299E, 0x492FC295, 0x9266BEAB, 0xB5676E69, 0x9BD3DDDA,
   0xDF7E052F, 0xDB25701C, 0x1B5E51EE, 0xF65324E6, 0x6AFCE36C, 0x0316CC04,
   0x8644213E, 0xB7DC59D0, 0x7965291F, 0xCCD6FD43, 0x41823979, 0x932BCDF6,
   0xB657C34D, 0x4EDFD282, 0x7AE5290C, 0x3CB9536B, 0x851E20FE, 0x9833557E,
   0x13ECF0B0, 0xD3FFB372, 0x3F85C5C1, 0x0AEF7ED2 };

}

namespace Botan {

void gcm_clmul_precompute(const uint8_t H[16], uint64_t H_pow[4*2]);

void gcm_multiply_clmul(uint8_t x[16],
                        const uint64_t H_pow[4*2],
                        const uint8_t input[], size_t blocks);

}

namespace Botan {

void gcm_multiply_ssse3(uint8_t x[16],
                        const uint64_t HM[256],
                        const uint8_t input[], size_t blocks);

}

namespace Botan {

/**
* Expand an input to a bit mask depending on it being being zero or non-zero
* @param tst the input
* @return the mask 0xFFFF if tst is non-zero and 0 otherwise
*/
template<typename T>
uint16_t expand_mask_16bit(T tst)
   {
   const uint16_t result = (tst != 0);
   return ~(result - 1);
   }

inline gf2m gray_to_lex(gf2m gray)
   {
   gf2m result = gray ^ (gray >> 8);
   result ^= (result >> 4);
   result ^= (result >> 2);
   result ^= (result >> 1);
   return result;
   }

inline gf2m lex_to_gray(gf2m lex)
   {
   return (lex >> 1) ^ lex;
   }

inline uint32_t bit_size_to_byte_size(uint32_t bit_size)
   {
   return (bit_size - 1) / 8 + 1;
   }

inline uint32_t bit_size_to_32bit_size(uint32_t bit_size)
   {
   return (bit_size - 1) / 32 + 1;
   }

}

namespace Botan {

/**
* Perform encoding using the base provided
* @param base object giving access to the encodings specifications
* @param output an array of at least base.encode_max_output bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding chars will be applied if needed
* @return number of bytes written to output
*/
template <class Base>
size_t base_encode(Base&& base,
                   char output[],
                   const uint8_t input[],
                   size_t input_length,
                   size_t& input_consumed,
                   bool final_inputs)
   {
   input_consumed = 0;

   const size_t encoding_bytes_in = base.encoding_bytes_in();
   const size_t encoding_bytes_out = base.encoding_bytes_out();

   size_t input_remaining = input_length;
   size_t output_produced = 0;

   while(input_remaining >= encoding_bytes_in)
      {
      base.encode(output + output_produced, input + input_consumed);

      input_consumed += encoding_bytes_in;
      output_produced += encoding_bytes_out;
      input_remaining -= encoding_bytes_in;
      }

   if(final_inputs && input_remaining)
      {
      std::vector<uint8_t> remainder(encoding_bytes_in, 0);
      for(size_t i = 0; i != input_remaining; ++i)
         { remainder[i] = input[input_consumed + i]; }

      base.encode(output + output_produced, remainder.data());

      const size_t bits_consumed = base.bits_consumed();
      const size_t remaining_bits_before_padding = base.remaining_bits_before_padding();

      size_t empty_bits = 8 * (encoding_bytes_in - input_remaining);
      size_t index = output_produced + encoding_bytes_out - 1;
      while(empty_bits >= remaining_bits_before_padding)
         {
         output[index--] = '=';
         empty_bits -= bits_consumed;
         }

      input_consumed += input_remaining;
      output_produced += encoding_bytes_out;
      }

   return output_produced;
   }


template <typename Base>
std::string base_encode_to_string(Base&& base, const uint8_t input[], size_t input_length)
   {
   const size_t output_length = base.encode_max_output(input_length);
   std::string output(output_length, 0);

   size_t consumed = 0;
   size_t produced = 0;

   if(output_length > 0)
      {
      produced = base_encode(base, &output.front(),
                                   input, input_length,
                                   consumed, true);
      }

   BOTAN_ASSERT_EQUAL(consumed, input_length, "Consumed the entire input");
   BOTAN_ASSERT_EQUAL(produced, output.size(), "Produced expected size");

   return output;
   }

/**
* Perform decoding using the base provided
* @param base object giving access to the encodings specifications
* @param output an array of at least Base::decode_max_output bytes
* @param input some base input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding is allowed
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
template <typename Base>
size_t base_decode(Base&& base,
                   uint8_t output[],
                   const char input[],
                   size_t input_length,
                   size_t& input_consumed,
                   bool final_inputs,
                   bool ignore_ws = true)
   {
   const size_t decoding_bytes_in = base.decoding_bytes_in();
   const size_t decoding_bytes_out = base.decoding_bytes_out();

   uint8_t* out_ptr = output;
   std::vector<uint8_t> decode_buf(decoding_bytes_in, 0);
   size_t decode_buf_pos = 0;
   size_t final_truncate = 0;

   clear_mem(output, base.decode_max_output(input_length));

   for(size_t i = 0; i != input_length; ++i)
      {
      const uint8_t bin = base.lookup_binary_value(input[i]);

      if(base.check_bad_char(bin, input[i], ignore_ws)) // May throw Invalid_Argument
         {
         decode_buf[decode_buf_pos] = bin;
         ++decode_buf_pos;
         }

      /*
      * If we're at the end of the input, pad with 0s and truncate
      */
      if(final_inputs && (i == input_length - 1))
         {
         if(decode_buf_pos)
            {
            for(size_t j = decode_buf_pos; j < decoding_bytes_in; ++j)
               { decode_buf[j] = 0; }

            final_truncate = decoding_bytes_in - decode_buf_pos;
            decode_buf_pos = decoding_bytes_in;
            }
         }

      if(decode_buf_pos == decoding_bytes_in)
         {
         base.decode(out_ptr, decode_buf.data());

         out_ptr += decoding_bytes_out;
         decode_buf_pos = 0;
         input_consumed = i+1;
         }
      }

   while(input_consumed < input_length &&
         base.lookup_binary_value(input[input_consumed]) == 0x80)
      {
      ++input_consumed;
      }

   size_t written = (out_ptr - output) - base.bytes_to_remove(final_truncate);

   return written;
   }

template<typename Base>
size_t base_decode_full(Base&& base, uint8_t output[], const char input[], size_t input_length, bool ignore_ws)
   {
   size_t consumed = 0;
   const size_t written = base_decode(base, output, input, input_length, consumed, true, ignore_ws);

   if(consumed != input_length)
      {
      throw Invalid_Argument(base.name() + " decoding failed, input did not have full bytes");
      }

   return written;
   }

template<typename Vector, typename Base>
Vector base_decode_to_vec(Base&& base,
                          const char input[],
                          size_t input_length,
                          bool ignore_ws)
   {
   const size_t output_length = base.decode_max_output(input_length);
   Vector bin(output_length);

   const size_t written =
      base_decode_full(base, bin.data(), input, input_length, ignore_ws);

   bin.resize(written);
   return bin;
   }

}

#if defined(BOTAN_HAS_VALGRIND)
  #include <valgrind/memcheck.h>
#endif

namespace Botan {

namespace CT {

/**
* Use valgrind to mark the contents of memory as being undefined.
* Valgrind will accept operations which manipulate undefined values,
* but will warn if an undefined value is used to decided a conditional
* jump or a load/store address. So if we poison all of our inputs we
* can confirm that the operations in question are truly const time
* when compiled by whatever compiler is in use.
*
* Even better, the VALGRIND_MAKE_MEM_* macros work even when the
* program is not run under valgrind (though with a few cycles of
* overhead, which is unfortunate in final binaries as these
* annotations tend to be used in fairly important loops).
*
* This approach was first used in ctgrind (https://github.com/agl/ctgrind)
* but calling the valgrind mecheck API directly works just as well and
* doesn't require a custom patched valgrind.
*/
template<typename T>
inline void poison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_UNDEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p);
   BOTAN_UNUSED(n);
#endif
   }

template<typename T>
inline void unpoison(T& p)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(&p, sizeof(T));
#else
   BOTAN_UNUSED(p);
#endif
   }

/**
* A Mask type used for constant-time operations. A Mask<T> always has value
* either 0 (all bits cleared) or ~0 (all bits set). All operations in a Mask<T>
* are intended to compile to code which does not contain conditional jumps.
* This must be verified with tooling (eg binary disassembly or using valgrind)
* since you never know what a compiler might do.
*/
template<typename T>
class Mask
   {
   public:
      static_assert(std::is_unsigned<T>::value, "CT::Mask only defined for unsigned integer types");

      Mask(const Mask<T>& other) = default;
      Mask<T>& operator=(const Mask<T>& other) = default;

      /**
      * Derive a Mask from a Mask of a larger type
      */
      template<typename U>
      Mask(Mask<U> o) : m_mask(static_cast<T>(o.value()))
         {
         static_assert(sizeof(U) > sizeof(T), "sizes ok");
         }

      /**
      * Return a Mask<T> with all bits set
      */
      static Mask<T> set()
         {
         return Mask<T>(static_cast<T>(~0));
         }

      /**
      * Return a Mask<T> with all bits cleared
      */
      static Mask<T> cleared()
         {
         return Mask<T>(0);
         }

      /**
      * Return a Mask<T> which is set if v is != 0
      */
      static Mask<T> expand(T v)
         {
         return ~Mask<T>::is_zero(v);
         }

      /**
      * Return a Mask<T> which is set if m is set
      */
      template<typename U>
      static Mask<T> expand(Mask<U> m)
         {
         static_assert(sizeof(U) < sizeof(T), "sizes ok");
         return ~Mask<T>::is_zero(m.value());
         }

      /**
      * Return a Mask<T> which is set if v is == 0 or cleared otherwise
      */
      static Mask<T> is_zero(T x)
         {
         return Mask<T>(ct_is_zero<T>(x));
         }

      /**
      * Return a Mask<T> which is set if x == y
      */
      static Mask<T> is_equal(T x, T y)
         {
         return Mask<T>::is_zero(static_cast<T>(x ^ y));
         }

      /**
      * Return a Mask<T> which is set if x < y
      */
      static Mask<T> is_lt(T x, T y)
         {
         return Mask<T>(expand_top_bit<T>(x^((x^y) | ((x-y)^x))));
         }

      /**
      * Return a Mask<T> which is set if x > y
      */
      static Mask<T> is_gt(T x, T y)
         {
         return Mask<T>::is_lt(y, x);
         }

      /**
      * Return a Mask<T> which is set if x <= y
      */
      static Mask<T> is_lte(T x, T y)
         {
         return ~Mask<T>::is_gt(x, y);
         }

      /**
      * Return a Mask<T> which is set if x >= y
      */
      static Mask<T> is_gte(T x, T y)
         {
         return ~Mask<T>::is_lt(x, y);
         }

      /**
      * AND-combine two masks
      */
      Mask<T>& operator&=(Mask<T> o)
         {
         m_mask &= o.value();
         return (*this);
         }

      /**
      * XOR-combine two masks
      */
      Mask<T>& operator^=(Mask<T> o)
         {
         m_mask ^= o.value();
         return (*this);
         }

      /**
      * OR-combine two masks
      */
      Mask<T>& operator|=(Mask<T> o)
         {
         m_mask |= o.value();
         return (*this);
         }

      /**
      * AND-combine two masks
      */
      friend Mask<T> operator&(Mask<T> x, Mask<T> y)
         {
         return Mask<T>(x.value() & y.value());
         }

      /**
      * XOR-combine two masks
      */
      friend Mask<T> operator^(Mask<T> x, Mask<T> y)
         {
         return Mask<T>(x.value() ^ y.value());
         }

      /**
      * OR-combine two masks
      */
      friend Mask<T> operator|(Mask<T> x, Mask<T> y)
         {
         return Mask<T>(x.value() | y.value());
         }

      /**
      * Negate this mask
      */
      Mask<T> operator~() const
         {
         return Mask<T>(~value());
         }

      /**
      * Return x if the mask is set, or otherwise zero
      */
      T if_set_return(T x) const
         {
         return m_mask & x;
         }

      /**
      * Return x if the mask is cleared, or otherwise zero
      */
      T if_not_set_return(T x) const
         {
         return ~m_mask & x;
         }

      /**
      * If this mask is set, return x, otherwise return y
      */
      T select(T x, T y) const
         {
         // (x & value()) | (y & ~value())
         return static_cast<T>(y ^ (value() & (x ^ y)));
         }

      T select_and_unpoison(T x, T y) const
         {
         T r = this->select(x, y);
         CT::unpoison(r);
         return r;
         }

      /**
      * If this mask is set, return x, otherwise return y
      */
      Mask<T> select_mask(Mask<T> x, Mask<T> y) const
         {
         return Mask<T>(select(x.value(), y.value()));
         }

      /**
      * Conditionally set output to x or y, depending on if mask is set or
      * cleared (resp)
      */
      void select_n(T output[], const T x[], const T y[], size_t len) const
         {
         for(size_t i = 0; i != len; ++i)
            output[i] = this->select(x[i], y[i]);
         }

      /**
      * If this mask is set, zero out buf, otherwise do nothing
      */
      void if_set_zero_out(T buf[], size_t elems)
         {
         for(size_t i = 0; i != elems; ++i)
            {
            buf[i] = this->if_not_set_return(buf[i]);
            }
         }

      /**
      * Return the value of the mask, unpoisoned
      */
      T unpoisoned_value() const
         {
         T r = value();
         CT::unpoison(r);
         return r;
         }

      /**
      * Return true iff this mask is set
      */
      bool is_set() const
         {
         return unpoisoned_value() != 0;
         }

      /**
      * Return the underlying value of the mask
      */
      T value() const
         {
         return m_mask;
         }

   private:
      Mask(T m) : m_mask(m) {}

      T m_mask;
   };

template<typename T>
inline Mask<T> conditional_copy_mem(T cnd,
                                    T* to,
                                    const T* from0,
                                    const T* from1,
                                    size_t elems)
   {
   const auto mask = CT::Mask<T>::expand(cnd);
   mask.select_n(to, from0, from1, elems);
   return mask;
   }

template<typename T>
inline void conditional_swap(bool cnd, T& x, T& y)
   {
   const auto swap = CT::Mask<T>::expand(cnd);

   T t0 = swap.select(y, x);
   T t1 = swap.select(x, y);
   x = t0;
   y = t1;
   }

template<typename T>
inline void conditional_swap_ptr(bool cnd, T& x, T& y)
   {
   uintptr_t xp = reinterpret_cast<uintptr_t>(x);
   uintptr_t yp = reinterpret_cast<uintptr_t>(y);

   conditional_swap<uintptr_t>(cnd, xp, yp);

   x = reinterpret_cast<T>(xp);
   y = reinterpret_cast<T>(yp);
   }

/**
* If bad_mask is unset, return in[delim_idx:input_length] copied to
* new buffer. If bad_mask is set, return an all zero vector of
* unspecified length.
*/
secure_vector<uint8_t> copy_output(CT::Mask<uint8_t> bad_input,
                                   const uint8_t input[],
                                   size_t input_length,
                                   size_t delim_idx);

secure_vector<uint8_t> strip_leading_zeros(const uint8_t in[], size_t length);

inline secure_vector<uint8_t> strip_leading_zeros(const secure_vector<uint8_t>& in)
   {
   return strip_leading_zeros(in.data(), in.size());
   }

}

}

namespace Botan {

/**
* Fixed Window Exponentiator
*/
class Fixed_Window_Exponentiator final : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&) override;
      void set_base(const BigInt&) override;
      BigInt execute() const override;

      Modular_Exponentiator* copy() const override
         { return new Fixed_Window_Exponentiator(*this); }

      Fixed_Window_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      Modular_Reducer m_reducer;
      BigInt m_exp;
      size_t m_window_bits;
      std::vector<BigInt> m_g;
      Power_Mod::Usage_Hints m_hints;
   };

class Montgomery_Params;
class Montgomery_Exponentation_State;

/**
* Montgomery Exponentiator
*/
class Montgomery_Exponentiator final : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&) override;
      void set_base(const BigInt&) override;
      BigInt execute() const override;

      Modular_Exponentiator* copy() const override
         { return new Montgomery_Exponentiator(*this); }

      Montgomery_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      BigInt m_p;
      Modular_Reducer m_mod_p;
      std::shared_ptr<const Montgomery_Params> m_monty_params;
      std::shared_ptr<const Montgomery_Exponentation_State> m_monty;

      BigInt m_e;
      Power_Mod::Usage_Hints m_hints;
   };

}

namespace Botan {

/**
* Entropy source reading from kernel devices like /dev/random
*/
class Device_EntropySource final : public Entropy_Source
   {
   public:
      std::string name() const override { return "dev_random"; }

      size_t poll(RandomNumberGenerator& rng) override;

      explicit Device_EntropySource(const std::vector<std::string>& fsnames);

      ~Device_EntropySource();
   private:
      std::vector<int> m_dev_fds;
      int m_max_fd;
   };

}

namespace Botan {

class donna128 final
   {
   public:
      donna128(uint64_t ll = 0, uint64_t hh = 0) { l = ll; h = hh; }

      donna128(const donna128&) = default;
      donna128& operator=(const donna128&) = default;

      friend donna128 operator>>(const donna128& x, size_t shift)
         {
         donna128 z = x;
         if(shift > 0)
            {
            const uint64_t carry = z.h << (64 - shift);
            z.h = (z.h >> shift);
            z.l = (z.l >> shift) | carry;
            }
         return z;
         }

      friend donna128 operator<<(const donna128& x, size_t shift)
         {
         donna128 z = x;
         if(shift > 0)
            {
            const uint64_t carry = z.l >> (64 - shift);
            z.l = (z.l << shift);
            z.h = (z.h << shift) | carry;
            }
         return z;
         }

      friend uint64_t operator&(const donna128& x, uint64_t mask)
         {
         return x.l & mask;
         }

      uint64_t operator&=(uint64_t mask)
         {
         h = 0;
         l &= mask;
         return l;
         }

      donna128& operator+=(const donna128& x)
         {
         l += x.l;
         h += x.h;

         const uint64_t carry = (l < x.l);
         h += carry;
         return *this;
         }

      donna128& operator+=(uint64_t x)
         {
         l += x;
         const uint64_t carry = (l < x);
         h += carry;
         return *this;
         }

      uint64_t lo() const { return l; }
      uint64_t hi() const { return h; }
   private:
      uint64_t h = 0, l = 0;
   };

inline donna128 operator*(const donna128& x, uint64_t y)
   {
   BOTAN_ARG_CHECK(x.hi() == 0, "High 64 bits of donna128 set to zero during multiply");

   uint64_t lo = 0, hi = 0;
   mul64x64_128(x.lo(), y, &lo, &hi);
   return donna128(lo, hi);
   }

inline donna128 operator*(uint64_t y, const donna128& x)
   {
   return x * y;
   }

inline donna128 operator+(const donna128& x, const donna128& y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator+(const donna128& x, uint64_t y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator|(const donna128& x, const donna128& y)
   {
   return donna128(x.lo() | y.lo(), x.hi() | y.hi());
   }

inline uint64_t carry_shift(const donna128& a, size_t shift)
   {
   return (a >> shift).lo();
   }

inline uint64_t combine_lower(const donna128& a, size_t s1,
                              const donna128& b, size_t s2)
   {
   donna128 z = (a >> s1) | (b << s2);
   return z.lo();
   }

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
inline uint64_t carry_shift(const uint128_t a, size_t shift)
   {
   return static_cast<uint64_t>(a >> shift);
   }

inline uint64_t combine_lower(const uint128_t a, size_t s1,
                              const uint128_t b, size_t s2)
   {
   return static_cast<uint64_t>((a >> s1) | (b << s2));
   }
#endif

}

namespace Botan {

/**
* An element of the field \\Z/(2^255-19)
*/
class FE_25519
   {
   public:
      ~FE_25519() { secure_scrub_memory(m_fe, sizeof(m_fe)); }

      /**
      * Zero element
      */
      FE_25519(int init = 0)
         {
         if(init != 0 && init != 1)
            throw Invalid_Argument("Invalid FE_25519 initial value");
         clear_mem(m_fe, 10);
         m_fe[0] = init;
         }

      FE_25519(std::initializer_list<int32_t> x)
         {
         if(x.size() != 10)
            throw Invalid_Argument("Invalid FE_25519 initializer list");
         copy_mem(m_fe, x.begin(), 10);
         }

      FE_25519(int64_t h0, int64_t h1, int64_t h2, int64_t h3, int64_t h4,
               int64_t h5, int64_t h6, int64_t h7, int64_t h8, int64_t h9)
         {
         m_fe[0] = static_cast<int32_t>(h0);
         m_fe[1] = static_cast<int32_t>(h1);
         m_fe[2] = static_cast<int32_t>(h2);
         m_fe[3] = static_cast<int32_t>(h3);
         m_fe[4] = static_cast<int32_t>(h4);
         m_fe[5] = static_cast<int32_t>(h5);
         m_fe[6] = static_cast<int32_t>(h6);
         m_fe[7] = static_cast<int32_t>(h7);
         m_fe[8] = static_cast<int32_t>(h8);
         m_fe[9] = static_cast<int32_t>(h9);
         }

      FE_25519(const FE_25519& other) = default;
      FE_25519& operator=(const FE_25519& other) = default;

      FE_25519(FE_25519&& other) = default;
      FE_25519& operator=(FE_25519&& other) = default;

      void from_bytes(const uint8_t b[32]);
      void to_bytes(uint8_t b[32]) const;

      bool is_zero() const
         {
         uint8_t s[32];
         to_bytes(s);

         uint8_t sum = 0;
         for(size_t i = 0; i != 32; ++i)
            { sum |= s[i]; }

         // TODO avoid ternary here
         return (sum == 0) ? 1 : 0;
         }

      /*
      return 1 if f is in {1,3,5,...,q-2}
      return 0 if f is in {0,2,4,...,q-1}
      */
      bool is_negative() const
         {
         // TODO could avoid most of the to_bytes computation here
         uint8_t s[32];
         to_bytes(s);
         return s[0] & 1;
         }

      static FE_25519 add(const FE_25519& a, const FE_25519& b)
         {
         FE_25519 z;
         for(size_t i = 0; i != 10; ++i)
            { z[i] = a[i] + b[i]; }
         return z;
         }

      static FE_25519 sub(const FE_25519& a, const FE_25519& b)
         {
         FE_25519 z;
         for(size_t i = 0; i != 10; ++i)
            { z[i] = a[i] - b[i]; }
         return z;
         }

      static FE_25519 negate(const FE_25519& a)
         {
         FE_25519 z;
         for(size_t i = 0; i != 10; ++i)
            { z[i] = -a[i]; }
         return z;
         }

      static FE_25519 mul(const FE_25519& a, const FE_25519& b);
      static FE_25519 sqr_iter(const FE_25519& a, size_t iter);
      static FE_25519 sqr(const FE_25519& a) { return sqr_iter(a, 1); }
      static FE_25519 sqr2(const FE_25519& a);
      static FE_25519 pow_22523(const FE_25519& a);
      static FE_25519 invert(const FE_25519& a);

      // TODO remove
      int32_t operator[](size_t i) const { return m_fe[i]; }
      int32_t& operator[](size_t i) { return m_fe[i]; }

   private:

      int32_t m_fe[10];
   };

typedef FE_25519 fe;

/*
fe means field element.
Here the field is
An element t, entries t[0]...t[9], represents the integer
t[0]+2^26 t[1]+2^51 t[2]+2^77 t[3]+2^102 t[4]+...+2^230 t[9].
Bounds on each t[i] vary depending on context.
*/

inline void fe_frombytes(fe& x, const uint8_t* b)
   {
   x.from_bytes(b);
   }

inline void fe_tobytes(uint8_t* b, const fe& x)
   {
   x.to_bytes(b);
   }

inline void fe_copy(fe& a, const fe& b)
   {
   a = b;
   }

inline int fe_isnonzero(const fe& x)
   {
   return x.is_zero() ? 0 : 1;
   }

inline int fe_isnegative(const fe& x)
   {
   return x.is_negative();
   }


inline void fe_0(fe& x)
   {
   x = FE_25519();
   }

inline void fe_1(fe& x)
   {
   x = FE_25519(1);
   }

inline void fe_add(fe& x, const fe& a, const fe& b)
   {
   x = FE_25519::add(a, b);
   }

inline void fe_sub(fe& x, const fe& a, const fe& b)
   {
   x = FE_25519::sub(a, b);
   }

inline void fe_neg(fe& x, const fe& z)
   {
   x = FE_25519::negate(z);
   }

inline void fe_mul(fe& x, const fe& a, const fe& b)
   {
   x = FE_25519::mul(a, b);
   }

inline void fe_sq(fe& x, const fe& z)
   {
   x = FE_25519::sqr(z);
   }

inline void fe_sq_iter(fe& x, const fe& z, size_t iter)
   {
   x = FE_25519::sqr_iter(z, iter);
   }

inline void fe_sq2(fe& x, const fe& z)
   {
   x = FE_25519::sqr2(z);
   }

inline void fe_invert(fe& x, const fe& z)
   {
   x = FE_25519::invert(z);
   }

inline void fe_pow22523(fe& x, const fe& y)
   {
   x = FE_25519::pow_22523(y);
   }

}

namespace Botan {

inline uint64_t load_3(const uint8_t in[3])
   {
   return static_cast<uint64_t>(in[0]) |
      (static_cast<uint64_t>(in[1]) << 8) |
      (static_cast<uint64_t>(in[2]) << 16);
   }

inline uint64_t load_4(const uint8_t* in)
   {
   return load_le<uint32_t>(in, 0);
   }

template<size_t S, int64_t MUL=1>
inline void carry(int64_t& h0, int64_t& h1)
   {
   static_assert(S > 0 && S < 64, "Shift in range");

   const int64_t X1 = (static_cast<int64_t>(1) << S);
   const int64_t X2 = (static_cast<int64_t>(1) << (S - 1));
   int64_t c = (h0 + X2)  >> S;
   h1 += c * MUL;
   h0 -= c * X1;
   }

template<size_t S>
inline void carry0(int64_t& h0, int64_t& h1)
   {
   static_assert(S > 0 && S < 64, "Shift in range");

   const int64_t X1 = (static_cast<int64_t>(1) << S);
   int64_t c = h0 >> S;
   h1 += c;
   h0 -= c * X1;
   }

template<size_t S>
inline void carry0(int32_t& h0, int32_t& h1)
   {
   static_assert(S > 0 && S < 32, "Shift in range");

   const int32_t X1 = (static_cast<int64_t>(1) << S);
   int32_t c = h0 >> S;
   h1 += c;
   h0 -= c * X1;
   }

inline void redc_mul(int64_t& s1,
                     int64_t& s2,
                     int64_t& s3,
                     int64_t& s4,
                     int64_t& s5,
                     int64_t& s6,
                     int64_t& X)
   {
   s1 += X * 666643;
   s2 += X * 470296;
   s3 += X * 654183;
   s4 -= X * 997805;
   s5 += X * 136657;
   s6 -= X * 683901;
   X = 0;
   }

/*
ge means group element.

Here the group is the set of pairs (x,y) of field elements (see fe.h)
satisfying -x^2 + y^2 = 1 + d x^2y^2
where d = -121665/121666.

Representations:
  ge_p3 (extended): (X:Y:Z:T) satisfying x=X/Z, y=Y/Z, XY=ZT
*/

typedef struct
   {
   fe X;
   fe Y;
   fe Z;
   fe T;
   } ge_p3;

int ge_frombytes_negate_vartime(ge_p3*, const uint8_t*);
void ge_scalarmult_base(uint8_t out[32], const uint8_t in[32]);

void ge_double_scalarmult_vartime(uint8_t out[32],
                                  const uint8_t a[],
                                  const ge_p3* A,
                                  const uint8_t b[]);

/*
The set of scalars is \Z/l
where l = 2^252 + 27742317777372353535851937790883648493.
*/

void sc_reduce(uint8_t*);
void sc_muladd(uint8_t*, const uint8_t*, const uint8_t*, const uint8_t*);

}

namespace Botan_FFI {

class BOTAN_UNSTABLE_API FFI_Error final : public Botan::Exception
   {
   public:
      FFI_Error(const std::string& what, int err_code) :
         Exception("FFI error", what),
         m_err_code(err_code)
         {}

      int error_code() const noexcept override { return m_err_code; }

      Botan::ErrorType error_type() const noexcept override { return Botan::ErrorType::InvalidArgument; }

   private:
      int m_err_code;
   };

template<typename T, uint32_t MAGIC>
struct botan_struct
   {
   public:
      botan_struct(T* obj) : m_magic(MAGIC), m_obj(obj) {}
      virtual ~botan_struct() { m_magic = 0; m_obj.reset(); }

      bool magic_ok() const { return (m_magic == MAGIC); }

      T* unsafe_get() const
         {
         return m_obj.get();
         }
   private:
      uint32_t m_magic = 0;
      std::unique_ptr<T> m_obj;
   };

#define BOTAN_FFI_DECLARE_STRUCT(NAME, TYPE, MAGIC) \
   struct NAME final : public Botan_FFI::botan_struct<TYPE, MAGIC> { explicit NAME(TYPE* x) : botan_struct(x) {} }

// Declared in ffi.cpp
int ffi_error_exception_thrown(const char* func_name, const char* exn,
                               int rc = BOTAN_FFI_ERROR_EXCEPTION_THROWN);

template<typename T, uint32_t M>
T& safe_get(botan_struct<T,M>* p)
   {
   if(!p)
      throw FFI_Error("Null pointer argument", BOTAN_FFI_ERROR_NULL_POINTER);
   if(p->magic_ok() == false)
      throw FFI_Error("Bad magic in ffi object", BOTAN_FFI_ERROR_INVALID_OBJECT);

   if(T* t = p->unsafe_get())
      return *t;

   throw FFI_Error("Invalid object pointer", BOTAN_FFI_ERROR_INVALID_OBJECT);
   }

int ffi_guard_thunk(const char* func_name, std::function<int ()>);

template<typename T, uint32_t M, typename F>
int apply_fn(botan_struct<T, M>* o, const char* func_name, F func)
   {
   if(!o)
      return BOTAN_FFI_ERROR_NULL_POINTER;

   if(o->magic_ok() == false)
      return BOTAN_FFI_ERROR_INVALID_OBJECT;

   return ffi_guard_thunk(func_name, [&]() { return func(*o->unsafe_get()); });
   }

#define BOTAN_FFI_DO(T, obj, param, block)                              \
   apply_fn(obj, __func__,                                \
            [=](T& param) -> int { do { block } while(0); return BOTAN_FFI_SUCCESS; })

template<typename T, uint32_t M>
int ffi_delete_object(botan_struct<T, M>* obj, const char* func_name)
   {
   try
      {
      if(obj == nullptr)
         return BOTAN_FFI_SUCCESS; // ignore delete of null objects

      if(obj->magic_ok() == false)
         return BOTAN_FFI_ERROR_INVALID_OBJECT;

      delete obj;
      return BOTAN_FFI_SUCCESS;
      }
   catch(std::exception& e)
      {
      return ffi_error_exception_thrown(func_name, e.what());
      }
   catch(...)
      {
      return ffi_error_exception_thrown(func_name, "unknown exception");
      }
   }

#define BOTAN_FFI_CHECKED_DELETE(o) ffi_delete_object(o, __func__)

inline int write_output(uint8_t out[], size_t* out_len, const uint8_t buf[], size_t buf_len)
   {
   if(out_len == nullptr)
      return BOTAN_FFI_ERROR_NULL_POINTER;

   const size_t avail = *out_len;
   *out_len = buf_len;

   if((avail >= buf_len) && (out != nullptr))
      {
      Botan::copy_mem(out, buf, buf_len);
      return BOTAN_FFI_SUCCESS;
      }
   else
      {
      if(out != nullptr)
         {
         Botan::clear_mem(out, avail);
         }
      return BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }
   }

template<typename Alloc>
int write_vec_output(uint8_t out[], size_t* out_len, const std::vector<uint8_t, Alloc>& buf)
   {
   return write_output(out, out_len, buf.data(), buf.size());
   }

inline int write_str_output(uint8_t out[], size_t* out_len, const std::string& str)
   {
   return write_output(out, out_len,
                       Botan::cast_char_ptr_to_uint8(str.data()),
                       str.size() + 1);
   }

inline int write_str_output(char out[], size_t* out_len, const std::string& str)
   {
   return write_str_output(Botan::cast_char_ptr_to_uint8(out), out_len, str);
   }

inline int write_str_output(char out[], size_t* out_len, const std::vector<uint8_t>& str_vec)
   {
   return write_output(Botan::cast_char_ptr_to_uint8(out),
                       out_len,
                       str_vec.data(),
                       str_vec.size());
   }

}

extern "C" {

BOTAN_FFI_DECLARE_STRUCT(botan_mp_struct, Botan::BigInt, 0xC828B9D2);

}

extern "C" {

BOTAN_FFI_DECLARE_STRUCT(botan_pubkey_struct, Botan::Public_Key, 0x2C286519);
BOTAN_FFI_DECLARE_STRUCT(botan_privkey_struct, Botan::Private_Key, 0x7F96385E);

}

extern "C" {

BOTAN_FFI_DECLARE_STRUCT(botan_rng_struct, Botan::RandomNumberGenerator, 0x4901F9C1);

}

namespace Botan {

/**
* No_Filesystem_Access Exception
*/
class BOTAN_PUBLIC_API(2,0) No_Filesystem_Access final : public Exception
   {
   public:
      No_Filesystem_Access() : Exception("No filesystem access enabled.")
         {}
   };

BOTAN_TEST_API bool has_filesystem_impl();

BOTAN_TEST_API std::vector<std::string> get_files_recursive(const std::string& dir);

}

namespace Botan {

void mceliece_decrypt(secure_vector<uint8_t>& plaintext_out,
                      secure_vector<uint8_t>& error_mask_out,
                      const uint8_t ciphertext[],
                      size_t ciphertext_len,
                      const McEliece_PrivateKey& key);

void mceliece_decrypt(secure_vector<uint8_t>& plaintext_out,
                      secure_vector<uint8_t>& error_mask_out,
                      const secure_vector<uint8_t>& ciphertext,
                      const McEliece_PrivateKey& key);

secure_vector<uint8_t> mceliece_decrypt(
   secure_vector<gf2m> & error_pos,
   const uint8_t *ciphertext, uint32_t ciphertext_len,
   const McEliece_PrivateKey & key);

void mceliece_encrypt(secure_vector<uint8_t>& ciphertext_out,
                      secure_vector<uint8_t>& error_mask_out,
                      const secure_vector<uint8_t>& plaintext,
                      const McEliece_PublicKey& key,
                      RandomNumberGenerator& rng);

McEliece_PrivateKey generate_mceliece_key(RandomNumberGenerator &rng,
                                          uint32_t ext_deg,
                                          uint32_t code_length,
                                          uint32_t t);

}

namespace Botan {

class Bucket;

class BOTAN_TEST_API Memory_Pool final
   {
   public:
      /**
      * Initialize a memory pool. The memory is not owned by *this,
      * it must be freed by the caller.
      * @param pages a list of pages to allocate from
      * @param page_size the system page size, each page should
      *        point to exactly this much memory.
      */
      Memory_Pool(const std::vector<void*>& pages,
                  size_t page_size);

      ~Memory_Pool();

      void* allocate(size_t size);

      bool deallocate(void* p, size_t size) noexcept;

      Memory_Pool(const Memory_Pool&) = delete;
      Memory_Pool(Memory_Pool&&) = delete;

      Memory_Pool& operator=(const Memory_Pool&) = delete;
      Memory_Pool& operator=(Memory_Pool&&) = delete;

   private:
      const size_t m_page_size = 0;

      mutex_type m_mutex;

      std::deque<uint8_t*> m_free_pages;
      std::map<size_t, std::deque<Bucket>> m_buckets_for;
      uintptr_t m_min_page_ptr;
      uintptr_t m_max_page_ptr;
   };

}

namespace Botan {

class BigInt;
class Modular_Reducer;

class Montgomery_Params;

class Montgomery_Exponentation_State;

/*
* Precompute for calculating values g^x mod p
*/
std::shared_ptr<const Montgomery_Exponentation_State>
monty_precompute(std::shared_ptr<const Montgomery_Params> params_p,
                 const BigInt& g,
                 size_t window_bits,
                 bool const_time = true);

/*
* Return g^k mod p
*/
BigInt monty_execute(const Montgomery_Exponentation_State& precomputed_state,
                     const BigInt& k, size_t max_k_bits);

/*
* Return g^k mod p taking variable time depending on k
* @warning only use this if k is public
*/
BigInt monty_execute_vartime(const Montgomery_Exponentation_State& precomputed_state,
                             const BigInt& k);

/**
* Return (x^z1 * y^z2) % p
*/
BigInt monty_multi_exp(std::shared_ptr<const Montgomery_Params> params_p,
                       const BigInt& x,
                       const BigInt& z1,
                       const BigInt& y,
                       const BigInt& z2);

}

namespace Botan {

#if (BOTAN_MP_WORD_BITS == 32)
  typedef uint64_t dword;
  #define BOTAN_HAS_MP_DWORD

#elif (BOTAN_MP_WORD_BITS == 64)
  #if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
    typedef uint128_t dword;
    #define BOTAN_HAS_MP_DWORD
  #else
    // No native 128 bit integer type; use mul64x64_128 instead
  #endif

#else
  #error BOTAN_MP_WORD_BITS must be 32 or 64
#endif

#if defined(BOTAN_TARGET_ARCH_IS_X86_32) && (BOTAN_MP_WORD_BITS == 32)

  #if defined(BOTAN_USE_GCC_INLINE_ASM)
    #define BOTAN_MP_USE_X86_32_ASM
  #elif defined(BOTAN_BUILD_COMPILER_IS_MSVC)
    #define BOTAN_MP_USE_X86_32_MSVC_ASM
  #endif

#elif defined(BOTAN_TARGET_ARCH_IS_X86_64) && (BOTAN_MP_WORD_BITS == 64) && defined(BOTAN_USE_GCC_INLINE_ASM)
  #define BOTAN_MP_USE_X86_64_ASM
#endif

#if defined(BOTAN_MP_USE_X86_32_ASM) || defined(BOTAN_MP_USE_X86_64_ASM)
  #define ASM(x) x "\n\t"
#endif

/*
* Word Multiply/Add
*/
inline word word_madd2(word a, word b, word* c)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ASM("mull %[b]")
      ASM("addl %[c],%[a]")
      ASM("adcl $0,%[carry]")

      : [a]"=a"(a), [b]"=rm"(b), [carry]"=&d"(*c)
      : "0"(a), "1"(b), [c]"g"(*c) : "cc");

   return a;

#elif defined(BOTAN_MP_USE_X86_64_ASM)
      asm(
      ASM("mulq %[b]")
      ASM("addq %[c],%[a]")
      ASM("adcq $0,%[carry]")

      : [a]"=a"(a), [b]"=rm"(b), [carry]"=&d"(*c)
      : "0"(a), "1"(b), [c]"g"(*c) : "cc");

   return a;

#elif defined(BOTAN_HAS_MP_DWORD)
   const dword s = static_cast<dword>(a) * b + *c;
   *c = static_cast<word>(s >> BOTAN_MP_WORD_BITS);
   return static_cast<word>(s);
#else
   static_assert(BOTAN_MP_WORD_BITS == 64, "Unexpected word size");

   word hi = 0, lo = 0;

   mul64x64_128(a, b, &lo, &hi);

   lo += *c;
   hi += (lo < *c); // carry?

   *c = hi;
   return lo;
#endif
   }

/*
* Word Multiply/Add
*/
inline word word_madd3(word a, word b, word c, word* d)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ASM("mull %[b]")

      ASM("addl %[c],%[a]")
      ASM("adcl $0,%[carry]")

      ASM("addl %[d],%[a]")
      ASM("adcl $0,%[carry]")

      : [a]"=a"(a), [b]"=rm"(b), [carry]"=&d"(*d)
      : "0"(a), "1"(b), [c]"g"(c), [d]"g"(*d) : "cc");

   return a;

#elif defined(BOTAN_MP_USE_X86_64_ASM)
   asm(
      ASM("mulq %[b]")

      ASM("addq %[c],%[a]")
      ASM("adcq $0,%[carry]")

      ASM("addq %[d],%[a]")
      ASM("adcq $0,%[carry]")

      : [a]"=a"(a), [b]"=rm"(b), [carry]"=&d"(*d)
      : "0"(a), "1"(b), [c]"g"(c), [d]"g"(*d) : "cc");

   return a;

#elif defined(BOTAN_HAS_MP_DWORD)
   const dword s = static_cast<dword>(a) * b + c + *d;
   *d = static_cast<word>(s >> BOTAN_MP_WORD_BITS);
   return static_cast<word>(s);
#else
   static_assert(BOTAN_MP_WORD_BITS == 64, "Unexpected word size");

   word hi = 0, lo = 0;

   mul64x64_128(a, b, &lo, &hi);

   lo += c;
   hi += (lo < c); // carry?

   lo += *d;
   hi += (lo < *d); // carry?

   *d = hi;
   return lo;
#endif
   }

#if defined(ASM)
  #undef ASM
#endif

}

namespace Botan {

#if defined(BOTAN_MP_USE_X86_32_ASM)

#define ADDSUB2_OP(OPERATION, INDEX)                     \
        ASM("movl 4*" #INDEX "(%[y]), %[carry]")         \
        ASM(OPERATION " %[carry], 4*" #INDEX "(%[x])")   \

#define ADDSUB3_OP(OPERATION, INDEX)                     \
        ASM("movl 4*" #INDEX "(%[x]), %[carry]")         \
        ASM(OPERATION " 4*" #INDEX "(%[y]), %[carry]")   \
        ASM("movl %[carry], 4*" #INDEX "(%[z])")         \

#define LINMUL_OP(WRITE_TO, INDEX)                       \
        ASM("movl 4*" #INDEX "(%[x]),%%eax")             \
        ASM("mull %[y]")                                 \
        ASM("addl %[carry],%%eax")                       \
        ASM("adcl $0,%%edx")                             \
        ASM("movl %%edx,%[carry]")                       \
        ASM("movl %%eax, 4*" #INDEX "(%[" WRITE_TO "])")

#define MULADD_OP(IGNORED, INDEX)                        \
        ASM("movl 4*" #INDEX "(%[x]),%%eax")             \
        ASM("mull %[y]")                                 \
        ASM("addl %[carry],%%eax")                       \
        ASM("adcl $0,%%edx")                             \
        ASM("addl 4*" #INDEX "(%[z]),%%eax")             \
        ASM("adcl $0,%%edx")                             \
        ASM("movl %%edx,%[carry]")                       \
        ASM("movl %%eax, 4*" #INDEX " (%[z])")

#define ADD_OR_SUBTRACT(CORE_CODE)     \
        ASM("rorl %[carry]")           \
        CORE_CODE                      \
        ASM("sbbl %[carry],%[carry]")  \
        ASM("negl %[carry]")

#elif defined(BOTAN_MP_USE_X86_64_ASM)

#define ADDSUB2_OP(OPERATION, INDEX)                     \
        ASM("movq 8*" #INDEX "(%[y]), %[carry]")         \
        ASM(OPERATION " %[carry], 8*" #INDEX "(%[x])")   \

#define ADDSUB3_OP(OPERATION, INDEX)                     \
        ASM("movq 8*" #INDEX "(%[x]), %[carry]")         \
        ASM(OPERATION " 8*" #INDEX "(%[y]), %[carry]")   \
        ASM("movq %[carry], 8*" #INDEX "(%[z])")         \

#define LINMUL_OP(WRITE_TO, INDEX)                       \
        ASM("movq 8*" #INDEX "(%[x]),%%rax")             \
        ASM("mulq %[y]")                                 \
        ASM("addq %[carry],%%rax")                       \
        ASM("adcq $0,%%rdx")                             \
        ASM("movq %%rdx,%[carry]")                       \
        ASM("movq %%rax, 8*" #INDEX "(%[" WRITE_TO "])")

#define MULADD_OP(IGNORED, INDEX)                        \
        ASM("movq 8*" #INDEX "(%[x]),%%rax")             \
        ASM("mulq %[y]")                                 \
        ASM("addq %[carry],%%rax")                       \
        ASM("adcq $0,%%rdx")                             \
        ASM("addq 8*" #INDEX "(%[z]),%%rax")             \
        ASM("adcq $0,%%rdx")                             \
        ASM("movq %%rdx,%[carry]")                       \
        ASM("movq %%rax, 8*" #INDEX " (%[z])")

#define ADD_OR_SUBTRACT(CORE_CODE)     \
        ASM("rorq %[carry]")           \
        CORE_CODE                      \
        ASM("sbbq %[carry],%[carry]")  \
        ASM("negq %[carry]")

#endif

#if defined(ADD_OR_SUBTRACT)

#define ASM(x) x "\n\t"

#define DO_8_TIMES(MACRO, ARG) \
        MACRO(ARG, 0) \
        MACRO(ARG, 1) \
        MACRO(ARG, 2) \
        MACRO(ARG, 3) \
        MACRO(ARG, 4) \
        MACRO(ARG, 5) \
        MACRO(ARG, 6) \
        MACRO(ARG, 7)

#endif

/*
* Word Addition
*/
inline word word_add(word x, word y, word* carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ADD_OR_SUBTRACT(ASM("adcl %[y],%[x]"))
      : [x]"=r"(x), [carry]"=r"(*carry)
      : "0"(x), [y]"rm"(y), "1"(*carry)
      : "cc");
   return x;

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ADD_OR_SUBTRACT(ASM("adcq %[y],%[x]"))
      : [x]"=r"(x), [carry]"=r"(*carry)
      : "0"(x), [y]"rm"(y), "1"(*carry)
      : "cc");
   return x;

#else
   word z = x + y;
   word c1 = (z < x);
   z += *carry;
   *carry = c1 | (z < *carry);
   return z;
#endif
   }

/*
* Eight Word Block Addition, Two Argument
*/
inline word word8_add2(word x[8], const word y[8], word carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB2_OP, "adcl"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), "0"(carry)
      : "cc", "memory");
   return carry;

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB2_OP, "adcq"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), "0"(carry)
      : "cc", "memory");
   return carry;

#elif defined(BOTAN_MP_USE_X86_32_MSVC_ASM)

   __asm {
      mov edx,[x]
      mov esi,[y]
      xor eax,eax
      sub eax,[carry] //force CF=1 iff *carry==1
      mov eax,[esi]
      adc [edx],eax
      mov eax,[esi+4]
      adc [edx+4],eax
      mov eax,[esi+8]
      adc [edx+8],eax
      mov eax,[esi+12]
      adc [edx+12],eax
      mov eax,[esi+16]
      adc [edx+16],eax
      mov eax,[esi+20]
      adc [edx+20],eax
      mov eax,[esi+24]
      adc [edx+24],eax
      mov eax,[esi+28]
      adc [edx+28],eax
      sbb eax,eax
      neg eax
      }

#else
   x[0] = word_add(x[0], y[0], &carry);
   x[1] = word_add(x[1], y[1], &carry);
   x[2] = word_add(x[2], y[2], &carry);
   x[3] = word_add(x[3], y[3], &carry);
   x[4] = word_add(x[4], y[4], &carry);
   x[5] = word_add(x[5], y[5], &carry);
   x[6] = word_add(x[6], y[6], &carry);
   x[7] = word_add(x[7], y[7], &carry);
   return carry;
#endif
   }

/*
* Eight Word Block Addition, Three Argument
*/
inline word word8_add3(word z[8], const word x[8],
                       const word y[8], word carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB3_OP, "adcl"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), [z]"r"(z), "0"(carry)
      : "cc", "memory");
   return carry;

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB3_OP, "adcq"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), [z]"r"(z), "0"(carry)
      : "cc", "memory");
   return carry;

#elif defined(BOTAN_MP_USE_X86_32_MSVC_ASM)

    __asm {
      mov edi,[x]
      mov esi,[y]
      mov ebx,[z]
      xor eax,eax
      sub eax,[carry] //force CF=1 iff *carry==1
      mov eax,[edi]
      adc eax,[esi]
      mov [ebx],eax

      mov eax,[edi+4]
      adc eax,[esi+4]
      mov [ebx+4],eax

      mov eax,[edi+8]
      adc eax,[esi+8]
      mov [ebx+8],eax

      mov eax,[edi+12]
      adc eax,[esi+12]
      mov [ebx+12],eax

      mov eax,[edi+16]
      adc eax,[esi+16]
      mov [ebx+16],eax

      mov eax,[edi+20]
      adc eax,[esi+20]
      mov [ebx+20],eax

      mov eax,[edi+24]
      adc eax,[esi+24]
      mov [ebx+24],eax

      mov eax,[edi+28]
      adc eax,[esi+28]
      mov [ebx+28],eax

      sbb eax,eax
      neg eax
      }

#else
   z[0] = word_add(x[0], y[0], &carry);
   z[1] = word_add(x[1], y[1], &carry);
   z[2] = word_add(x[2], y[2], &carry);
   z[3] = word_add(x[3], y[3], &carry);
   z[4] = word_add(x[4], y[4], &carry);
   z[5] = word_add(x[5], y[5], &carry);
   z[6] = word_add(x[6], y[6], &carry);
   z[7] = word_add(x[7], y[7], &carry);
   return carry;
#endif
   }

/*
* Word Subtraction
*/
inline word word_sub(word x, word y, word* carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ADD_OR_SUBTRACT(ASM("sbbl %[y],%[x]"))
      : [x]"=r"(x), [carry]"=r"(*carry)
      : "0"(x), [y]"rm"(y), "1"(*carry)
      : "cc");
   return x;

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ADD_OR_SUBTRACT(ASM("sbbq %[y],%[x]"))
      : [x]"=r"(x), [carry]"=r"(*carry)
      : "0"(x), [y]"rm"(y), "1"(*carry)
      : "cc");
   return x;

#else
   word t0 = x - y;
   word c1 = (t0 > x);
   word z = t0 - *carry;
   *carry = c1 | (z > t0);
   return z;
#endif
   }

/*
* Eight Word Block Subtraction, Two Argument
*/
inline word word8_sub2(word x[8], const word y[8], word carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB2_OP, "sbbl"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), "0"(carry)
      : "cc", "memory");
   return carry;

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB2_OP, "sbbq"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), "0"(carry)
      : "cc", "memory");
   return carry;

#elif defined(BOTAN_MP_USE_X86_32_MSVC_ASM)

    __asm {
      mov edi,[x]
      mov esi,[y]
      xor eax,eax
      sub eax,[carry] //force CF=1 iff *carry==1
      mov eax,[edi]
      sbb eax,[esi]
      mov [edi],eax
      mov eax,[edi+4]
      sbb eax,[esi+4]
      mov [edi+4],eax
      mov eax,[edi+8]
      sbb eax,[esi+8]
      mov [edi+8],eax
      mov eax,[edi+12]
      sbb eax,[esi+12]
      mov [edi+12],eax
      mov eax,[edi+16]
      sbb eax,[esi+16]
      mov [edi+16],eax
      mov eax,[edi+20]
      sbb eax,[esi+20]
      mov [edi+20],eax
      mov eax,[edi+24]
      sbb eax,[esi+24]
      mov [edi+24],eax
      mov eax,[edi+28]
      sbb eax,[esi+28]
      mov [edi+28],eax
      sbb eax,eax
      neg eax
      }

#else
   x[0] = word_sub(x[0], y[0], &carry);
   x[1] = word_sub(x[1], y[1], &carry);
   x[2] = word_sub(x[2], y[2], &carry);
   x[3] = word_sub(x[3], y[3], &carry);
   x[4] = word_sub(x[4], y[4], &carry);
   x[5] = word_sub(x[5], y[5], &carry);
   x[6] = word_sub(x[6], y[6], &carry);
   x[7] = word_sub(x[7], y[7], &carry);
   return carry;
#endif
   }

/*
* Eight Word Block Subtraction, Two Argument
*/
inline word word8_sub2_rev(word x[8], const word y[8], word carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB3_OP, "sbbl"))
      : [carry]"=r"(carry)
      : [x]"r"(y), [y]"r"(x), [z]"r"(x), "0"(carry)
      : "cc", "memory");
   return carry;

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB3_OP, "sbbq"))
      : [carry]"=r"(carry)
      : [x]"r"(y), [y]"r"(x), [z]"r"(x), "0"(carry)
      : "cc", "memory");
   return carry;

#else
   x[0] = word_sub(y[0], x[0], &carry);
   x[1] = word_sub(y[1], x[1], &carry);
   x[2] = word_sub(y[2], x[2], &carry);
   x[3] = word_sub(y[3], x[3], &carry);
   x[4] = word_sub(y[4], x[4], &carry);
   x[5] = word_sub(y[5], x[5], &carry);
   x[6] = word_sub(y[6], x[6], &carry);
   x[7] = word_sub(y[7], x[7], &carry);
   return carry;
#endif
   }

/*
* Eight Word Block Subtraction, Three Argument
*/
inline word word8_sub3(word z[8], const word x[8],
                       const word y[8], word carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB3_OP, "sbbl"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), [z]"r"(z), "0"(carry)
      : "cc", "memory");
   return carry;

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ADD_OR_SUBTRACT(DO_8_TIMES(ADDSUB3_OP, "sbbq"))
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"r"(y), [z]"r"(z), "0"(carry)
      : "cc", "memory");
   return carry;

#elif defined(BOTAN_MP_USE_X86_32_MSVC_ASM)

   __asm {
      mov edi,[x]
      mov esi,[y]
      xor eax,eax
      sub eax,[carry] //force CF=1 iff *carry==1
      mov ebx,[z]
      mov eax,[edi]
      sbb eax,[esi]
      mov [ebx],eax
      mov eax,[edi+4]
      sbb eax,[esi+4]
      mov [ebx+4],eax
      mov eax,[edi+8]
      sbb eax,[esi+8]
      mov [ebx+8],eax
      mov eax,[edi+12]
      sbb eax,[esi+12]
      mov [ebx+12],eax
      mov eax,[edi+16]
      sbb eax,[esi+16]
      mov [ebx+16],eax
      mov eax,[edi+20]
      sbb eax,[esi+20]
      mov [ebx+20],eax
      mov eax,[edi+24]
      sbb eax,[esi+24]
      mov [ebx+24],eax
      mov eax,[edi+28]
      sbb eax,[esi+28]
      mov [ebx+28],eax
      sbb eax,eax
      neg eax
      }

#else
   z[0] = word_sub(x[0], y[0], &carry);
   z[1] = word_sub(x[1], y[1], &carry);
   z[2] = word_sub(x[2], y[2], &carry);
   z[3] = word_sub(x[3], y[3], &carry);
   z[4] = word_sub(x[4], y[4], &carry);
   z[5] = word_sub(x[5], y[5], &carry);
   z[6] = word_sub(x[6], y[6], &carry);
   z[7] = word_sub(x[7], y[7], &carry);
   return carry;
#endif
   }

/*
* Eight Word Block Linear Multiplication
*/
inline word word8_linmul2(word x[8], word y, word carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      DO_8_TIMES(LINMUL_OP, "x")
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"rm"(y), "0"(carry)
      : "cc", "%eax", "%edx");
   return carry;

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      DO_8_TIMES(LINMUL_OP, "x")
      : [carry]"=r"(carry)
      : [x]"r"(x), [y]"rm"(y), "0"(carry)
      : "cc", "%rax", "%rdx");
   return carry;

#elif defined(BOTAN_MP_USE_X86_32_MSVC_ASM)

   __asm {
      mov esi,[x]
      mov eax,[esi]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,[carry]      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [esi],eax        //load a

      mov eax,[esi+4]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [esi+4],eax        //load a

      mov eax,[esi+8]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [esi+8],eax        //load a

      mov eax,[esi+12]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [esi+12],eax        //load a

      mov eax,[esi+16]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [esi+16],eax        //load a

      mov eax,[esi+20]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [esi+20],eax        //load a

      mov eax,[esi+24]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [esi+24],eax        //load a

      mov eax,[esi+28]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov [esi+28],eax        //load a

      mov eax,edx      //store carry
      }

#else
   x[0] = word_madd2(x[0], y, &carry);
   x[1] = word_madd2(x[1], y, &carry);
   x[2] = word_madd2(x[2], y, &carry);
   x[3] = word_madd2(x[3], y, &carry);
   x[4] = word_madd2(x[4], y, &carry);
   x[5] = word_madd2(x[5], y, &carry);
   x[6] = word_madd2(x[6], y, &carry);
   x[7] = word_madd2(x[7], y, &carry);
   return carry;
#endif
   }

/*
* Eight Word Block Linear Multiplication
*/
inline word word8_linmul3(word z[8], const word x[8], word y, word carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      DO_8_TIMES(LINMUL_OP, "z")
      : [carry]"=r"(carry)
      : [z]"r"(z), [x]"r"(x), [y]"rm"(y), "0"(carry)
      : "cc", "%eax", "%edx");
   return carry;

#elif defined(BOTAN_MP_USE_X86_64_ASM)
   asm(
      DO_8_TIMES(LINMUL_OP, "z")
      : [carry]"=r"(carry)
      : [z]"r"(z), [x]"r"(x), [y]"rm"(y), "0"(carry)
      : "cc", "%rax", "%rdx");
   return carry;

#elif defined(BOTAN_MP_USE_X86_32_MSVC_ASM)

   __asm {
      mov edi,[z]
      mov esi,[x]
      mov eax,[esi]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,[carry]    //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [edi],eax        //load a

      mov eax,[esi+4]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [edi+4],eax        //load a

      mov eax,[esi+8]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [edi+8],eax        //load a

      mov eax,[esi+12]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [edi+12],eax        //load a

      mov eax,[esi+16]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [edi+16],eax        //load a

      mov eax,[esi+20]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [edi+20],eax        //load a

      mov eax,[esi+24]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov ecx,edx      //store carry
      mov [edi+24],eax        //load a

      mov eax,[esi+28]        //load a
      mul [y]           //edx(hi):eax(lo)=a*b
      add eax,ecx      //sum lo carry
      adc edx,0          //sum hi carry
      mov [edi+28],eax        //load a
      mov eax,edx      //store carry
      }

#else
   z[0] = word_madd2(x[0], y, &carry);
   z[1] = word_madd2(x[1], y, &carry);
   z[2] = word_madd2(x[2], y, &carry);
   z[3] = word_madd2(x[3], y, &carry);
   z[4] = word_madd2(x[4], y, &carry);
   z[5] = word_madd2(x[5], y, &carry);
   z[6] = word_madd2(x[6], y, &carry);
   z[7] = word_madd2(x[7], y, &carry);
   return carry;
#endif
   }

/*
* Eight Word Block Multiply/Add
*/
inline word word8_madd3(word z[8], const word x[8], word y, word carry)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      DO_8_TIMES(MULADD_OP, "")
      : [carry]"=r"(carry)
      : [z]"r"(z), [x]"r"(x), [y]"rm"(y), "0"(carry)
      : "cc", "%eax", "%edx");
   return carry;

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      DO_8_TIMES(MULADD_OP, "")
      : [carry]"=r"(carry)
      : [z]"r"(z), [x]"r"(x), [y]"rm"(y), "0"(carry)
      : "cc", "%rax", "%rdx");
   return carry;

#else
   z[0] = word_madd3(x[0], y, z[0], &carry);
   z[1] = word_madd3(x[1], y, z[1], &carry);
   z[2] = word_madd3(x[2], y, z[2], &carry);
   z[3] = word_madd3(x[3], y, z[3], &carry);
   z[4] = word_madd3(x[4], y, z[4], &carry);
   z[5] = word_madd3(x[5], y, z[5], &carry);
   z[6] = word_madd3(x[6], y, z[6], &carry);
   z[7] = word_madd3(x[7], y, z[7], &carry);
   return carry;
#endif
   }

/*
* Multiply-Add Accumulator
* (w2,w1,w0) += x * y
*/
inline void word3_muladd(word* w2, word* w1, word* w0, word x, word y)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   word z0 = 0, z1 = 0;

   asm ("mull %[y]"
        : "=a"(z0),"=d"(z1)
        : "a"(x), [y]"rm"(y)
        : "cc");

   asm(ASM("addl %[z0],%[w0]")
       ASM("adcl %[z1],%[w1]")
       ASM("adcl $0,%[w2]")

       : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
       : [z0]"r"(z0), [z1]"r"(z1), "0"(*w0), "1"(*w1), "2"(*w2)
       : "cc");

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   word z0 = 0, z1 = 0;

   asm ("mulq %[y]"
        : "=a"(z0),"=d"(z1)
        : "a"(x), [y]"rm"(y)
        : "cc");

   asm(ASM("addq %[z0],%[w0]")
       ASM("adcq %[z1],%[w1]")
       ASM("adcq $0,%[w2]")

       : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
       : [z0]"r"(z0), [z1]"r"(z1), "0"(*w0), "1"(*w1), "2"(*w2)
       : "cc");

#else
   word carry = *w0;
   *w0 = word_madd2(x, y, &carry);
   *w1 += carry;
   *w2 += (*w1 < carry);
#endif
   }

/*
* 3-word addition
* (w2,w1,w0) += x
*/
inline void word3_add(word* w2, word* w1, word* w0, word x)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ASM("addl %[x],%[w0]")
      ASM("adcl $0,%[w1]")
      ASM("adcl $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [x]"r"(x), "0"(*w0), "1"(*w1), "2"(*w2)
      : "cc");

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ASM("addq %[x],%[w0]")
      ASM("adcq $0,%[w1]")
      ASM("adcq $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [x]"r"(x), "0"(*w0), "1"(*w1), "2"(*w2)
      : "cc");

#else
   *w0 += x;
   word c1 = (*w0 < x);
   *w1 += c1;
   word c2 = (*w1 < c1);
   *w2 += c2;
#endif
   }

/*
* Multiply-Add Accumulator
* (w2,w1,w0) += 2 * x * y
*/
inline void word3_muladd_2(word* w2, word* w1, word* w0, word x, word y)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)

   word z0 = 0, z1 = 0;

   asm ("mull %[y]"
        : "=a"(z0),"=d"(z1)
        : "a"(x), [y]"rm"(y)
        : "cc");

   asm(
      ASM("addl %[z0],%[w0]")
      ASM("adcl %[z1],%[w1]")
      ASM("adcl $0,%[w2]")

      ASM("addl %[z0],%[w0]")
      ASM("adcl %[z1],%[w1]")
      ASM("adcl $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [z0]"r"(z0), [z1]"r"(z1), "0"(*w0), "1"(*w1), "2"(*w2)
      : "cc");

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   word z0 = 0, z1 = 0;

   asm ("mulq %[y]"
        : "=a"(z0),"=d"(z1)
        : "a"(x), [y]"rm"(y)
        : "cc");

   asm(
      ASM("addq %[z0],%[w0]")
      ASM("adcq %[z1],%[w1]")
      ASM("adcq $0,%[w2]")

      ASM("addq %[z0],%[w0]")
      ASM("adcq %[z1],%[w1]")
      ASM("adcq $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [z0]"r"(z0), [z1]"r"(z1), "0"(*w0), "1"(*w1), "2"(*w2)
      : "cc");

#else
   word carry = 0;
   x = word_madd2(x, y, &carry);
   y = carry;

   word top = (y >> (BOTAN_MP_WORD_BITS-1));
   y <<= 1;
   y |= (x >> (BOTAN_MP_WORD_BITS-1));
   x <<= 1;

   carry = 0;
   *w0 = word_add(*w0, x, &carry);
   *w1 = word_add(*w1, y, &carry);
   *w2 = word_add(*w2, top, &carry);
#endif
   }

#if defined(ASM)
  #undef ASM
  #undef DO_8_TIMES
  #undef ADD_OR_SUBTRACT
  #undef ADDSUB2_OP
  #undef ADDSUB3_OP
  #undef LINMUL_OP
  #undef MULADD_OP
#endif

}

namespace Botan {

const word MP_WORD_MASK = ~static_cast<word>(0);
const word MP_WORD_TOP_BIT = static_cast<word>(1) << (8*sizeof(word) - 1);
const word MP_WORD_MAX = MP_WORD_MASK;

/*
* If cond == 0, does nothing.
* If cond > 0, swaps x[0:size] with y[0:size]
* Runs in constant time
*/
inline void bigint_cnd_swap(word cnd, word x[], word y[], size_t size)
   {
   const auto mask = CT::Mask<word>::expand(cnd);

   for(size_t i = 0; i != size; ++i)
      {
      const word a = x[i];
      const word b = y[i];
      x[i] = mask.select(b, a);
      y[i] = mask.select(a, b);
      }
   }

inline word bigint_cnd_add(word cnd, word x[], word x_size,
                           const word y[], size_t y_size)
   {
   BOTAN_ASSERT(x_size >= y_size, "Expected sizes");

   const auto mask = CT::Mask<word>::expand(cnd);

   word carry = 0;

   const size_t blocks = y_size - (y_size % 8);
   word z[8] = { 0 };

   for(size_t i = 0; i != blocks; i += 8)
      {
      carry = word8_add3(z, x + i, y + i, carry);
      mask.select_n(x + i, z, x + i, 8);
      }

   for(size_t i = blocks; i != y_size; ++i)
      {
      z[0] = word_add(x[i], y[i], &carry);
      x[i] = mask.select(z[0], x[i]);
      }

   for(size_t i = y_size; i != x_size; ++i)
      {
      z[0] = word_add(x[i], 0, &carry);
      x[i] = mask.select(z[0], x[i]);
      }

   return mask.if_set_return(carry);
   }

/*
* If cond > 0 adds x[0:size] and y[0:size] and returns carry
* Runs in constant time
*/
inline word bigint_cnd_add(word cnd, word x[], const word y[], size_t size)
   {
   return bigint_cnd_add(cnd, x, size, y, size);
   }

/*
* If cond > 0 subtracts x[0:size] and y[0:size] and returns borrow
* Runs in constant time
*/
inline word bigint_cnd_sub(word cnd,
                           word x[], size_t x_size,
                           const word y[], size_t y_size)
   {
   BOTAN_ASSERT(x_size >= y_size, "Expected sizes");

   const auto mask = CT::Mask<word>::expand(cnd);

   word carry = 0;

   const size_t blocks = y_size - (y_size % 8);
   word z[8] = { 0 };

   for(size_t i = 0; i != blocks; i += 8)
      {
      carry = word8_sub3(z, x + i, y + i, carry);
      mask.select_n(x + i, z, x + i, 8);
      }

   for(size_t i = blocks; i != y_size; ++i)
      {
      z[0] = word_sub(x[i], y[i], &carry);
      x[i] = mask.select(z[0], x[i]);
      }

   for(size_t i = y_size; i != x_size; ++i)
      {
      z[0] = word_sub(x[i], 0, &carry);
      x[i] = mask.select(z[0], x[i]);
      }

   return mask.if_set_return(carry);
   }

/*
* If cond > 0 adds x[0:size] and y[0:size] and returns carry
* Runs in constant time
*/
inline word bigint_cnd_sub(word cnd, word x[], const word y[], size_t size)
   {
   return bigint_cnd_sub(cnd, x, size, y, size);
   }


/*
* Equivalent to
*   bigint_cnd_add( mask, x, y, size);
*   bigint_cnd_sub(~mask, x, y, size);
*
* Mask must be either 0 or all 1 bits
*/
inline void bigint_cnd_add_or_sub(CT::Mask<word> mask, word x[], const word y[], size_t size)
   {
   const size_t blocks = size - (size % 8);

   word carry = 0;
   word borrow = 0;

   word t0[8] = { 0 };
   word t1[8] = { 0 };

   for(size_t i = 0; i != blocks; i += 8)
      {
      carry = word8_add3(t0, x + i, y + i, carry);
      borrow = word8_sub3(t1, x + i, y + i, borrow);

      for(size_t j = 0; j != 8; ++j)
         x[i+j] = mask.select(t0[j], t1[j]);
      }

   for(size_t i = blocks; i != size; ++i)
      {
      const word a = word_add(x[i], y[i], &carry);
      const word s = word_sub(x[i], y[i], &borrow);

      x[i] = mask.select(a, s);
      }
   }

/*
* Equivalent to
*   bigint_cnd_add( mask, x, size, y, size);
*   bigint_cnd_sub(~mask, x, size, z, size);
*
* Mask must be either 0 or all 1 bits
*
* Returns the carry or borrow resp
*/
inline word bigint_cnd_addsub(CT::Mask<word> mask, word x[],
                              const word y[], const word z[],
                              size_t size)
   {
   const size_t blocks = size - (size % 8);

   word carry = 0;
   word borrow = 0;

   word t0[8] = { 0 };
   word t1[8] = { 0 };

   for(size_t i = 0; i != blocks; i += 8)
      {
      carry = word8_add3(t0, x + i, y + i, carry);
      borrow = word8_sub3(t1, x + i, z + i, borrow);

      for(size_t j = 0; j != 8; ++j)
         x[i+j] = mask.select(t0[j], t1[j]);
      }

   for(size_t i = blocks; i != size; ++i)
      {
      t0[0] = word_add(x[i], y[i], &carry);
      t1[0] = word_sub(x[i], z[i], &borrow);
      x[i] = mask.select(t0[0], t1[0]);
      }

   return mask.select(carry, borrow);
   }

/*
* 2s complement absolute value
* If cond > 0 sets x to ~x + 1
* Runs in constant time
*/
inline void bigint_cnd_abs(word cnd, word x[], size_t size)
   {
   const auto mask = CT::Mask<word>::expand(cnd);

   word carry = mask.if_set_return(1);
   for(size_t i = 0; i != size; ++i)
      {
      const word z = word_add(~x[i], 0, &carry);
      x[i] = mask.select(z, x[i]);
      }
   }

/**
* Two operand addition with carry out
*/
inline word bigint_add2_nc(word x[], size_t x_size, const word y[], size_t y_size)
   {
   word carry = 0;

   BOTAN_ASSERT(x_size >= y_size, "Expected sizes");

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_add2(x + i, y + i, carry);

   for(size_t i = blocks; i != y_size; ++i)
      x[i] = word_add(x[i], y[i], &carry);

   for(size_t i = y_size; i != x_size; ++i)
      x[i] = word_add(x[i], 0, &carry);

   return carry;
   }

/**
* Three operand addition with carry out
*/
inline word bigint_add3_nc(word z[],
                           const word x[], size_t x_size,
                           const word y[], size_t y_size)
   {
   if(x_size < y_size)
      { return bigint_add3_nc(z, y, y_size, x, x_size); }

   word carry = 0;

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_add3(z + i, x + i, y + i, carry);

   for(size_t i = blocks; i != y_size; ++i)
      z[i] = word_add(x[i], y[i], &carry);

   for(size_t i = y_size; i != x_size; ++i)
      z[i] = word_add(x[i], 0, &carry);

   return carry;
   }

/**
* Two operand addition
* @param x the first operand (and output)
* @param x_size size of x
* @param y the second operand
* @param y_size size of y (must be >= x_size)
*/
inline void bigint_add2(word x[], size_t x_size,
                        const word y[], size_t y_size)
   {
   x[x_size] += bigint_add2_nc(x, x_size, y, y_size);
   }

/**
* Three operand addition
*/
inline void bigint_add3(word z[],
                        const word x[], size_t x_size,
                        const word y[], size_t y_size)
   {
   z[x_size > y_size ? x_size : y_size] +=
      bigint_add3_nc(z, x, x_size, y, y_size);
   }

/**
* Two operand subtraction
*/
inline word bigint_sub2(word x[], size_t x_size,
                        const word y[], size_t y_size)
   {
   word borrow = 0;

   BOTAN_ASSERT(x_size >= y_size, "Expected sizes");

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      borrow = word8_sub2(x + i, y + i, borrow);

   for(size_t i = blocks; i != y_size; ++i)
      x[i] = word_sub(x[i], y[i], &borrow);

   for(size_t i = y_size; i != x_size; ++i)
      x[i] = word_sub(x[i], 0, &borrow);

   return borrow;
   }

/**
* Two operand subtraction, x = y - x; assumes y >= x
*/
inline void bigint_sub2_rev(word x[], const word y[], size_t y_size)
   {
   word borrow = 0;

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      borrow = word8_sub2_rev(x + i, y + i, borrow);

   for(size_t i = blocks; i != y_size; ++i)
      x[i] = word_sub(y[i], x[i], &borrow);

   BOTAN_ASSERT(borrow == 0, "y must be greater than x");
   }

/**
* Three operand subtraction
*/
inline word bigint_sub3(word z[],
                        const word x[], size_t x_size,
                        const word y[], size_t y_size)
   {
   word borrow = 0;

   BOTAN_ASSERT(x_size >= y_size, "Expected sizes");

   const size_t blocks = y_size - (y_size % 8);

   for(size_t i = 0; i != blocks; i += 8)
      borrow = word8_sub3(z + i, x + i, y + i, borrow);

   for(size_t i = blocks; i != y_size; ++i)
      z[i] = word_sub(x[i], y[i], &borrow);

   for(size_t i = y_size; i != x_size; ++i)
      z[i] = word_sub(x[i], 0, &borrow);

   return borrow;
   }

/**
* Return abs(x-y), ie if x >= y, then compute z = x - y
* Otherwise compute z = y - x
* No borrow is possible since the result is always >= 0
*
* Returns ~0 if x >= y or 0 if x < y
* @param z output array of at least N words
* @param x input array of N words
* @param y input array of N words
* @param N length of x and y
* @param ws array of at least 2*N words
*/
inline CT::Mask<word>
bigint_sub_abs(word z[],
               const word x[], const word y[], size_t N,
               word ws[])
   {
   // Subtract in both direction then conditional copy out the result

   word* ws0 = ws;
   word* ws1 = ws + N;

   word borrow0 = 0;
   word borrow1 = 0;

   const size_t blocks = N - (N % 8);

   for(size_t i = 0; i != blocks; i += 8)
      {
      borrow0 = word8_sub3(ws0 + i, x + i, y + i, borrow0);
      borrow1 = word8_sub3(ws1 + i, y + i, x + i, borrow1);
      }

   for(size_t i = blocks; i != N; ++i)
      {
      ws0[i] = word_sub(x[i], y[i], &borrow0);
      ws1[i] = word_sub(y[i], x[i], &borrow1);
      }

   return CT::conditional_copy_mem(borrow0, z, ws1, ws0, N);
   }

/*
* Shift Operations
*/
inline void bigint_shl1(word x[], size_t x_size, size_t x_words,
                        size_t word_shift, size_t bit_shift)
   {
   copy_mem(x + word_shift, x, x_words);
   clear_mem(x, word_shift);

   const auto carry_mask = CT::Mask<word>::expand(bit_shift);
   const size_t carry_shift = carry_mask.if_set_return(BOTAN_MP_WORD_BITS - bit_shift);

   word carry = 0;
   for(size_t i = word_shift; i != x_size; ++i)
      {
      const word w = x[i];
      x[i] = (w << bit_shift) | carry;
      carry = carry_mask.if_set_return(w >> carry_shift);
      }
   }

inline void bigint_shr1(word x[], size_t x_size,
                        size_t word_shift, size_t bit_shift)
   {
   const size_t top = x_size >= word_shift ? (x_size - word_shift) : 0;

   copy_mem(x, x + word_shift, top);
   clear_mem(x + top, std::min(word_shift, x_size));

   const auto carry_mask = CT::Mask<word>::expand(bit_shift);
   const size_t carry_shift = carry_mask.if_set_return(BOTAN_MP_WORD_BITS - bit_shift);

   word carry = 0;

   for(size_t i = 0; i != top; ++i)
      {
      const word w = x[top - i - 1];
      x[top-i-1] = (w >> bit_shift) | carry;
      carry = carry_mask.if_set_return(w << carry_shift);
      }
   }

inline void bigint_shl2(word y[], const word x[], size_t x_size,
                        size_t word_shift, size_t bit_shift)
   {
   copy_mem(y + word_shift, x, x_size);

   const auto carry_mask = CT::Mask<word>::expand(bit_shift);
   const size_t carry_shift = carry_mask.if_set_return(BOTAN_MP_WORD_BITS - bit_shift);

   word carry = 0;
   for(size_t i = word_shift; i != x_size + word_shift + 1; ++i)
      {
      const word w = y[i];
      y[i] = (w << bit_shift) | carry;
      carry = carry_mask.if_set_return(w >> carry_shift);
      }
   }

inline void bigint_shr2(word y[], const word x[], size_t x_size,
                        size_t word_shift, size_t bit_shift)
   {
   const size_t new_size = x_size < word_shift ? 0 : (x_size - word_shift);

   copy_mem(y, x + word_shift, new_size);

   const auto carry_mask = CT::Mask<word>::expand(bit_shift);
   const size_t carry_shift = carry_mask.if_set_return(BOTAN_MP_WORD_BITS - bit_shift);

   word carry = 0;
   for(size_t i = new_size; i > 0; --i)
      {
      word w = y[i-1];
      y[i-1] = (w >> bit_shift) | carry;
      carry = carry_mask.if_set_return(w << carry_shift);
      }
   }

/*
* Linear Multiply
*/
inline void bigint_linmul2(word x[], size_t x_size, word y)
   {
   const size_t blocks = x_size - (x_size % 8);

   word carry = 0;

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_linmul2(x + i, y, carry);

   for(size_t i = blocks; i != x_size; ++i)
      x[i] = word_madd2(x[i], y, &carry);

   x[x_size] = carry;
   }

inline void bigint_linmul3(word z[], const word x[], size_t x_size, word y)
   {
   const size_t blocks = x_size - (x_size % 8);

   word carry = 0;

   for(size_t i = 0; i != blocks; i += 8)
      carry = word8_linmul3(z + i, x + i, y, carry);

   for(size_t i = blocks; i != x_size; ++i)
      z[i] = word_madd2(x[i], y, &carry);

   z[x_size] = carry;
   }

/**
* Compare x and y
* Return -1 if x < y
* Return 0 if x == y
* Return 1 if x > y
*/
inline int32_t bigint_cmp(const word x[], size_t x_size,
                          const word y[], size_t y_size)
   {
   static_assert(sizeof(word) >= sizeof(uint32_t), "Size assumption");

   const word LT = static_cast<word>(-1);
   const word EQ = 0;
   const word GT = 1;

   const size_t common_elems = std::min(x_size, y_size);

   word result = EQ; // until found otherwise

   for(size_t i = 0; i != common_elems; i++)
      {
      const auto is_eq = CT::Mask<word>::is_equal(x[i], y[i]);
      const auto is_lt = CT::Mask<word>::is_lt(x[i], y[i]);

      result = is_eq.select(result, is_lt.select(LT, GT));
      }

   if(x_size < y_size)
      {
      word mask = 0;
      for(size_t i = x_size; i != y_size; i++)
         mask |= y[i];

      // If any bits were set in high part of y, then x < y
      result = CT::Mask<word>::is_zero(mask).select(result, LT);
      }
   else if(y_size < x_size)
      {
      word mask = 0;
      for(size_t i = y_size; i != x_size; i++)
         mask |= x[i];

      // If any bits were set in high part of x, then x > y
      result = CT::Mask<word>::is_zero(mask).select(result, GT);
      }

   CT::unpoison(result);
   BOTAN_DEBUG_ASSERT(result == LT || result == GT || result == EQ);
   return static_cast<int32_t>(result);
   }

/**
* Compare x and y
* Return ~0 if x[0:x_size] < y[0:y_size] or 0 otherwise
* If lt_or_equal is true, returns ~0 also for x == y
*/
inline CT::Mask<word>
bigint_ct_is_lt(const word x[], size_t x_size,
                const word y[], size_t y_size,
                bool lt_or_equal = false)
   {
   const size_t common_elems = std::min(x_size, y_size);

   auto is_lt = CT::Mask<word>::expand(lt_or_equal);

   for(size_t i = 0; i != common_elems; i++)
      {
      const auto eq = CT::Mask<word>::is_equal(x[i], y[i]);
      const auto lt = CT::Mask<word>::is_lt(x[i], y[i]);
      is_lt = eq.select_mask(is_lt, lt);
      }

   if(x_size < y_size)
      {
      word mask = 0;
      for(size_t i = x_size; i != y_size; i++)
         mask |= y[i];
      // If any bits were set in high part of y, then is_lt should be forced true
      is_lt |= CT::Mask<word>::expand(mask);
      }
   else if(y_size < x_size)
      {
      word mask = 0;
      for(size_t i = y_size; i != x_size; i++)
         mask |= x[i];

      // If any bits were set in high part of x, then is_lt should be false
      is_lt &= CT::Mask<word>::is_zero(mask);
      }

   return is_lt;
   }

inline CT::Mask<word>
bigint_ct_is_eq(const word x[], size_t x_size,
                const word y[], size_t y_size)
   {
   const size_t common_elems = std::min(x_size, y_size);

   word diff = 0;

   for(size_t i = 0; i != common_elems; i++)
      {
      diff |= (x[i] ^ y[i]);
      }

   // If any bits were set in high part of x/y, then they are not equal
   if(x_size < y_size)
      {
      for(size_t i = x_size; i != y_size; i++)
         diff |= y[i];
      }
   else if(y_size < x_size)
      {
      for(size_t i = y_size; i != x_size; i++)
         diff |= x[i];
      }

   return CT::Mask<word>::is_zero(diff);
   }

/**
* Set z to abs(x-y), ie if x >= y, then compute z = x - y
* Otherwise compute z = y - x
* No borrow is possible since the result is always >= 0
*
* Return the relative size of x vs y (-1, 0, 1)
*
* @param z output array of max(x_size,y_size) words
* @param x input param
* @param x_size length of x
* @param y input param
* @param y_size length of y
*/
inline int32_t
bigint_sub_abs(word z[],
               const word x[], size_t x_size,
               const word y[], size_t y_size)
   {
   const int32_t relative_size = bigint_cmp(x, x_size, y, y_size);

   // Swap if relative_size == -1
   CT::conditional_swap_ptr(relative_size < 0, x, y);
   CT::conditional_swap(relative_size < 0, x_size, y_size);

   /*
   * We know at this point that x >= y so if y_size is larger than
   * x_size, we are guaranteed they are just leading zeros which can
   * be ignored
   */
   y_size = std::min(x_size, y_size);

   bigint_sub3(z, x, x_size, y, y_size);

   return relative_size;
   }

/**
* Set t to t-s modulo mod
*
* @param t first integer
* @param s second integer
* @param mod the modulus
* @param mod_sw size of t, s, and mod
* @param ws workspace of size mod_sw
*/
inline void
bigint_mod_sub(word t[], const word s[], const word mod[], size_t mod_sw, word ws[])
   {
   // is t < s or not?
   const auto is_lt = bigint_ct_is_lt(t, mod_sw, s, mod_sw);

   // ws = p - s
   const word borrow = bigint_sub3(ws, mod, mod_sw, s, mod_sw);

   // Compute either (t - s) or (t + (p - s)) depending on mask
   const word carry = bigint_cnd_addsub(is_lt, t, ws, s, mod_sw);

   BOTAN_DEBUG_ASSERT(borrow == 0 && carry == 0);
   BOTAN_UNUSED(carry, borrow);
   }

template<size_t N>
inline void bigint_mod_sub_n(word t[], const word s[], const word mod[], word ws[])
   {
   // is t < s or not?
   const auto is_lt = bigint_ct_is_lt(t, N, s, N);

   // ws = p - s
   const word borrow = bigint_sub3(ws, mod, N, s, N);

   // Compute either (t - s) or (t + (p - s)) depending on mask
   const word carry = bigint_cnd_addsub(is_lt, t, ws, s, N);

   BOTAN_DEBUG_ASSERT(borrow == 0 && carry == 0);
   BOTAN_UNUSED(carry, borrow);
   }

/**
* Compute ((n1<<bits) + n0) / d
*/
inline word bigint_divop(word n1, word n0, word d)
   {
   if(d == 0)
      throw Invalid_Argument("bigint_divop divide by zero");

#if defined(BOTAN_HAS_MP_DWORD)
   return ((static_cast<dword>(n1) << BOTAN_MP_WORD_BITS) | n0) / d;
#else

   word high = n1 % d;
   word quotient = 0;

   for(size_t i = 0; i != BOTAN_MP_WORD_BITS; ++i)
      {
      word high_top_bit = (high & MP_WORD_TOP_BIT);

      high <<= 1;
      high |= (n0 >> (BOTAN_MP_WORD_BITS-1-i)) & 1;
      quotient <<= 1;

      if(high_top_bit || high >= d)
         {
         high -= d;
         quotient |= 1;
         }
      }

   return quotient;
#endif
   }

/**
* Compute ((n1<<bits) + n0) % d
*/
inline word bigint_modop(word n1, word n0, word d)
   {
   if(d == 0)
      throw Invalid_Argument("bigint_modop divide by zero");

#if defined(BOTAN_HAS_MP_DWORD)
   return ((static_cast<dword>(n1) << BOTAN_MP_WORD_BITS) | n0) % d;
#else
   word z = bigint_divop(n1, n0, d);
   word dummy = 0;
   z = word_madd2(z, d, &dummy);
   return (n0-z);
#endif
   }

/*
* Comba Multiplication / Squaring
*/
void bigint_comba_mul4(word z[8], const word x[4], const word y[4]);
void bigint_comba_mul6(word z[12], const word x[6], const word y[6]);
void bigint_comba_mul8(word z[16], const word x[8], const word y[8]);
void bigint_comba_mul9(word z[18], const word x[9], const word y[9]);
void bigint_comba_mul16(word z[32], const word x[16], const word y[16]);
void bigint_comba_mul24(word z[48], const word x[24], const word y[24]);

void bigint_comba_sqr4(word out[8], const word in[4]);
void bigint_comba_sqr6(word out[12], const word in[6]);
void bigint_comba_sqr8(word out[16], const word in[8]);
void bigint_comba_sqr9(word out[18], const word in[9]);
void bigint_comba_sqr16(word out[32], const word in[16]);
void bigint_comba_sqr24(word out[48], const word in[24]);

/**
* Montgomery Reduction
* @param z integer to reduce, of size exactly 2*(p_size+1).
           Output is in the first p_size+1 words, higher
           words are set to zero.
* @param p modulus
* @param p_size size of p
* @param p_dash Montgomery value
* @param workspace array of at least 2*(p_size+1) words
* @param ws_size size of workspace in words
*/
void bigint_monty_redc(word z[],
                       const word p[], size_t p_size,
                       word p_dash,
                       word workspace[],
                       size_t ws_size);

/*
* High Level Multiplication/Squaring Interfaces
*/

void bigint_mul(word z[], size_t z_size,
                const word x[], size_t x_size, size_t x_sw,
                const word y[], size_t y_size, size_t y_sw,
                word workspace[], size_t ws_size);

void bigint_sqr(word z[], size_t z_size,
                const word x[], size_t x_size, size_t x_sw,
                word workspace[], size_t ws_size);

}

namespace Botan {

/*
* Each of these functions makes the following assumptions:
*
* z_size >= 2*(p_size + 1)
* ws_size >= z_size
*/

void bigint_monty_redc_4(word z[], const word p[], word p_dash, word ws[]);
void bigint_monty_redc_6(word z[], const word p[], word p_dash, word ws[]);
void bigint_monty_redc_8(word z[], const word p[], word p_dash, word ws[]);
void bigint_monty_redc_16(word z[], const word p[], word p_dash, word ws[]);
void bigint_monty_redc_24(word z[], const word p[], word p_dash, word ws[]);
void bigint_monty_redc_32(word z[], const word p[], word p_dash, word ws[]);


}

namespace Botan {

namespace OS {

/*
* This header is internal (not installed) and these functions are not
* intended to be called by applications. However they are given public
* visibility (using BOTAN_TEST_API macro) for the tests. This also probably
* allows them to be overridden by the application on ELF systems, but
* this hasn't been tested.
*/

/**
* @return process ID assigned by the operating system.
* On Unix and Windows systems, this always returns a result
* On IncludeOS it returns 0 since there is no process ID to speak of
* in a unikernel.
*/
uint32_t BOTAN_TEST_API get_process_id();

/**
* Test if we are currently running with elevated permissions
* eg setuid, setgid, or with POSIX caps set.
*/
bool running_in_privileged_state();

/**
* @return CPU processor clock, if available
*
* On Windows, calls QueryPerformanceCounter.
*
* Under GCC or Clang on supported platforms the hardware cycle counter is queried.
* Currently supported processors are x86, PPC, Alpha, SPARC, IA-64, S/390x, and HP-PA.
* If no CPU cycle counter is available on this system, returns zero.
*/
uint64_t BOTAN_TEST_API get_cpu_cycle_counter();

/*
* @return best resolution timestamp available
*
* The epoch and update rate of this clock is arbitrary and depending
* on the hardware it may not tick at a constant rate.
*
* Uses hardware cycle counter, if available.
* On POSIX platforms clock_gettime is used with a monotonic timer
* As a final fallback std::chrono::high_resolution_clock is used.
*/
uint64_t BOTAN_TEST_API get_high_resolution_clock();

/**
* @return system clock (reflecting wall clock) with best resolution
* available, normalized to nanoseconds resolution.
*/
uint64_t BOTAN_TEST_API get_system_timestamp_ns();

/**
* @return maximum amount of memory (in bytes) Botan could/should
* hyptothetically allocate for the memory poool. Reads environment
* variable "BOTAN_MLOCK_POOL_SIZE", set to "0" to disable pool.
*/
size_t get_memory_locking_limit();

/**
* Return the size of a memory page, if that can be derived on the
* current system. Otherwise returns some default value (eg 4096)
*/
size_t system_page_size();

/**
* Read the value of an environment variable. Return nullptr if
* no such variable is set. If the process seems to be running in
* a privileged state (such as setuid) then always returns nullptr,
* similiar to glibc's secure_getenv.
*/
const char* read_env_variable(const std::string& var_name);

/**
* Request @count pages of RAM which are locked into memory using mlock,
* VirtualLock, or some similar OS specific API. Free it with free_locked_pages.
*
* Returns an empty list on failure. This function is allowed to return fewer
* than @count pages.
*
* The contents of the allocated pages are undefined.
*
* Each page is preceded by and followed by a page which is marked
* as noaccess, such that accessing it will cause a crash. This turns
* out of bound reads/writes into crash events.
*
* @param count requested number of locked pages
*/
std::vector<void*> allocate_locked_pages(size_t count);

/**
* Free memory allocated by allocate_locked_pages
* @param pages a list of pages returned by allocate_locked_pages
*/
void free_locked_pages(const std::vector<void*>& pages);

/**
* Set the MMU to prohibit access to this page
*/
void page_prohibit_access(void* page);

/**
* Set the MMU to allow R/W access to this page
*/
void page_allow_access(void* page);


/**
* Run a probe instruction to test for support for a CPU instruction.
* Runs in system-specific env that catches illegal instructions; this
* function always fails if the OS doesn't provide this.
* Returns value of probe_fn, if it could run.
* If error occurs, returns negative number.
* This allows probe_fn to indicate errors of its own, if it wants.
* For example the instruction might not only be only available on some
* CPUs, but also buggy on some subset of these - the probe function
* can test to make sure the instruction works properly before
* indicating that the instruction is available.
*
* @warning on Unix systems uses signal handling in a way that is not
* thread safe. It should only be called in a single-threaded context
* (ie, at static init time).
*
* If probe_fn throws an exception the result is undefined.
*
* Return codes:
* -1 illegal instruction detected
*/
int BOTAN_TEST_API run_cpu_instruction_probe(std::function<int ()> probe_fn);

/**
* Represents a terminal state
*/
class BOTAN_UNSTABLE_API Echo_Suppression
   {
   public:
      /**
      * Reenable echo on this terminal. Can be safely called
      * multiple times. May throw if an error occurs.
      */
      virtual void reenable_echo() = 0;

      /**
      * Implicitly calls reenable_echo, but swallows/ignored all
      * errors which would leave the terminal in an invalid state.
      */
      virtual ~Echo_Suppression() = default;
   };

/**
* Suppress echo on the terminal
* Returns null if this operation is not supported on the current system.
*/
std::unique_ptr<Echo_Suppression> BOTAN_UNSTABLE_API suppress_echo_on_terminal();

}

}

namespace Botan {

/**
* Container of output buffers for Pipe
*/
class Output_Buffers final
   {
   public:
      size_t read(uint8_t[], size_t, Pipe::message_id);
      size_t peek(uint8_t[], size_t, size_t, Pipe::message_id) const;
      size_t get_bytes_read(Pipe::message_id) const;
      size_t remaining(Pipe::message_id) const;

      void add(class SecureQueue*);
      void retire();

      Pipe::message_id message_count() const;

      Output_Buffers();
   private:
      class SecureQueue* get(Pipe::message_id) const;

      std::deque<std::unique_ptr<SecureQueue>> m_buffers;
      Pipe::message_id m_offset;
   };

}

namespace Botan {

/**
* Returns the allowed padding schemes when using the given
* algorithm (key type) for creating digital signatures.
*
* @param algo the algorithm for which to look up supported padding schemes
* @return a vector of supported padding schemes
*/
BOTAN_TEST_API const std::vector<std::string> get_sig_paddings(const std::string algo);

/**
* Returns true iff the given padding scheme is valid for the given
* signature algorithm (key type).
*
* @param algo the signature algorithm to be used
* @param padding the padding scheme to be used
*/
bool sig_algo_and_pad_ok(const std::string algo, const std::string padding);

}

namespace Botan {

namespace PK_Ops {

class Encryption_with_EME : public Encryption
   {
   public:
      size_t max_input_bits() const override;

      secure_vector<uint8_t> encrypt(const uint8_t msg[], size_t msg_len,
                                  RandomNumberGenerator& rng) override;

      ~Encryption_with_EME() = default;
   protected:
      explicit Encryption_with_EME(const std::string& eme);
   private:
      virtual size_t max_raw_input_bits() const = 0;

      virtual secure_vector<uint8_t> raw_encrypt(const uint8_t msg[], size_t len,
                                              RandomNumberGenerator& rng) = 0;
      std::unique_ptr<EME> m_eme;
   };

class Decryption_with_EME : public Decryption
   {
   public:
      secure_vector<uint8_t> decrypt(uint8_t& valid_mask,
                                  const uint8_t msg[], size_t msg_len) override;

      ~Decryption_with_EME() = default;
   protected:
      explicit Decryption_with_EME(const std::string& eme);
   private:
      virtual secure_vector<uint8_t> raw_decrypt(const uint8_t msg[], size_t len) = 0;
      std::unique_ptr<EME> m_eme;
   };

class Verification_with_EMSA : public Verification
   {
   public:
      ~Verification_with_EMSA() = default;

      void update(const uint8_t msg[], size_t msg_len) override;
      bool is_valid_signature(const uint8_t sig[], size_t sig_len) override;

      bool do_check(const secure_vector<uint8_t>& msg,
                    const uint8_t sig[], size_t sig_len);

      std::string hash_for_signature() { return m_hash; }

   protected:
      explicit Verification_with_EMSA(const std::string& emsa);

      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message in bits
      */
      virtual size_t max_input_bits() const = 0;

      /**
      * @return boolean specifying if this signature scheme uses
      * a message prefix returned by message_prefix()
      */
      virtual bool has_prefix() { return false; }

      /**
      * @return the message prefix if this signature scheme uses
      * a message prefix, signaled via has_prefix()
      */
      virtual secure_vector<uint8_t> message_prefix() const { throw Invalid_State("No prefix"); }

      /**
      * @return boolean specifying if this key type supports message
      * recovery and thus if you need to call verify() or verify_mr()
      */
      virtual bool with_recovery() const = 0;

      /*
      * Perform a signature check operation
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @param sig the signature
      * @param sig_len the length of sig in bytes
      * @returns if signature is a valid one for message
      */
      virtual bool verify(const uint8_t[], size_t,
                          const uint8_t[], size_t)
         {
         throw Invalid_State("Message recovery required");
         }

      /*
      * Perform a signature operation (with message recovery)
      * Only call this if with_recovery() returns true
      * @param msg the message
      * @param msg_len the length of msg in bytes
      * @returns recovered message
      */
      virtual secure_vector<uint8_t> verify_mr(const uint8_t[], size_t)
         {
         throw Invalid_State("Message recovery not supported");
         }

      std::unique_ptr<EMSA> clone_emsa() const { return std::unique_ptr<EMSA>(m_emsa->clone()); }

   private:
      std::unique_ptr<EMSA> m_emsa;
      const std::string m_hash;
      bool m_prefix_used;
   };

class Signature_with_EMSA : public Signature
   {
   public:
      void update(const uint8_t msg[], size_t msg_len) override;

      secure_vector<uint8_t> sign(RandomNumberGenerator& rng) override;
   protected:
      explicit Signature_with_EMSA(const std::string& emsa);
      ~Signature_with_EMSA() = default;

      std::string hash_for_signature() { return m_hash; }

      /**
      * @return boolean specifying if this signature scheme uses
      * a message prefix returned by message_prefix()
      */
      virtual bool has_prefix() { return false; }

      /**
      * @return the message prefix if this signature scheme uses
      * a message prefix, signaled via has_prefix()
      */
      virtual secure_vector<uint8_t> message_prefix() const { throw Invalid_State("No prefix"); }

      std::unique_ptr<EMSA> clone_emsa() const { return std::unique_ptr<EMSA>(m_emsa->clone()); }

   private:

      /**
      * Get the maximum message size in bits supported by this public key.
      * @return maximum message in bits
      */
      virtual size_t max_input_bits() const = 0;

      bool self_test_signature(const std::vector<uint8_t>& msg,
                               const std::vector<uint8_t>& sig) const;

      virtual secure_vector<uint8_t> raw_sign(const uint8_t msg[], size_t msg_len,
                                           RandomNumberGenerator& rng) = 0;

      std::unique_ptr<EMSA> m_emsa;
      const std::string m_hash;
      bool m_prefix_used;
   };

class Key_Agreement_with_KDF : public Key_Agreement
   {
   public:
      secure_vector<uint8_t> agree(size_t key_len,
                                const uint8_t other_key[], size_t other_key_len,
                                const uint8_t salt[], size_t salt_len) override;

   protected:
      explicit Key_Agreement_with_KDF(const std::string& kdf);
      ~Key_Agreement_with_KDF() = default;
   private:
      virtual secure_vector<uint8_t> raw_agree(const uint8_t w[], size_t w_len) = 0;
      std::unique_ptr<KDF> m_kdf;
   };

class KEM_Encryption_with_KDF : public KEM_Encryption
   {
   public:
      void kem_encrypt(secure_vector<uint8_t>& out_encapsulated_key,
                       secure_vector<uint8_t>& out_shared_key,
                       size_t desired_shared_key_len,
                       Botan::RandomNumberGenerator& rng,
                       const uint8_t salt[],
                       size_t salt_len) override;

   protected:
      virtual void raw_kem_encrypt(secure_vector<uint8_t>& out_encapsulated_key,
                                   secure_vector<uint8_t>& raw_shared_key,
                                   Botan::RandomNumberGenerator& rng) = 0;

      explicit KEM_Encryption_with_KDF(const std::string& kdf);
      ~KEM_Encryption_with_KDF() = default;
   private:
      std::unique_ptr<KDF> m_kdf;
   };

class KEM_Decryption_with_KDF : public KEM_Decryption
   {
   public:
      secure_vector<uint8_t> kem_decrypt(const uint8_t encap_key[],
                                      size_t len,
                                      size_t desired_shared_key_len,
                                      const uint8_t salt[],
                                      size_t salt_len) override;

   protected:
      virtual secure_vector<uint8_t>
      raw_kem_decrypt(const uint8_t encap_key[], size_t len) = 0;

      explicit KEM_Decryption_with_KDF(const std::string& kdf);
      ~KEM_Decryption_with_KDF() = default;
   private:
      std::unique_ptr<KDF> m_kdf;
   };

}

}

namespace Botan {

class Modular_Reducer;

static const size_t PointGFp_SCALAR_BLINDING_BITS = 80;

class PointGFp_Base_Point_Precompute final
   {
   public:
      PointGFp_Base_Point_Precompute(const PointGFp& base_point,
                                     const Modular_Reducer& mod_order);

      PointGFp mul(const BigInt& k,
                   RandomNumberGenerator& rng,
                   const BigInt& group_order,
                   std::vector<BigInt>& ws) const;
   private:
      const PointGFp& m_base_point;
      const Modular_Reducer& m_mod_order;

      enum { WINDOW_BITS = 3 };
      enum { WINDOW_SIZE = (1 << WINDOW_BITS) - 1 };

      const size_t m_p_words;
      const size_t m_T_size;

      /*
      * This is a table of T_size * 3*p_word words
      */
      std::vector<word> m_W;
   };

class PointGFp_Var_Point_Precompute final
   {
   public:
      PointGFp_Var_Point_Precompute(const PointGFp& point,
                                    RandomNumberGenerator& rng,
                                    std::vector<BigInt>& ws);

      PointGFp mul(const BigInt& k,
                   RandomNumberGenerator& rng,
                   const BigInt& group_order,
                   std::vector<BigInt>& ws) const;
   private:
      const CurveGFp m_curve;
      const size_t m_p_words;
      const size_t m_window_bits;

      /*
      * Table of 2^window_bits * 3*2*p_word words
      * Kept in locked vector since the base point might be sensitive
      * (normally isn't in most protocols but hard to say anything
      * categorically.)
      */
      secure_vector<word> m_T;
   };

class PointGFp_Multi_Point_Precompute final
   {
   public:
      PointGFp_Multi_Point_Precompute(const PointGFp& g1,
                                      const PointGFp& g2);

      /*
      * Return (g1*k1 + g2*k2)
      * Not constant time, intended to use with public inputs
      */
      PointGFp multi_exp(const BigInt& k1,
                         const BigInt& k2) const;
   private:
      std::vector<PointGFp> m_M;
   };

}

namespace Botan {

/**
* Polynomial doubling in GF(2^n)
*/
void BOTAN_PUBLIC_API(2,3) poly_double_n(uint8_t out[], const uint8_t in[], size_t n);

/**
* Returns true iff poly_double_n is implemented for this size.
*/
inline bool poly_double_supported_size(size_t n)
   {
   return (n == 8 || n == 16 || n == 24 || n == 32 || n == 64 || n == 128);
   }

inline void poly_double_n(uint8_t buf[], size_t n)
   {
   return poly_double_n(buf, buf, n);
   }

/*
* Little endian convention - used for XTS
*/
void poly_double_n_le(uint8_t out[], const uint8_t in[], size_t n);

}

namespace Botan {

template<typename T>
inline void prefetch_readonly(const T* addr, size_t length)
   {
#if defined(__GNUG__)
   const size_t Ts_per_cache_line = CPUID::cache_line_size() / sizeof(T);

   for(size_t i = 0; i <= length; i += Ts_per_cache_line)
      __builtin_prefetch(addr + i, 0);
#endif
   }

template<typename T>
inline void prefetch_readwrite(const T* addr, size_t length)
   {
#if defined(__GNUG__)
   const size_t Ts_per_cache_line = CPUID::cache_line_size() / sizeof(T);

   for(size_t i = 0; i <= length; i += Ts_per_cache_line)
      __builtin_prefetch(addr + i, 1);
#endif
   }

}

namespace Botan {

class BigInt;
class Modular_Reducer;
class Montgomery_Params;
class RandomNumberGenerator;

/**
* Perform Lucas primality test
* @see FIPS 186-4 C.3.3
*
* @warning it is possible to construct composite integers which pass
* this test alone.
*
* @param n the positive integer to test
* @param mod_n a pre-created Modular_Reducer for n
* @return true if n seems probably prime, false if n is composite
*/
bool BOTAN_TEST_API is_lucas_probable_prime(const BigInt& n, const Modular_Reducer& mod_n);

/**
* Perform Bailie-PSW primality test
*
* This is a combination of Miller-Rabin with base 2 and a Lucas test. No known
* composite integer passes both tests, though it is conjectured that infinitely
* many composite counterexamples exist.
*
* @param n the positive integer to test
* @param mod_n a pre-created Modular_Reducer for n
* @return true if n seems probably prime, false if n is composite
*/
bool BOTAN_TEST_API is_bailie_psw_probable_prime(const BigInt& n, const Modular_Reducer& mod_n);

/**
* Perform Bailie-PSW primality test
*
* This is a combination of Miller-Rabin with base 2 and a Lucas test. No known
* composite integer passes both tests, though it is conjectured that infinitely
* many composite counterexamples exist.
*
* @param n the positive integer to test
* @return true if n seems probably prime, false if n is composite
*/
bool is_bailie_psw_probable_prime(const BigInt& n);

/**
* Return required number of Miller-Rabin tests in order to
* reach the specified probability of error.
*
* @param n_bits the bit-length of the integer being tested
* @param prob chance of false positive is bounded by 1/2**prob
* @param random is set if (and only if) the integer was randomly generated by us
*        and thus cannot have been maliciously constructed.
*/
size_t miller_rabin_test_iterations(size_t n_bits, size_t prob, bool random);

/**
* Perform a single Miller-Rabin test with specified base
*
* @param n the positive integer to test
* @param mod_n a pre-created Modular_Reducer for n
* @param monty_n Montgomery parameters for n
* @param a the base to check
* @return result of primality test
*/
bool passes_miller_rabin_test(const BigInt& n,
                              const Modular_Reducer& mod_n,
                              const std::shared_ptr<Montgomery_Params>& monty_n,
                              const BigInt& a);

/**
* Perform t iterations of a Miller-Rabin primality test with random bases
*
* @param n the positive integer to test
* @param mod_n a pre-created Modular_Reducer for n
* @param rng a random number generator
* @param t number of tests to perform
*
* @return result of primality test
*/
bool BOTAN_TEST_API is_miller_rabin_probable_prime(const BigInt& n,
                                                   const Modular_Reducer& mod_n,
                                                   RandomNumberGenerator& rng,
                                                   size_t t);

}

namespace Botan {

class File_Descriptor_Source
   {
   public:
      virtual int next_fd() = 0;
      virtual ~File_Descriptor_Source() = default;
   };

/**
* File Tree Walking Entropy Source
*/
class ProcWalking_EntropySource final : public Entropy_Source
   {
   public:
      std::string name() const override { return "proc_walk"; }

      size_t poll(RandomNumberGenerator& rng) override;

      explicit ProcWalking_EntropySource(const std::string& root_dir) :
         m_path(root_dir), m_dir(nullptr) {}

   private:
      const std::string m_path;
      mutex_type m_mutex;
      std::unique_ptr<File_Descriptor_Source> m_dir;
      secure_vector<uint8_t> m_buf;
   };

}

namespace Botan {

/**
* Entropy source using the rdrand instruction first introduced on
* Intel's Ivy Bridge architecture.
*/
class Intel_Rdrand final : public Entropy_Source
   {
   public:
      std::string name() const override { return "rdrand"; }
      size_t poll(RandomNumberGenerator& rng) override;
   };

}

namespace Botan {

/**
* Entropy source using the rdseed instruction first introduced on
* Intel's Broadwell architecture.
*/
class Intel_Rdseed final : public Entropy_Source
   {
   public:
      std::string name() const override { return "rdseed"; }
      size_t poll(RandomNumberGenerator& rng) override;
   };

}

namespace Botan {

/**
* Round up
* @param n a non-negative integer
* @param align_to the alignment boundary
* @return n rounded up to a multiple of align_to
*/
inline size_t round_up(size_t n, size_t align_to)
   {
   BOTAN_ARG_CHECK(align_to != 0, "align_to must not be 0");

   if(n % align_to)
      n += align_to - (n % align_to);
   return n;
   }

/**
* Round down
* @param n an integer
* @param align_to the alignment boundary
* @return n rounded down to a multiple of align_to
*/
template<typename T>
inline constexpr T round_down(T n, T align_to)
   {
   return (align_to == 0) ? n : (n - (n % align_to));
   }

/**
* Clamp
*/
inline size_t clamp(size_t n, size_t lower_bound, size_t upper_bound)
   {
   if(n < lower_bound)
      return lower_bound;
   if(n > upper_bound)
      return upper_bound;
   return n;
   }

}

namespace Botan {

class BOTAN_PUBLIC_API(2,0) Integer_Overflow_Detected final : public Exception
   {
   public:
      Integer_Overflow_Detected(const std::string& file, int line) :
         Exception("Integer overflow detected at " + file + ":" + std::to_string(line))
         {}

      ErrorType error_type() const noexcept override { return ErrorType::InternalError; }
   };

inline size_t checked_add(size_t x, size_t y, const char* file, int line)
   {
   // TODO: use __builtin_x_overflow on GCC and Clang
   size_t z = x + y;
   if(z < x)
      {
      throw Integer_Overflow_Detected(file, line);
      }
   return z;
   }

#define BOTAN_CHECKED_ADD(x,y) checked_add(x,y,__FILE__,__LINE__)

}

namespace Botan {

class Semaphore final
   {
   public:
      explicit Semaphore(int value = 0) : m_value(value), m_wakeups(0) {}

      void acquire();

      void release(size_t n = 1);

   private:
      int m_value;
      int m_wakeups;
      mutex_type m_mutex;
      std::condition_variable m_cond;
   };

}
#define SBoxE1(B0, B1, B2, B3)                    \
   do {                                           \
      B3 ^= B0;                                   \
      auto B4 = B1;                               \
      B1 &= B3;                                   \
      B4 ^= B2;                                   \
      B1 ^= B0;                                   \
      B0 |= B3;                                   \
      B0 ^= B4;                                   \
      B4 ^= B3;                                   \
      B3 ^= B2;                                   \
      B2 |= B1;                                   \
      B2 ^= B4;                                   \
      B4 = ~B4;                                   \
      B4 |= B1;                                   \
      B1 ^= B3;                                   \
      B1 ^= B4;                                   \
      B3 |= B0;                                   \
      B1 ^= B3;                                   \
      B4 ^= B3;                                   \
      B3 = B0;                                    \
      B0 = B1;                                    \
      B1 = B4;                                    \
   } while(0)

#define SBoxE2(B0, B1, B2, B3)                    \
   do {                                           \
      B0 = ~B0;                                   \
      B2 = ~B2;                                   \
      auto B4 = B0;                               \
      B0 &= B1;                                   \
      B2 ^= B0;                                   \
      B0 |= B3;                                   \
      B3 ^= B2;                                   \
      B1 ^= B0;                                   \
      B0 ^= B4;                                   \
      B4 |= B1;                                   \
      B1 ^= B3;                                   \
      B2 |= B0;                                   \
      B2 &= B4;                                   \
      B0 ^= B1;                                   \
      B1 &= B2;                                   \
      B1 ^= B0;                                   \
      B0 &= B2;                                   \
      B4 ^= B0;                                   \
      B0 = B2;                                    \
      B2 = B3;                                    \
      B3 = B1;                                    \
      B1 = B4;                                    \
   } while(0)

#define SBoxE3(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B0;                               \
      B0 &= B2;                                   \
      B0 ^= B3;                                   \
      B2 ^= B1;                                   \
      B2 ^= B0;                                   \
      B3 |= B4;                                   \
      B3 ^= B1;                                   \
      B4 ^= B2;                                   \
      B1 = B3;                                    \
      B3 |= B4;                                   \
      B3 ^= B0;                                   \
      B0 &= B1;                                   \
      B4 ^= B0;                                   \
      B1 ^= B3;                                   \
      B1 ^= B4;                                   \
      B0 = B2;                                    \
      B2 = B1;                                    \
      B1 = B3;                                    \
      B3 = ~B4;                                   \
   } while(0)

#define SBoxE4(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B0;                               \
      B0 |= B3;                                   \
      B3 ^= B1;                                   \
      B1 &= B4;                                   \
      B4 ^= B2;                                   \
      B2 ^= B3;                                   \
      B3 &= B0;                                   \
      B4 |= B1;                                   \
      B3 ^= B4;                                   \
      B0 ^= B1;                                   \
      B4 &= B0;                                   \
      B1 ^= B3;                                   \
      B4 ^= B2;                                   \
      B1 |= B0;                                   \
      B1 ^= B2;                                   \
      B0 ^= B3;                                   \
      B2 = B1;                                    \
      B1 |= B3;                                   \
      B0 ^= B1;                                   \
      B1 = B2;                                    \
      B2 = B3;                                    \
      B3 = B4;                                    \
   } while(0)

#define SBoxE5(B0, B1, B2, B3)                    \
   do {                                           \
      B1 ^= B3;                                   \
      B3 = ~B3;                                   \
      B2 ^= B3;                                   \
      B3 ^= B0;                                   \
      auto B4 = B1;                               \
      B1 &= B3;                                   \
      B1 ^= B2;                                   \
      B4 ^= B3;                                   \
      B0 ^= B4;                                   \
      B2 &= B4;                                   \
      B2 ^= B0;                                   \
      B0 &= B1;                                   \
      B3 ^= B0;                                   \
      B4 |= B1;                                   \
      B4 ^= B0;                                   \
      B0 |= B3;                                   \
      B0 ^= B2;                                   \
      B2 &= B3;                                   \
      B0 = ~B0;                                   \
      B4 ^= B2;                                   \
      B2 = B0;                                    \
      B0 = B1;                                    \
      B1 = B4;                                    \
   } while(0)

#define SBoxE6(B0, B1, B2, B3)                    \
   do {                                           \
      B0 ^= B1;                                   \
      B1 ^= B3;                                   \
      B3 = ~B3;                                   \
      auto B4 = B1;                               \
      B1 &= B0;                                   \
      B2 ^= B3;                                   \
      B1 ^= B2;                                   \
      B2 |= B4;                                   \
      B4 ^= B3;                                   \
      B3 &= B1;                                   \
      B3 ^= B0;                                   \
      B4 ^= B1;                                   \
      B4 ^= B2;                                   \
      B2 ^= B0;                                   \
      B0 &= B3;                                   \
      B2 = ~B2;                                   \
      B0 ^= B4;                                   \
      B4 |= B3;                                   \
      B4 ^= B2;                                   \
      B2 = B0;                                    \
      B0 = B1;                                    \
      B1 = B3;                                    \
      B3 = B4;                                    \
   } while(0)

#define SBoxE7(B0, B1, B2, B3)                    \
   do {                                           \
      B2 = ~B2;                                   \
      auto B4 = B3;                               \
      B3 &= B0;                                   \
      B0 ^= B4;                                   \
      B3 ^= B2;                                   \
      B2 |= B4;                                   \
      B1 ^= B3;                                   \
      B2 ^= B0;                                   \
      B0 |= B1;                                   \
      B2 ^= B1;                                   \
      B4 ^= B0;                                   \
      B0 |= B3;                                   \
      B0 ^= B2;                                   \
      B4 ^= B3;                                   \
      B4 ^= B0;                                   \
      B3 = ~B3;                                   \
      B2 &= B4;                                   \
      B3 ^= B2;                                   \
      B2 = B4;                                    \
   } while(0)

#define SBoxE8(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B1;                               \
      B1 |= B2;                                   \
      B1 ^= B3;                                   \
      B4 ^= B2;                                   \
      B2 ^= B1;                                   \
      B3 |= B4;                                   \
      B3 &= B0;                                   \
      B4 ^= B2;                                   \
      B3 ^= B1;                                   \
      B1 |= B4;                                   \
      B1 ^= B0;                                   \
      B0 |= B4;                                   \
      B0 ^= B2;                                   \
      B1 ^= B4;                                   \
      B2 ^= B1;                                   \
      B1 &= B0;                                   \
      B1 ^= B4;                                   \
      B2 = ~B2;                                   \
      B2 |= B0;                                   \
      B4 ^= B2;                                   \
      B2 = B1;                                    \
      B1 = B3;                                    \
      B3 = B0;                                    \
      B0 = B4;                                    \
   } while(0)

#define SBoxD1(B0, B1, B2, B3)                    \
   do {                                           \
      B2 = ~B2;                                   \
      auto B4 = B1;                               \
      B1 |= B0;                                   \
      B4 = ~B4;                                   \
      B1 ^= B2;                                   \
      B2 |= B4;                                   \
      B1 ^= B3;                                   \
      B0 ^= B4;                                   \
      B2 ^= B0;                                   \
      B0 &= B3;                                   \
      B4 ^= B0;                                   \
      B0 |= B1;                                   \
      B0 ^= B2;                                   \
      B3 ^= B4;                                   \
      B2 ^= B1;                                   \
      B3 ^= B0;                                   \
      B3 ^= B1;                                   \
      B2 &= B3;                                   \
      B4 ^= B2;                                   \
      B2 = B1;                                    \
      B1 = B4;                                    \
      } while(0)

#define SBoxD2(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B1;                               \
      B1 ^= B3;                                   \
      B3 &= B1;                                   \
      B4 ^= B2;                                   \
      B3 ^= B0;                                   \
      B0 |= B1;                                   \
      B2 ^= B3;                                   \
      B0 ^= B4;                                   \
      B0 |= B2;                                   \
      B1 ^= B3;                                   \
      B0 ^= B1;                                   \
      B1 |= B3;                                   \
      B1 ^= B0;                                   \
      B4 = ~B4;                                   \
      B4 ^= B1;                                   \
      B1 |= B0;                                   \
      B1 ^= B0;                                   \
      B1 |= B4;                                   \
      B3 ^= B1;                                   \
      B1 = B0;                                    \
      B0 = B4;                                    \
      B4 = B2;                                    \
      B2 = B3;                                    \
      B3 = B4;                                    \
      } while(0)

#define SBoxD3(B0, B1, B2, B3)                    \
   do {                                           \
      B2 ^= B3;                                   \
      B3 ^= B0;                                   \
      auto B4 = B3;                               \
      B3 &= B2;                                   \
      B3 ^= B1;                                   \
      B1 |= B2;                                   \
      B1 ^= B4;                                   \
      B4 &= B3;                                   \
      B2 ^= B3;                                   \
      B4 &= B0;                                   \
      B4 ^= B2;                                   \
      B2 &= B1;                                   \
      B2 |= B0;                                   \
      B3 = ~B3;                                   \
      B2 ^= B3;                                   \
      B0 ^= B3;                                   \
      B0 &= B1;                                   \
      B3 ^= B4;                                   \
      B3 ^= B0;                                   \
      B0 = B1;                                    \
      B1 = B4;                                    \
      } while(0)

#define SBoxD4(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B2;                               \
      B2 ^= B1;                                   \
      B0 ^= B2;                                   \
      B4 &= B2;                                   \
      B4 ^= B0;                                   \
      B0 &= B1;                                   \
      B1 ^= B3;                                   \
      B3 |= B4;                                   \
      B2 ^= B3;                                   \
      B0 ^= B3;                                   \
      B1 ^= B4;                                   \
      B3 &= B2;                                   \
      B3 ^= B1;                                   \
      B1 ^= B0;                                   \
      B1 |= B2;                                   \
      B0 ^= B3;                                   \
      B1 ^= B4;                                   \
      B0 ^= B1;                                   \
      B4 = B0;                                    \
      B0 = B2;                                    \
      B2 = B3;                                    \
      B3 = B4;                                    \
      } while(0)

#define SBoxD5(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B2;                               \
      B2 &= B3;                                   \
      B2 ^= B1;                                   \
      B1 |= B3;                                   \
      B1 &= B0;                                   \
      B4 ^= B2;                                   \
      B4 ^= B1;                                   \
      B1 &= B2;                                   \
      B0 = ~B0;                                   \
      B3 ^= B4;                                   \
      B1 ^= B3;                                   \
      B3 &= B0;                                   \
      B3 ^= B2;                                   \
      B0 ^= B1;                                   \
      B2 &= B0;                                   \
      B3 ^= B0;                                   \
      B2 ^= B4;                                   \
      B2 |= B3;                                   \
      B3 ^= B0;                                   \
      B2 ^= B1;                                   \
      B1 = B3;                                    \
      B3 = B4;                                    \
      } while(0)

#define SBoxD6(B0, B1, B2, B3)                    \
   do {                                           \
      B1 = ~B1;                                   \
      auto B4 = B3;                               \
      B2 ^= B1;                                   \
      B3 |= B0;                                   \
      B3 ^= B2;                                   \
      B2 |= B1;                                   \
      B2 &= B0;                                   \
      B4 ^= B3;                                   \
      B2 ^= B4;                                   \
      B4 |= B0;                                   \
      B4 ^= B1;                                   \
      B1 &= B2;                                   \
      B1 ^= B3;                                   \
      B4 ^= B2;                                   \
      B3 &= B4;                                   \
      B4 ^= B1;                                   \
      B3 ^= B4;                                   \
      B4 = ~B4;                                   \
      B3 ^= B0;                                   \
      B0 = B1;                                    \
      B1 = B4;                                    \
      B4 = B3;                                    \
      B3 = B2;                                    \
      B2 = B4;                                    \
      } while(0)

#define SBoxD7(B0, B1, B2, B3)                    \
   do {                                           \
      B0 ^= B2;                                   \
      auto B4 = B2;                               \
      B2 &= B0;                                   \
      B4 ^= B3;                                   \
      B2 = ~B2;                                   \
      B3 ^= B1;                                   \
      B2 ^= B3;                                   \
      B4 |= B0;                                   \
      B0 ^= B2;                                   \
      B3 ^= B4;                                   \
      B4 ^= B1;                                   \
      B1 &= B3;                                   \
      B1 ^= B0;                                   \
      B0 ^= B3;                                   \
      B0 |= B2;                                   \
      B3 ^= B1;                                   \
      B4 ^= B0;                                   \
      B0 = B1;                                    \
      B1 = B2;                                    \
      B2 = B4;                                    \
      } while(0)

#define SBoxD8(B0, B1, B2, B3)                    \
   do {                                           \
      auto B4 = B2;                               \
      B2 ^= B0;                                   \
      B0 &= B3;                                   \
      B4 |= B3;                                   \
      B2 = ~B2;                                   \
      B3 ^= B1;                                   \
      B1 |= B0;                                   \
      B0 ^= B2;                                   \
      B2 &= B4;                                   \
      B3 &= B4;                                   \
      B1 ^= B2;                                   \
      B2 ^= B0;                                   \
      B0 |= B2;                                   \
      B4 ^= B1;                                   \
      B0 ^= B3;                                   \
      B3 ^= B4;                                   \
      B4 |= B0;                                   \
      B3 ^= B2;                                   \
      B4 ^= B2;                                   \
      B2 = B1;                                    \
      B1 = B0;                                    \
      B0 = B3;                                    \
      B3 = B4;                                    \
      } while(0)

#if defined(BOTAN_TARGET_SUPPORTS_SSE2)
  #include <emmintrin.h>
  #define BOTAN_SIMD_USE_SSE2

#elif defined(BOTAN_TARGET_SUPPORTS_ALTIVEC)
  #include <altivec.h>
  #undef vector
  #undef bool
  #define BOTAN_SIMD_USE_ALTIVEC

#elif defined(BOTAN_TARGET_SUPPORTS_NEON)
  #include <arm_neon.h>
  #define BOTAN_SIMD_USE_NEON

#else
#endif

namespace Botan {

/**
* 4x32 bit SIMD register
*
* This class is not a general purpose SIMD type, and only offers
* instructions needed for evaluation of specific crypto primitives.
* For example it does not currently have equality operators of any
* kind.
*
* Implemented for SSE2, VMX (Altivec), and NEON.
*/
class SIMD_4x32 final
   {
   public:

      SIMD_4x32& operator=(const SIMD_4x32& other) = default;
      SIMD_4x32(const SIMD_4x32& other) = default;

      SIMD_4x32& operator=(SIMD_4x32&& other) = default;
      SIMD_4x32(SIMD_4x32&& other) = default;

      /**
      * Zero initialize SIMD register with 4 32-bit elements
      */
      SIMD_4x32() // zero initialized
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_setzero_si128();
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_splat_u32(0);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vdupq_n_u32(0);
#else
         m_scalar[0] = 0;
         m_scalar[1] = 0;
         m_scalar[2] = 0;
         m_scalar[3] = 0;
#endif
         }

      /**
      * Load SIMD register with 4 32-bit elements
      */
      explicit SIMD_4x32(const uint32_t B[4])
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_loadu_si128(reinterpret_cast<const __m128i*>(B));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = (__vector unsigned int){B[0], B[1], B[2], B[3]};
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vld1q_u32(B);
#else
         m_scalar[0] = B[0];
         m_scalar[1] = B[1];
         m_scalar[2] = B[2];
         m_scalar[3] = B[3];
#endif
         }

      /**
      * Load SIMD register with 4 32-bit elements
      */
      SIMD_4x32(uint32_t B0, uint32_t B1, uint32_t B2, uint32_t B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_set_epi32(B3, B2, B1, B0);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = (__vector unsigned int){B0, B1, B2, B3};
#elif defined(BOTAN_SIMD_USE_NEON)
         // Better way to do this?
         const uint32_t B[4] = { B0, B1, B2, B3 };
         m_neon = vld1q_u32(B);
#else
         m_scalar[0] = B0;
         m_scalar[1] = B1;
         m_scalar[2] = B2;
         m_scalar[3] = B3;
#endif
         }

      /**
      * Load SIMD register with one 32-bit element repeated
      */
      static SIMD_4x32 splat(uint32_t B)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_set1_epi32(B));
#elif defined(BOTAN_SIMD_USE_ARM)
         return SIMD_4x32(vdupq_n_u32(B));
#else
         return SIMD_4x32(B, B, B, B);
#endif
         }

      /**
      * Load a SIMD register with little-endian convention
      */
      static SIMD_4x32 load_le(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(in)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         uint32_t R[4];
         Botan::load_le(R, static_cast<const uint8_t*>(in), 4);
         return SIMD_4x32(R);

#elif defined(BOTAN_SIMD_USE_NEON)

         SIMD_4x32 l(vld1q_u32(static_cast<const uint32_t*>(in)));
         return CPUID::is_big_endian() ? l.bswap() : l;
#else
         SIMD_4x32 out;
         Botan::load_le(out.m_scalar, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      /**
      * Load a SIMD register with big-endian convention
      */
      static SIMD_4x32 load_be(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         return load_le(in).bswap();

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         uint32_t R[4];
         Botan::load_be(R, static_cast<const uint8_t*>(in), 4);
         return SIMD_4x32(R);

#elif defined(BOTAN_SIMD_USE_NEON)

         SIMD_4x32 l(vld1q_u32(static_cast<const uint32_t*>(in)));
         return CPUID::is_little_endian() ? l.bswap() : l;

#else
         SIMD_4x32 out;
         Botan::load_be(out.m_scalar, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      /**
      * Load a SIMD register with little-endian convention
      */
      void store_le(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         _mm_storeu_si128(reinterpret_cast<__m128i*>(out), m_sse);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;
         vec.V = m_vmx;
         Botan::store_le(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)

         if(CPUID::is_big_endian())
            {
            bswap().store_le(out);
            }
         else
            {
            vst1q_u8(out, vreinterpretq_u8_u32(m_neon));
            }
#else
         Botan::store_le(out, m_scalar[0], m_scalar[1], m_scalar[2], m_scalar[3]);
#endif
         }

      /**
      * Load a SIMD register with big-endian convention
      */
      void store_be(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         bswap().store_le(out);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;
         vec.V = m_vmx;
         Botan::store_be(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)

         if(CPUID::is_little_endian())
            {
            bswap().store_le(out);
            }
         else
            {
            vst1q_u8(out, vreinterpretq_u8_u32(m_neon));
            }

#else
         Botan::store_be(out, m_scalar[0], m_scalar[1], m_scalar[2], m_scalar[3]);
#endif
         }


      /*
      * This is used for SHA-2/SHACAL2
      * Return rotr(ROT1) ^ rotr(ROT2) ^ rotr(ROT3)
      */
      template<size_t ROT1, size_t ROT2, size_t ROT3>
      SIMD_4x32 rho() const
         {
         const SIMD_4x32 rot1 = this->rotr<ROT1>();
         const SIMD_4x32 rot2 = this->rotr<ROT2>();
         const SIMD_4x32 rot3 = this->rotr<ROT3>();
         return (rot1 ^ rot2 ^ rot3);
         }

      /**
      * Left rotation by a compile time constant
      */
      template<size_t ROT>
      SIMD_4x32 rotl() const
         {
         static_assert(ROT > 0 && ROT < 32, "Invalid rotation constant");

#if defined(BOTAN_SIMD_USE_SSE2)

         return SIMD_4x32(_mm_or_si128(_mm_slli_epi32(m_sse, static_cast<int>(ROT)),
                                       _mm_srli_epi32(m_sse, static_cast<int>(32-ROT))));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         const unsigned int r = static_cast<unsigned int>(ROT);
         return SIMD_4x32(vec_rl(m_vmx, (__vector unsigned int){r, r, r, r}));

#elif defined(BOTAN_SIMD_USE_NEON)

         #if defined(BOTAN_TARGET_ARCH_IS_ARM32)

         return SIMD_4x32(vorrq_u32(vshlq_n_u32(m_neon, static_cast<int>(ROT)),
                                    vshrq_n_u32(m_neon, static_cast<int>(32-ROT))));

         #else

         BOTAN_IF_CONSTEXPR(ROT == 8)
            {
            const uint8_t maskb[16] = { 3,0,1,2, 7,4,5,6, 11,8,9,10, 15,12,13,14 };
            const uint8x16_t mask = vld1q_u8(maskb);
            return SIMD_4x32(vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(m_neon), mask)));
            }
         else BOTAN_IF_CONSTEXPR(ROT == 16)
            {
            return SIMD_4x32(vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(m_neon))));
            }
         else
            {
            return SIMD_4x32(vorrq_u32(vshlq_n_u32(m_neon, static_cast<int>(ROT)),
                                       vshrq_n_u32(m_neon, static_cast<int>(32-ROT))));
            }

         #endif

#else
         return SIMD_4x32(Botan::rotl<ROT>(m_scalar[0]),
                          Botan::rotl<ROT>(m_scalar[1]),
                          Botan::rotl<ROT>(m_scalar[2]),
                          Botan::rotl<ROT>(m_scalar[3]));
#endif
         }

      /**
      * Right rotation by a compile time constant
      */
      template<size_t ROT>
      SIMD_4x32 rotr() const
         {
         return this->rotl<32-ROT>();
         }

      /**
      * Add elements of a SIMD vector
      */
      SIMD_4x32 operator+(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval += other;
         return retval;
         }

      /**
      * Subtract elements of a SIMD vector
      */
      SIMD_4x32 operator-(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval -= other;
         return retval;
         }

      /**
      * XOR elements of a SIMD vector
      */
      SIMD_4x32 operator^(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval ^= other;
         return retval;
         }

      /**
      * Binary OR elements of a SIMD vector
      */
      SIMD_4x32 operator|(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval |= other;
         return retval;
         }

      /**
      * Binary AND elements of a SIMD vector
      */
      SIMD_4x32 operator&(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval &= other;
         return retval;
         }

      void operator+=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_add_epi32(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_add(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vaddq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] += other.m_scalar[0];
         m_scalar[1] += other.m_scalar[1];
         m_scalar[2] += other.m_scalar[2];
         m_scalar[3] += other.m_scalar[3];
#endif
         }

      void operator-=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_sub_epi32(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_sub(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vsubq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] -= other.m_scalar[0];
         m_scalar[1] -= other.m_scalar[1];
         m_scalar[2] -= other.m_scalar[2];
         m_scalar[3] -= other.m_scalar[3];
#endif
         }

      void operator^=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_xor_si128(m_sse, other.m_sse);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_xor(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = veorq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] ^= other.m_scalar[0];
         m_scalar[1] ^= other.m_scalar[1];
         m_scalar[2] ^= other.m_scalar[2];
         m_scalar[3] ^= other.m_scalar[3];
#endif
         }

      void operator|=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_or_si128(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_or(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vorrq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] |= other.m_scalar[0];
         m_scalar[1] |= other.m_scalar[1];
         m_scalar[2] |= other.m_scalar[2];
         m_scalar[3] |= other.m_scalar[3];
#endif
         }

      void operator&=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_and_si128(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_and(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vandq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] &= other.m_scalar[0];
         m_scalar[1] &= other.m_scalar[1];
         m_scalar[2] &= other.m_scalar[2];
         m_scalar[3] &= other.m_scalar[3];
#endif
         }


      template<int SHIFT> SIMD_4x32 shl() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_slli_epi32(m_sse, SHIFT));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(SHIFT);
         return SIMD_4x32(vec_sl(m_vmx, (__vector unsigned int){s, s, s, s}));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vshlq_n_u32(m_neon, SHIFT));
#else
         return SIMD_4x32(m_scalar[0] << SHIFT,
                          m_scalar[1] << SHIFT,
                          m_scalar[2] << SHIFT,
                          m_scalar[3] << SHIFT);
#endif
         }

      template<int SHIFT> SIMD_4x32 shr() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_srli_epi32(m_sse, SHIFT));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(SHIFT);
         return SIMD_4x32(vec_sr(m_vmx, (__vector unsigned int){s, s, s, s}));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vshrq_n_u32(m_neon, SHIFT));
#else
         return SIMD_4x32(m_scalar[0] >> SHIFT, m_scalar[1] >> SHIFT,
                          m_scalar[2] >> SHIFT, m_scalar[3] >> SHIFT);

#endif
         }

      SIMD_4x32 operator~() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_xor_si128(m_sse, _mm_set1_epi32(0xFFFFFFFF)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_nor(m_vmx, m_vmx));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vmvnq_u32(m_neon));
#else
         return SIMD_4x32(~m_scalar[0], ~m_scalar[1], ~m_scalar[2], ~m_scalar[3]);
#endif
         }

      // (~reg) & other
      SIMD_4x32 andc(const SIMD_4x32& other) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_andnot_si128(m_sse, other.m_sse));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         /*
         AltiVec does arg1 & ~arg2 rather than SSE's ~arg1 & arg2
         so swap the arguments
         */
         return SIMD_4x32(vec_andc(other.m_vmx, m_vmx));
#elif defined(BOTAN_SIMD_USE_NEON)
         // NEON is also a & ~b
         return SIMD_4x32(vbicq_u32(other.m_neon, m_neon));
#else
         return SIMD_4x32((~m_scalar[0]) & other.m_scalar[0],
                          (~m_scalar[1]) & other.m_scalar[1],
                          (~m_scalar[2]) & other.m_scalar[2],
                          (~m_scalar[3]) & other.m_scalar[3]);
#endif
         }

      /**
      * Return copy *this with each word byte swapped
      */
      SIMD_4x32 bswap() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         __m128i T = m_sse;
         T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
         T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
         return SIMD_4x32(_mm_or_si128(_mm_srli_epi16(T, 8), _mm_slli_epi16(T, 8)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;

         vec.V = m_vmx;
         bswap_4(vec.R);
         return SIMD_4x32(vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)

         return SIMD_4x32(vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(m_neon))));

#else
         // scalar
         return SIMD_4x32(reverse_bytes(m_scalar[0]),
                          reverse_bytes(m_scalar[1]),
                          reverse_bytes(m_scalar[2]),
                          reverse_bytes(m_scalar[3]));
#endif
         }

      /**
      * 4x4 Transposition on SIMD registers
      */
      static void transpose(SIMD_4x32& B0, SIMD_4x32& B1,
                            SIMD_4x32& B2, SIMD_4x32& B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         const __m128i T0 = _mm_unpacklo_epi32(B0.m_sse, B1.m_sse);
         const __m128i T1 = _mm_unpacklo_epi32(B2.m_sse, B3.m_sse);
         const __m128i T2 = _mm_unpackhi_epi32(B0.m_sse, B1.m_sse);
         const __m128i T3 = _mm_unpackhi_epi32(B2.m_sse, B3.m_sse);

         B0.m_sse = _mm_unpacklo_epi64(T0, T1);
         B1.m_sse = _mm_unpackhi_epi64(T0, T1);
         B2.m_sse = _mm_unpacklo_epi64(T2, T3);
         B3.m_sse = _mm_unpackhi_epi64(T2, T3);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const __vector unsigned int T0 = vec_mergeh(B0.m_vmx, B2.m_vmx);
         const __vector unsigned int T1 = vec_mergeh(B1.m_vmx, B3.m_vmx);
         const __vector unsigned int T2 = vec_mergel(B0.m_vmx, B2.m_vmx);
         const __vector unsigned int T3 = vec_mergel(B1.m_vmx, B3.m_vmx);

         B0.m_vmx = vec_mergeh(T0, T1);
         B1.m_vmx = vec_mergel(T0, T1);
         B2.m_vmx = vec_mergeh(T2, T3);
         B3.m_vmx = vec_mergel(T2, T3);
#elif defined(BOTAN_SIMD_USE_NEON)

#if defined(BOTAN_TARGET_ARCH_IS_ARM32)

         const uint32x4x2_t T0 = vzipq_u32(B0.m_neon, B2.m_neon);
         const uint32x4x2_t T1 = vzipq_u32(B1.m_neon, B3.m_neon);
         const uint32x4x2_t O0 = vzipq_u32(T0.val[0], T1.val[0]);
         const uint32x4x2_t O1 = vzipq_u32(T0.val[1], T1.val[1]);

         B0.m_neon = O0.val[0];
         B1.m_neon = O0.val[1];
         B2.m_neon = O1.val[0];
         B3.m_neon = O1.val[1];

#elif defined(BOTAN_TARGET_ARCH_IS_ARM64)
         const uint32x4_t T0 = vzip1q_u32(B0.m_neon, B2.m_neon);
         const uint32x4_t T2 = vzip2q_u32(B0.m_neon, B2.m_neon);

         const uint32x4_t T1 = vzip1q_u32(B1.m_neon, B3.m_neon);
         const uint32x4_t T3 = vzip2q_u32(B1.m_neon, B3.m_neon);

         B0.m_neon = vzip1q_u32(T0, T1);
         B1.m_neon = vzip2q_u32(T0, T1);

         B2.m_neon = vzip1q_u32(T2, T3);
         B3.m_neon = vzip2q_u32(T2, T3);
#endif

#else
         // scalar
         SIMD_4x32 T0(B0.m_scalar[0], B1.m_scalar[0], B2.m_scalar[0], B3.m_scalar[0]);
         SIMD_4x32 T1(B0.m_scalar[1], B1.m_scalar[1], B2.m_scalar[1], B3.m_scalar[1]);
         SIMD_4x32 T2(B0.m_scalar[2], B1.m_scalar[2], B2.m_scalar[2], B3.m_scalar[2]);
         SIMD_4x32 T3(B0.m_scalar[3], B1.m_scalar[3], B2.m_scalar[3], B3.m_scalar[3]);

         B0 = T0;
         B1 = T1;
         B2 = T2;
         B3 = T3;
#endif
         }

   private:

#if defined(BOTAN_SIMD_USE_SSE2)
      explicit SIMD_4x32(__m128i in) : m_sse(in) {}
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      explicit SIMD_4x32(__vector unsigned int in) : m_vmx(in) {}
#elif defined(BOTAN_SIMD_USE_NEON)
      explicit SIMD_4x32(uint32x4_t in) : m_neon(in) {}
#endif

#if defined(BOTAN_SIMD_USE_SSE2)
      __m128i m_sse;
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      __vector unsigned int m_vmx;
#elif defined(BOTAN_SIMD_USE_NEON)
      uint32x4_t m_neon;
#else
      uint32_t m_scalar[4];
#endif
   };

}

namespace Botan {

namespace OS {

/*
* This header is internal (not installed) and these functions are not
* intended to be called by applications. However they are given public
* visibility (using BOTAN_TEST_API macro) for the tests. This also probably
* allows them to be overridden by the application on ELF systems, but
* this hasn't been tested.
*/


/**
* A wrapper around a simple blocking TCP socket
*/
class BOTAN_TEST_API Socket
   {
   public:
      /**
      * The socket will be closed upon destruction
      */
      virtual ~Socket() = default;

      /**
      * Write to the socket. Blocks until all bytes sent.
      * Throws on error.
      */
      virtual void write(const uint8_t buf[], size_t len) = 0;

      /**
      * Reads up to len bytes, returns bytes written to buf.
      * Returns 0 on EOF. Throws on error.
      */
      virtual size_t read(uint8_t buf[], size_t len) = 0;
   };

/**
* Open up a socket. Will throw on error. Returns null if sockets are
* not available on this platform.
*/
std::unique_ptr<Socket>
BOTAN_TEST_API open_socket(const std::string& hostname,
                           const std::string& service,
                           std::chrono::milliseconds timeout);

} // OS
} // Botan

namespace Botan {

inline std::vector<uint8_t> to_byte_vector(const std::string& s)
   {
   return std::vector<uint8_t>(s.cbegin(), s.cend());
   }

inline std::string to_string(const secure_vector<uint8_t> &bytes)
   {
   return std::string(bytes.cbegin(), bytes.cend());
   }

/**
* Return the keys of a map as a std::set
*/
template<typename K, typename V>
std::set<K> map_keys_as_set(const std::map<K, V>& kv)
   {
   std::set<K> s;
   for(auto&& i : kv)
      {
      s.insert(i.first);
      }
   return s;
   }

/*
* Searching through a std::map
* @param mapping the map to search
* @param key is what to look for
* @param null_result is the value to return if key is not in mapping
* @return mapping[key] or null_result
*/
template<typename K, typename V>
inline V search_map(const std::map<K, V>& mapping,
                    const K& key,
                    const V& null_result = V())
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return i->second;
   }

template<typename K, typename V, typename R>
inline R search_map(const std::map<K, V>& mapping, const K& key,
                    const R& null_result, const R& found_result)
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return found_result;
   }

/*
* Insert a key/value pair into a multimap
*/
template<typename K, typename V>
void multimap_insert(std::multimap<K, V>& multimap,
                     const K& key, const V& value)
   {
   multimap.insert(std::make_pair(key, value));
   }

/**
* Existence check for values
*/
template<typename T>
bool value_exists(const std::vector<T>& vec,
                  const T& val)
   {
   for(size_t i = 0; i != vec.size(); ++i)
      if(vec[i] == val)
         return true;
   return false;
   }

template<typename T, typename Pred>
void map_remove_if(Pred pred, T& assoc)
   {
   auto i = assoc.begin();
   while(i != assoc.end())
      {
      if(pred(i->first))
         assoc.erase(i++);
      else
         i++;
      }
   }

}

namespace Botan {

class BOTAN_TEST_API Timer final
   {
   public:
      Timer(const std::string& name,
            const std::string& provider,
            const std::string& doing,
            uint64_t event_mult,
            size_t buf_size,
            double clock_cycle_ratio,
            uint64_t clock_speed)
         : m_name(name + ((provider.empty() || provider == "base") ? "" : " [" + provider + "]"))
         , m_doing(doing)
         , m_buf_size(buf_size)
         , m_event_mult(event_mult)
         , m_clock_cycle_ratio(clock_cycle_ratio)
         , m_clock_speed(clock_speed)
         {}

      Timer(const std::string& name) :
         Timer(name, "", "", 1, 0, 0.0, 0)
         {}

      Timer(const std::string& name, size_t buf_size) :
         Timer(name, "", "", buf_size, buf_size, 0.0, 0)
         {}

      Timer(const Timer& other) = default;

      void start();

      void stop();

      bool under(std::chrono::milliseconds msec)
         {
         return (milliseconds() < msec.count());
         }

      class Timer_Scope final
         {
         public:
            explicit Timer_Scope(Timer& timer)
               : m_timer(timer)
               {
               m_timer.start();
               }
            ~Timer_Scope()
               {
               try
                  {
                  m_timer.stop();
                  }
               catch(...) {}
               }
         private:
            Timer& m_timer;
         };

      template<typename F>
      auto run(F f) -> decltype(f())
         {
         Timer_Scope timer(*this);
         return f();
         }

      template<typename F>
      void run_until_elapsed(std::chrono::milliseconds msec, F f)
         {
         while(this->under(msec))
            {
            run(f);
            }
         }

      uint64_t value() const
         {
         return m_time_used;
         }

      double seconds() const
         {
         return milliseconds() / 1000.0;
         }

      double milliseconds() const
         {
         return value() / 1000000.0;
         }

      double ms_per_event() const
         {
         return milliseconds() / events();
         }

      uint64_t cycles_consumed() const
         {
         if(m_clock_speed != 0)
            {
            return static_cast<uint64_t>((m_clock_speed * value()) / 1000.0);
            }
         return m_cpu_cycles_used;
         }

      uint64_t events() const
         {
         return m_event_count * m_event_mult;
         }

      const std::string& get_name() const
         {
         return m_name;
         }

      const std::string& doing() const
         {
         return m_doing;
         }

      size_t buf_size() const
         {
         return m_buf_size;
         }

      double bytes_per_second() const
         {
         return seconds() > 0.0 ? events() / seconds() : 0.0;
         }

      double events_per_second() const
         {
         return seconds() > 0.0 ? events() / seconds() : 0.0;
         }

      double seconds_per_event() const
         {
         return events() > 0 ? seconds() / events() : 0.0;
         }

      void set_custom_msg(const std::string& s)
         {
         m_custom_msg = s;
         }

      bool operator<(const Timer& other) const;

      std::string to_string() const;

   private:
      std::string result_string_bps() const;
      std::string result_string_ops() const;

      // const data
      std::string m_name, m_doing;
      size_t m_buf_size;
      uint64_t m_event_mult;
      double m_clock_cycle_ratio;
      uint64_t m_clock_speed;

      // set at runtime
      std::string m_custom_msg;
      uint64_t m_time_used = 0, m_timer_start = 0;
      uint64_t m_event_count = 0;

      uint64_t m_max_time = 0, m_min_time = 0;
      uint64_t m_cpu_cycles_start = 0, m_cpu_cycles_used = 0;
   };

}

namespace Botan {

namespace TLS {

/**
* TLS CBC+HMAC AEAD base class (GenericBlockCipher in TLS spec)
* This is the weird TLS-specific mode, not for general consumption.
*/
class BOTAN_TEST_API TLS_CBC_HMAC_AEAD_Mode : public AEAD_Mode
   {
   public:
      size_t process(uint8_t buf[], size_t sz) override final;

      std::string name() const override final;

      void set_associated_data(const uint8_t ad[], size_t ad_len) override;

      size_t update_granularity() const override final;

      Key_Length_Specification key_spec() const override final;

      bool valid_nonce_length(size_t nl) const override final;

      size_t tag_size() const override final { return m_tag_size; }

      size_t default_nonce_length() const override final { return m_iv_size; }

      void clear() override final;

      void reset() override final;

 protected:
      TLS_CBC_HMAC_AEAD_Mode(Cipher_Dir direction,
                             std::unique_ptr<BlockCipher> cipher,
                             std::unique_ptr<MessageAuthenticationCode> mac,
                             size_t cipher_keylen,
                             size_t mac_keylen,
                             bool use_explicit_iv,
                             bool use_encrypt_then_mac);

      size_t cipher_keylen() const { return m_cipher_keylen; }
      size_t mac_keylen() const { return m_mac_keylen; }
      size_t iv_size() const { return m_iv_size; }
      size_t block_size() const { return m_block_size; }

      bool use_encrypt_then_mac() const { return m_use_encrypt_then_mac; }

      Cipher_Mode& cbc() const { return *m_cbc; }

      MessageAuthenticationCode& mac() const
         {
         BOTAN_ASSERT_NONNULL(m_mac);
         return *m_mac;
         }

      secure_vector<uint8_t>& cbc_state() { return m_cbc_state; }
      std::vector<uint8_t>& assoc_data() { return m_ad; }
      secure_vector<uint8_t>& msg() { return m_msg; }

      std::vector<uint8_t> assoc_data_with_len(uint16_t len);

   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override final;

      void key_schedule(const uint8_t key[], size_t length) override final;

      const std::string m_cipher_name;
      const std::string m_mac_name;
      size_t m_cipher_keylen;
      size_t m_mac_keylen;
      size_t m_iv_size;
      size_t m_tag_size;
      size_t m_block_size;
      bool m_use_encrypt_then_mac;

      std::unique_ptr<Cipher_Mode> m_cbc;
      std::unique_ptr<MessageAuthenticationCode> m_mac;

      secure_vector<uint8_t> m_cbc_state;
      std::vector<uint8_t> m_ad;
      secure_vector<uint8_t> m_msg;
   };

/**
* TLS_CBC_HMAC_AEAD Encryption
*/
class BOTAN_TEST_API TLS_CBC_HMAC_AEAD_Encryption final : public TLS_CBC_HMAC_AEAD_Mode
   {
   public:
      /**
      */
      TLS_CBC_HMAC_AEAD_Encryption(
                             std::unique_ptr<BlockCipher> cipher,
                             std::unique_ptr<MessageAuthenticationCode> mac,
                             const size_t cipher_keylen,
                             const size_t mac_keylen,
                             bool use_explicit_iv,
                             bool use_encrypt_then_mac) :
         TLS_CBC_HMAC_AEAD_Mode(ENCRYPTION,
                                std::move(cipher),
                                std::move(mac),
                                cipher_keylen,
                                mac_keylen,
                                use_explicit_iv,
                                use_encrypt_then_mac)
         {}

      void set_associated_data(const uint8_t ad[], size_t ad_len) override;

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override { return 0; }

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   private:
      void cbc_encrypt_record(uint8_t record_contents[], size_t record_len);
   };

/**
* TLS_CBC_HMAC_AEAD Decryption
*/
class BOTAN_TEST_API TLS_CBC_HMAC_AEAD_Decryption final : public TLS_CBC_HMAC_AEAD_Mode
   {
   public:
      /**
      */
      TLS_CBC_HMAC_AEAD_Decryption(std::unique_ptr<BlockCipher> cipher,
                                   std::unique_ptr<MessageAuthenticationCode> mac,
                                   const size_t cipher_keylen,
                                   const size_t mac_keylen,
                                   bool use_explicit_iv,
                                   bool use_encrypt_then_mac) :
         TLS_CBC_HMAC_AEAD_Mode(DECRYPTION,
                                std::move(cipher),
                                std::move(mac),
                                cipher_keylen,
                                mac_keylen,
                                use_explicit_iv,
                                use_encrypt_then_mac)
         {}

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override { return tag_size(); }

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

   private:
      void cbc_decrypt_record(uint8_t record_contents[], size_t record_len);

      void perform_additional_compressions(size_t plen, size_t padlen);
   };

/**
* Check the TLS padding of a record
* @param record the record bits
* @param record_len length of record
* @return 0 if padding is invalid, otherwise padding_bytes + 1
*/
BOTAN_TEST_API uint16_t check_tls_cbc_padding(const uint8_t record[], size_t record_len);

}

}

namespace Botan {

namespace TLS {

/**
* TLS Handshake Hash
*/
class Handshake_Hash final
   {
   public:
      void update(const uint8_t in[], size_t length)
         { m_data += std::make_pair(in, length); }

      void update(const std::vector<uint8_t>& in)
         { m_data += in; }

      secure_vector<uint8_t> final(Protocol_Version version,
                                const std::string& mac_algo) const;

      const std::vector<uint8_t>& get_contents() const { return m_data; }

      void reset() { m_data.clear(); }
   private:
      std::vector<uint8_t> m_data;
   };

}

}

namespace Botan {

namespace TLS {

class Handshake_Message;

/**
* Handshake IO Interface
*/
class Handshake_IO
   {
   public:
      virtual Protocol_Version initial_record_version() const = 0;

      virtual std::vector<uint8_t> send(const Handshake_Message& msg) = 0;

      virtual bool timeout_check() = 0;

      virtual std::vector<uint8_t> format(
         const std::vector<uint8_t>& handshake_msg,
         Handshake_Type handshake_type) const = 0;

      virtual void add_record(const std::vector<uint8_t>& record,
                              Record_Type type,
                              uint64_t sequence_number) = 0;

      /**
      * Returns (HANDSHAKE_NONE, std::vector<>()) if no message currently available
      */
      virtual std::pair<Handshake_Type, std::vector<uint8_t>>
         get_next_record(bool expecting_ccs) = 0;

      Handshake_IO() = default;

      Handshake_IO(const Handshake_IO&) = delete;

      Handshake_IO& operator=(const Handshake_IO&) = delete;

      virtual ~Handshake_IO() = default;
   };

/**
* Handshake IO for stream-based handshakes
*/
class Stream_Handshake_IO final : public Handshake_IO
   {
   public:
      typedef std::function<void (uint8_t, const std::vector<uint8_t>&)> writer_fn;

      explicit Stream_Handshake_IO(writer_fn writer) : m_send_hs(writer) {}

      Protocol_Version initial_record_version() const override;

      bool timeout_check() override { return false; }

      std::vector<uint8_t> send(const Handshake_Message& msg) override;

      std::vector<uint8_t> format(
         const std::vector<uint8_t>& handshake_msg,
         Handshake_Type handshake_type) const override;

      void add_record(const std::vector<uint8_t>& record,
                      Record_Type type,
                      uint64_t sequence_number) override;

      std::pair<Handshake_Type, std::vector<uint8_t>>
         get_next_record(bool expecting_ccs) override;
   private:
      std::deque<uint8_t> m_queue;
      writer_fn m_send_hs;
   };

/**
* Handshake IO for datagram-based handshakes
*/
class Datagram_Handshake_IO final : public Handshake_IO
   {
   public:
      typedef std::function<void (uint16_t, uint8_t, const std::vector<uint8_t>&)> writer_fn;

      Datagram_Handshake_IO(writer_fn writer,
                            class Connection_Sequence_Numbers& seq,
                            uint16_t mtu, uint64_t initial_timeout_ms, uint64_t max_timeout_ms) :
         m_seqs(seq),
         m_flights(1),
         m_initial_timeout(initial_timeout_ms),
         m_max_timeout(max_timeout_ms),
         m_send_hs(writer),
         m_mtu(mtu)
         {}

      Protocol_Version initial_record_version() const override;

      bool timeout_check() override;

      std::vector<uint8_t> send(const Handshake_Message& msg) override;

      std::vector<uint8_t> format(
         const std::vector<uint8_t>& handshake_msg,
         Handshake_Type handshake_type) const override;

      void add_record(const std::vector<uint8_t>& record,
                      Record_Type type,
                      uint64_t sequence_number) override;

      std::pair<Handshake_Type, std::vector<uint8_t>>
         get_next_record(bool expecting_ccs) override;
   private:
      void retransmit_flight(size_t flight);
      void retransmit_last_flight();

      std::vector<uint8_t> format_fragment(
         const uint8_t fragment[],
         size_t fragment_len,
         uint16_t frag_offset,
         uint16_t msg_len,
         Handshake_Type type,
         uint16_t msg_sequence) const;

      std::vector<uint8_t> format_w_seq(
         const std::vector<uint8_t>& handshake_msg,
         Handshake_Type handshake_type,
         uint16_t msg_sequence) const;

      std::vector<uint8_t> send_message(uint16_t msg_seq, uint16_t epoch,
                                     Handshake_Type msg_type,
                                     const std::vector<uint8_t>& msg);

      class Handshake_Reassembly final
         {
         public:
            void add_fragment(const uint8_t fragment[],
                              size_t fragment_length,
                              size_t fragment_offset,
                              uint16_t epoch,
                              uint8_t msg_type,
                              size_t msg_length);

            bool complete() const;

            uint16_t epoch() const { return m_epoch; }

            std::pair<Handshake_Type, std::vector<uint8_t>> message() const;
         private:
            uint8_t m_msg_type = HANDSHAKE_NONE;
            size_t m_msg_length = 0;
            uint16_t m_epoch = 0;

            // vector<bool> m_seen;
            // vector<uint8_t> m_fragments
            std::map<size_t, uint8_t> m_fragments;
            std::vector<uint8_t> m_message;
         };

      struct Message_Info final
         {
         Message_Info(uint16_t e, Handshake_Type mt, const std::vector<uint8_t>& msg) :
            epoch(e), msg_type(mt), msg_bits(msg) {}

         Message_Info() : epoch(0xFFFF), msg_type(HANDSHAKE_NONE) {}

         uint16_t epoch;
         Handshake_Type msg_type;
         std::vector<uint8_t> msg_bits;
         };

      class Connection_Sequence_Numbers& m_seqs;
      std::map<uint16_t, Handshake_Reassembly> m_messages;
      std::set<uint16_t> m_ccs_epochs;
      std::vector<std::vector<uint16_t>> m_flights;
      std::map<uint16_t, Message_Info> m_flight_data;

      uint64_t m_initial_timeout = 0;
      uint64_t m_max_timeout = 0;

      uint64_t m_last_write = 0;
      uint64_t m_next_timeout = 0;

      uint16_t m_in_message_seq = 0;
      uint16_t m_out_message_seq = 0;

      writer_fn m_send_hs;
      uint16_t m_mtu;
   };

}

}

namespace Botan {

namespace TLS {

class Handshake_State;

/**
* TLS Session Keys
*/
class Session_Keys final
   {
   public:
      /**
      * @return client encipherment key
      */
      const SymmetricKey& client_cipher_key() const { return m_c_cipher; }

      /**
      * @return client encipherment key
      */
      const SymmetricKey& server_cipher_key() const { return m_s_cipher; }

      /**
      * @return client MAC key
      */
      const SymmetricKey& client_mac_key() const { return m_c_mac; }

      /**
      * @return server MAC key
      */
      const SymmetricKey& server_mac_key() const { return m_s_mac; }

      /**
      * @return client IV
      */
      const InitializationVector& client_iv() const { return m_c_iv; }

      /**
      * @return server IV
      */
      const InitializationVector& server_iv() const { return m_s_iv; }

      /**
      * @return TLS master secret
      */
      const secure_vector<uint8_t>& master_secret() const { return m_master_sec; }

      Session_Keys() = default;

      /**
      * @param state state the handshake state
      * @param pre_master_secret the pre-master secret
      * @param resuming whether this TLS session is resumed
      */
      Session_Keys(const Handshake_State* state,
                   const secure_vector<uint8_t>& pre_master_secret,
                   bool resuming);

   private:
      secure_vector<uint8_t> m_master_sec;
      SymmetricKey m_c_cipher, m_s_cipher, m_c_mac, m_s_mac;
      InitializationVector m_c_iv, m_s_iv;
   };

}

}

namespace Botan {

class KDF;

namespace TLS {

class Callbacks;
class Policy;

class Hello_Verify_Request;
class Client_Hello;
class Server_Hello;
class Certificate;
class Certificate_Status;
class Server_Key_Exchange;
class Certificate_Req;
class Server_Hello_Done;
class Certificate;
class Client_Key_Exchange;
class Certificate_Verify;
class New_Session_Ticket;
class Finished;

/**
* SSL/TLS Handshake State
*/
class Handshake_State
   {
   public:
      Handshake_State(Handshake_IO* io, Callbacks& callbacks);

      virtual ~Handshake_State() = default;

      Handshake_State(const Handshake_State&) = delete;
      Handshake_State& operator=(const Handshake_State&) = delete;

      Handshake_IO& handshake_io() { return *m_handshake_io; }

      /**
      * Return true iff we have received a particular message already
      * @param msg_type the message type
      */
      bool received_handshake_msg(Handshake_Type msg_type) const;

      /**
      * Confirm that we were expecting this message type
      * @param msg_type the message type
      */
      void confirm_transition_to(Handshake_Type msg_type);

      /**
      * Record that we are expecting a particular message type next
      * @param msg_type the message type
      */
      void set_expected_next(Handshake_Type msg_type);

      std::pair<Handshake_Type, std::vector<uint8_t>>
         get_next_handshake_msg();

      std::vector<uint8_t> session_ticket() const;

      std::pair<std::string, Signature_Format>
         parse_sig_format(const Public_Key& key,
                          Signature_Scheme scheme,
                          bool for_client_auth,
                          const Policy& policy) const;

      std::pair<std::string, Signature_Format>
         choose_sig_format(const Private_Key& key,
                           Signature_Scheme& scheme,
                           bool for_client_auth,
                           const Policy& policy) const;

      std::string srp_identifier() const;

      KDF* protocol_specific_prf() const;

      Protocol_Version version() const { return m_version; }

      void set_version(const Protocol_Version& version);

      void hello_verify_request(const Hello_Verify_Request& hello_verify);

      void client_hello(Client_Hello* client_hello);
      void server_hello(Server_Hello* server_hello);
      void server_certs(Certificate* server_certs);
      void server_cert_status(Certificate_Status* server_cert_status);
      void server_kex(Server_Key_Exchange* server_kex);
      void cert_req(Certificate_Req* cert_req);
      void server_hello_done(Server_Hello_Done* server_hello_done);
      void client_certs(Certificate* client_certs);
      void client_kex(Client_Key_Exchange* client_kex);
      void client_verify(Certificate_Verify* client_verify);
      void new_session_ticket(New_Session_Ticket* new_session_ticket);
      void server_finished(Finished* server_finished);
      void client_finished(Finished* client_finished);

      const Client_Hello* client_hello() const
         { return m_client_hello.get(); }

      const Server_Hello* server_hello() const
         { return m_server_hello.get(); }

      const Certificate* server_certs() const
         { return m_server_certs.get(); }

      const Server_Key_Exchange* server_kex() const
         { return m_server_kex.get(); }

      const Certificate_Req* cert_req() const
         { return m_cert_req.get(); }

      const Server_Hello_Done* server_hello_done() const
         { return m_server_hello_done.get(); }

      const Certificate* client_certs() const
         { return m_client_certs.get(); }

      const Client_Key_Exchange* client_kex() const
         { return m_client_kex.get(); }

      const Certificate_Verify* client_verify() const
         { return m_client_verify.get(); }

      const Certificate_Status* server_cert_status() const
         { return m_server_cert_status.get(); }

      const New_Session_Ticket* new_session_ticket() const
         { return m_new_session_ticket.get(); }

      const Finished* server_finished() const
         { return m_server_finished.get(); }

      const Finished* client_finished() const
         { return m_client_finished.get(); }

      const Ciphersuite& ciphersuite() const { return m_ciphersuite; }

      const Session_Keys& session_keys() const { return m_session_keys; }

      Callbacks& callbacks() const { return m_callbacks; }

      void compute_session_keys();

      void compute_session_keys(const secure_vector<uint8_t>& resume_master_secret);

      Handshake_Hash& hash() { return m_handshake_hash; }

      const Handshake_Hash& hash() const { return m_handshake_hash; }

      void note_message(const Handshake_Message& msg);
   private:

      Callbacks& m_callbacks;

      std::unique_ptr<Handshake_IO> m_handshake_io;

      uint32_t m_hand_expecting_mask = 0;
      uint32_t m_hand_received_mask = 0;
      Protocol_Version m_version;
      Ciphersuite m_ciphersuite;
      Session_Keys m_session_keys;
      Handshake_Hash m_handshake_hash;

      std::unique_ptr<Client_Hello> m_client_hello;
      std::unique_ptr<Server_Hello> m_server_hello;
      std::unique_ptr<Certificate> m_server_certs;
      std::unique_ptr<Certificate_Status> m_server_cert_status;
      std::unique_ptr<Server_Key_Exchange> m_server_kex;
      std::unique_ptr<Certificate_Req> m_cert_req;
      std::unique_ptr<Server_Hello_Done> m_server_hello_done;
      std::unique_ptr<Certificate> m_client_certs;
      std::unique_ptr<Client_Key_Exchange> m_client_kex;
      std::unique_ptr<Certificate_Verify> m_client_verify;
      std::unique_ptr<New_Session_Ticket> m_new_session_ticket;
      std::unique_ptr<Finished> m_server_finished;
      std::unique_ptr<Finished> m_client_finished;
   };

}

}

namespace Botan {

namespace TLS {

/**
* Helper class for decoding TLS protocol messages
*/
class TLS_Data_Reader final
   {
   public:
      TLS_Data_Reader(const char* type, const std::vector<uint8_t>& buf_in) :
         m_typename(type), m_buf(buf_in), m_offset(0) {}

      void assert_done() const
         {
         if(has_remaining())
            throw decode_error("Extra bytes at end of message");
         }

      size_t read_so_far() const { return m_offset; }

      size_t remaining_bytes() const { return m_buf.size() - m_offset; }

      bool has_remaining() const { return (remaining_bytes() > 0); }

      std::vector<uint8_t> get_remaining()
         {
         return std::vector<uint8_t>(m_buf.begin() + m_offset, m_buf.end());
         }

      void discard_next(size_t bytes)
         {
         assert_at_least(bytes);
         m_offset += bytes;
         }

      uint32_t get_uint32_t()
         {
         assert_at_least(4);
         uint32_t result = make_uint32(m_buf[m_offset  ], m_buf[m_offset+1],
                                     m_buf[m_offset+2], m_buf[m_offset+3]);
         m_offset += 4;
         return result;
         }

      uint16_t get_uint16_t()
         {
         assert_at_least(2);
         uint16_t result = make_uint16(m_buf[m_offset], m_buf[m_offset+1]);
         m_offset += 2;
         return result;
         }

      uint8_t get_byte()
         {
         assert_at_least(1);
         uint8_t result = m_buf[m_offset];
         m_offset += 1;
         return result;
         }

      template<typename T, typename Container>
      Container get_elem(size_t num_elems)
         {
         assert_at_least(num_elems * sizeof(T));

         Container result(num_elems);

         for(size_t i = 0; i != num_elems; ++i)
            result[i] = load_be<T>(&m_buf[m_offset], i);

         m_offset += num_elems * sizeof(T);

         return result;
         }

      template<typename T>
      std::vector<T> get_range(size_t len_bytes,
                               size_t min_elems,
                               size_t max_elems)
         {
         const size_t num_elems =
            get_num_elems(len_bytes, sizeof(T), min_elems, max_elems);

         return get_elem<T, std::vector<T> >(num_elems);
         }

      template<typename T>
      std::vector<T> get_range_vector(size_t len_bytes,
                                      size_t min_elems,
                                      size_t max_elems)
         {
         const size_t num_elems =
            get_num_elems(len_bytes, sizeof(T), min_elems, max_elems);

         return get_elem<T, std::vector<T> >(num_elems);
         }

      std::string get_string(size_t len_bytes,
                             size_t min_bytes,
                             size_t max_bytes)
         {
         std::vector<uint8_t> v =
            get_range_vector<uint8_t>(len_bytes, min_bytes, max_bytes);

         return std::string(cast_uint8_ptr_to_char(v.data()), v.size());
         }

      template<typename T>
      std::vector<T> get_fixed(size_t size)
         {
         return get_elem<T, std::vector<T> >(size);
         }

   private:
      size_t get_length_field(size_t len_bytes)
         {
         assert_at_least(len_bytes);

         if(len_bytes == 1)
            return get_byte();
         else if(len_bytes == 2)
            return get_uint16_t();

         throw decode_error("Bad length size");
         }

      size_t get_num_elems(size_t len_bytes,
                           size_t T_size,
                           size_t min_elems,
                           size_t max_elems)
         {
         const size_t byte_length = get_length_field(len_bytes);

         if(byte_length % T_size != 0)
            throw decode_error("Size isn't multiple of T");

         const size_t num_elems = byte_length / T_size;

         if(num_elems < min_elems || num_elems > max_elems)
            throw decode_error("Length field outside parameters");

         return num_elems;
         }

      void assert_at_least(size_t n) const
         {
         if(m_buf.size() - m_offset < n)
            throw decode_error("Expected " + std::to_string(n) +
                               " bytes remaining, only " +
                               std::to_string(m_buf.size()-m_offset) +
                               " left");
         }

      Decoding_Error decode_error(const std::string& why) const
         {
         return Decoding_Error("Invalid " + std::string(m_typename) + ": " + why);
         }

      const char* m_typename;
      const std::vector<uint8_t>& m_buf;
      size_t m_offset;
   };

/**
* Helper function for encoding length-tagged vectors
*/
template<typename T, typename Alloc>
void append_tls_length_value(std::vector<uint8_t, Alloc>& buf,
                             const T* vals,
                             size_t vals_size,
                             size_t tag_size)
   {
   const size_t T_size = sizeof(T);
   const size_t val_bytes = T_size * vals_size;

   if(tag_size != 1 && tag_size != 2)
      throw Invalid_Argument("append_tls_length_value: invalid tag size");

   if((tag_size == 1 && val_bytes > 255) ||
      (tag_size == 2 && val_bytes > 65535))
      throw Invalid_Argument("append_tls_length_value: value too large");

   for(size_t i = 0; i != tag_size; ++i)
      buf.push_back(get_byte(sizeof(val_bytes)-tag_size+i, val_bytes));

   for(size_t i = 0; i != vals_size; ++i)
      for(size_t j = 0; j != T_size; ++j)
         buf.push_back(get_byte(j, vals[i]));
   }

template<typename T, typename Alloc, typename Alloc2>
void append_tls_length_value(std::vector<uint8_t, Alloc>& buf,
                             const std::vector<T, Alloc2>& vals,
                             size_t tag_size)
   {
   append_tls_length_value(buf, vals.data(), vals.size(), tag_size);
   }

template<typename Alloc>
void append_tls_length_value(std::vector<uint8_t, Alloc>& buf,
                             const std::string& str,
                             size_t tag_size)
   {
   append_tls_length_value(buf,
                           cast_char_ptr_to_uint8(str.data()),
                           str.size(),
                           tag_size);
   }

}

}

namespace Botan {

namespace TLS {

class Ciphersuite;
class Session_Keys;

class Connection_Sequence_Numbers;

/**
* TLS Cipher State
*/
class Connection_Cipher_State final
   {
   public:
      /**
      * Initialize a new cipher state
      */
      Connection_Cipher_State(Protocol_Version version,
                              Connection_Side which_side,
                              bool is_our_side,
                              const Ciphersuite& suite,
                              const Session_Keys& keys,
                              bool uses_encrypt_then_mac);

      AEAD_Mode* aead() { return m_aead.get(); }

      std::vector<uint8_t> aead_nonce(uint64_t seq, RandomNumberGenerator& rng);

      std::vector<uint8_t> aead_nonce(const uint8_t record[], size_t record_len, uint64_t seq);

      std::vector<uint8_t> format_ad(uint64_t seq, uint8_t type,
                                  Protocol_Version version,
                                  uint16_t ptext_length);

      size_t nonce_bytes_from_handshake() const { return m_nonce_bytes_from_handshake; }
      size_t nonce_bytes_from_record() const { return m_nonce_bytes_from_record; }

      Nonce_Format nonce_format() const { return m_nonce_format; }

      std::chrono::seconds age() const
         {
         return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - m_start_time);
         }

   private:
      std::chrono::system_clock::time_point m_start_time;
      std::unique_ptr<AEAD_Mode> m_aead;

      std::vector<uint8_t> m_nonce;
      Nonce_Format m_nonce_format = Nonce_Format::CBC_MODE;
      size_t m_nonce_bytes_from_handshake = 0;
      size_t m_nonce_bytes_from_record = 0;
   };

class Record final
   {
   public:
      Record(secure_vector<uint8_t>& data,
             uint64_t* sequence,
             Protocol_Version* protocol_version,
             Record_Type* type)
         : m_data(data), m_sequence(sequence), m_protocol_version(protocol_version),
           m_type(type), m_size(data.size()) {}

      secure_vector<uint8_t>& get_data() { return m_data; }

      Protocol_Version* get_protocol_version() { return m_protocol_version; }

      uint64_t* get_sequence() { return m_sequence; }

      Record_Type* get_type() { return m_type; }

      size_t& get_size() { return m_size; }

   private:
      secure_vector<uint8_t>& m_data;
      uint64_t* m_sequence;
      Protocol_Version* m_protocol_version;
      Record_Type* m_type;
      size_t m_size;
   };

class Record_Message final
   {
   public:
      Record_Message(const uint8_t* data, size_t size)
         : m_type(0), m_sequence(0), m_data(data), m_size(size) {}
      Record_Message(uint8_t type, uint64_t sequence, const uint8_t* data, size_t size)
         : m_type(type), m_sequence(sequence), m_data(data),
           m_size(size) {}

      uint8_t& get_type() { return m_type; }
      uint64_t& get_sequence() { return m_sequence; }
      const uint8_t* get_data() { return m_data; }
      size_t& get_size() { return m_size; }

   private:
      uint8_t m_type;
      uint64_t m_sequence;
      const uint8_t* m_data;
      size_t m_size;
};

class Record_Raw_Input final
   {
   public:
      Record_Raw_Input(const uint8_t* data, size_t size, size_t& consumed,
                       bool is_datagram)
         : m_data(data), m_size(size), m_consumed(consumed),
           m_is_datagram(is_datagram) {}

      const uint8_t*& get_data() { return m_data; }

      size_t& get_size() { return m_size; }

      size_t& get_consumed() { return m_consumed; }
      void set_consumed(size_t consumed) { m_consumed = consumed; }

      bool is_datagram() { return m_is_datagram; }

   private:
      const uint8_t* m_data;
      size_t m_size;
      size_t& m_consumed;
      bool m_is_datagram;
   };


/**
* Create a TLS record
* @param write_buffer the output record is placed here
* @param rec_msg is the plaintext message
* @param version is the protocol version
* @param msg_sequence is the sequence number
* @param cipherstate is the writing cipher state
* @param rng is a random number generator
*/
void write_record(secure_vector<uint8_t>& write_buffer,
                  Record_Message rec_msg,
                  Protocol_Version version,
                  uint64_t msg_sequence,
                  Connection_Cipher_State* cipherstate,
                  RandomNumberGenerator& rng);

// epoch -> cipher state
typedef std::function<std::shared_ptr<Connection_Cipher_State> (uint16_t)> get_cipherstate_fn;

/**
* Decode a TLS record
* @return zero if full message, else number of bytes still needed
*/
size_t read_record(secure_vector<uint8_t>& read_buffer,
                   Record_Raw_Input& raw_input,
                   Record& rec,
                   Connection_Sequence_Numbers* sequence_numbers,
                   get_cipherstate_fn get_cipherstate);

}

}

namespace Botan {

namespace TLS {

class Connection_Sequence_Numbers
   {
   public:
      virtual ~Connection_Sequence_Numbers() = default;

      virtual void new_read_cipher_state() = 0;
      virtual void new_write_cipher_state() = 0;

      virtual uint16_t current_read_epoch() const = 0;
      virtual uint16_t current_write_epoch() const = 0;

      virtual uint64_t next_write_sequence(uint16_t) = 0;
      virtual uint64_t next_read_sequence() = 0;

      virtual bool already_seen(uint64_t seq) const = 0;
      virtual void read_accept(uint64_t seq) = 0;
   };

class Stream_Sequence_Numbers final : public Connection_Sequence_Numbers
   {
   public:
      void new_read_cipher_state() override { m_read_seq_no = 0; m_read_epoch++; }
      void new_write_cipher_state() override { m_write_seq_no = 0; m_write_epoch++; }

      uint16_t current_read_epoch() const override { return m_read_epoch; }
      uint16_t current_write_epoch() const override { return m_write_epoch; }

      uint64_t next_write_sequence(uint16_t) override { return m_write_seq_no++; }
      uint64_t next_read_sequence() override { return m_read_seq_no; }

      bool already_seen(uint64_t) const override { return false; }
      void read_accept(uint64_t) override { m_read_seq_no++; }
   private:
      uint64_t m_write_seq_no = 0;
      uint64_t m_read_seq_no = 0;
      uint16_t m_read_epoch = 0;
      uint16_t m_write_epoch = 0;
   };

class Datagram_Sequence_Numbers final : public Connection_Sequence_Numbers
   {
   public:
      Datagram_Sequence_Numbers() { m_write_seqs[0] = 0; }

      void new_read_cipher_state() override { m_read_epoch++; }

      void new_write_cipher_state() override
         {
         m_write_epoch++;
         m_write_seqs[m_write_epoch] = 0;
         }

      uint16_t current_read_epoch() const override { return m_read_epoch; }
      uint16_t current_write_epoch() const override { return m_write_epoch; }

      uint64_t next_write_sequence(uint16_t epoch) override
         {
         auto i = m_write_seqs.find(epoch);
         BOTAN_ASSERT(i != m_write_seqs.end(), "Found epoch");
         return (static_cast<uint64_t>(epoch) << 48) | i->second++;
         }

      uint64_t next_read_sequence() override
         {
         throw Invalid_State("DTLS uses explicit sequence numbers");
         }

      bool already_seen(uint64_t sequence) const override
         {
         const size_t window_size = sizeof(m_window_bits) * 8;

         if(sequence > m_window_highest)
            return false;

         const uint64_t offset = m_window_highest - sequence;

         if(offset >= window_size)
            return true; // really old?

         return (((m_window_bits >> offset) & 1) == 1);
         }

      void read_accept(uint64_t sequence) override
         {
         const size_t window_size = sizeof(m_window_bits) * 8;

         if(sequence > m_window_highest)
            {
            const uint64_t offset = sequence - m_window_highest;
            m_window_highest += offset;

            if(offset >= window_size)
               m_window_bits = 0;
            else
               m_window_bits <<= offset;

            m_window_bits |= 0x01;
            }
         else
            {
            const uint64_t offset = m_window_highest - sequence;
            m_window_bits |= (static_cast<uint64_t>(1) << offset);
            }
         }

   private:
      std::map<uint16_t, uint64_t> m_write_seqs;
      uint16_t m_write_epoch = 0;
      uint16_t m_read_epoch = 0;
      uint64_t m_window_highest = 0;
      uint64_t m_window_bits = 0;
   };

}

}

namespace Botan {

class XMSS_Signature final
   {
   public:
      /**
       * Creates a signature from an XMSS signature method and a uint8_t sequence
       * representing a raw signature.
       *
       * @param oid XMSS signature method
       * @param raw_sig An XMSS signature serialized using
       *                XMSS_Signature::bytes().
       **/
      XMSS_Signature(XMSS_Parameters::xmss_algorithm_t oid,
                     const secure_vector<uint8_t>& raw_sig);

      /**
       * Creates an XMSS Signature from a leaf index used for signature
       * generation, a random value and a tree signature.
       *
       * @param leaf_idx Leaf index used to generate the signature.
       * @param randomness A random value.
       * @param tree_sig A tree signature.
       **/
      XMSS_Signature(size_t leaf_idx,
                     const secure_vector<uint8_t>& randomness,
                     const XMSS_WOTS_PublicKey::TreeSignature& tree_sig)
         : m_leaf_idx(leaf_idx), m_randomness(randomness),
           m_tree_sig(tree_sig) {}

      /**
       * Creates an XMSS Signature from a leaf index used for signature
       * generation, a random value and a tree signature.
       *
       * @param leaf_idx Leaf index used to generate the signature.
       * @param randomness A random value.
       * @param tree_sig A tree signature.
       **/
      XMSS_Signature(size_t leaf_idx,
                     secure_vector<uint8_t>&& randomness,
                     XMSS_WOTS_PublicKey::TreeSignature&& tree_sig)
         : m_leaf_idx(leaf_idx), m_randomness(std::move(randomness)),
           m_tree_sig(std::move(tree_sig)) {}

      size_t unused_leaf_index() const { return m_leaf_idx; }
      void set_unused_leaf_idx(size_t idx) { m_leaf_idx = idx; }

      const secure_vector<uint8_t> randomness() const
         {
         return m_randomness;
         }

      secure_vector<uint8_t>& randomness()
         {
         return m_randomness;
         }

      void set_randomness(const secure_vector<uint8_t>& randomness)
         {
         m_randomness = randomness;
         }

      void set_randomness(secure_vector<uint8_t>&& randomness)
         {
         m_randomness = std::move(randomness);
         }

      const XMSS_WOTS_PublicKey::TreeSignature& tree() const
         {
         return m_tree_sig;
         }

      XMSS_WOTS_PublicKey::TreeSignature& tree()
         {
         return m_tree_sig;
         }

      void set_tree(const XMSS_WOTS_PublicKey::TreeSignature& tree_sig)
         {
         m_tree_sig = tree_sig;
         }

      void set_tree(XMSS_WOTS_PublicKey::TreeSignature&& tree_sig)
         {
         m_tree_sig = std::move(tree_sig);
         }

      /**
       * Generates a serialized representation of XMSS Signature by
       * concatenating the following elements in order:
       * 8-byte leaf index, n-bytes randomness, ots_signature,
       * authentication path.
       *
       * n is the element_size(), len equal to len(), h the tree height
       * defined by the chosen XMSS signature method.
       *
       * @return serialized signature, a sequence of
       *         (len + h + 1)n bytes.
       **/
      secure_vector<uint8_t> bytes() const;

   private:
      uint64_t m_leaf_idx;
      secure_vector<uint8_t> m_randomness;
      XMSS_WOTS_PublicKey::TreeSignature m_tree_sig;
   };

}

namespace Botan {

/**
 * Signature generation operation for Extended Hash-Based Signatures (XMSS) as
 * defined in:
 *
 * [1] XMSS: Extended Hash-Based Signatures,
 *     draft-itrf-cfrg-xmss-hash-based-signatures-06
 *     Release: July 2016.
 *     https://datatracker.ietf.org/doc/
 *     draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
 **/
class XMSS_Signature_Operation final : public virtual PK_Ops::Signature,
                                       public XMSS_Common_Ops
   {
   public:
      XMSS_Signature_Operation(const XMSS_PrivateKey& private_key);

      /**
       * Creates an XMSS signature for the message provided through call to
       * update().
       *
       * @return serialized XMSS signature.
       **/
      secure_vector<uint8_t> sign(RandomNumberGenerator&) override;

      void update(const uint8_t msg[], size_t msg_len) override;

      size_t signature_length() const override;

   private:
      /**
       * Algorithm 11: "treeSig"
       * Generate a WOTS+ signature on a message with corresponding auth path.
       *
       * @param msg A message.
       * @param xmss_priv_key A XMSS private key.
       * @param adrs A XMSS Address.
       **/
      XMSS_WOTS_PublicKey::TreeSignature generate_tree_signature(
         const secure_vector<uint8_t>& msg,
         XMSS_PrivateKey& xmss_priv_key,
         XMSS_Address& adrs);

      /**
       * Algorithm 12: "XMSS_sign"
       * Generate an XMSS signature and update the XMSS secret key
       *
       * @param msg A message to sign of arbitrary length.
       * @param [out] xmss_priv_key A XMSS private key. The private key will be
       *              updated during the signing process.
       *
       * @return The signature of msg signed using xmss_priv_key.
       **/
      XMSS_Signature sign(
         const secure_vector<uint8_t>& msg,
         XMSS_PrivateKey& xmss_priv_key);

      wots_keysig_t build_auth_path(XMSS_PrivateKey& priv_key,
                                    XMSS_Address& adrs);

      void initialize();

      XMSS_PrivateKey m_priv_key;
      secure_vector<uint8_t> m_randomness;
      size_t m_leaf_idx;
      bool m_is_initialized;
   };

}

namespace Botan {

/**
 * Provides signature verification capabilities for Extended Hash-Based
 * Signatures (XMSS).
 **/
 class XMSS_Verification_Operation final : public virtual PK_Ops::Verification,
                                           public XMSS_Common_Ops
   {
   public:
      XMSS_Verification_Operation(
         const XMSS_PublicKey& public_key);

      bool is_valid_signature(const uint8_t sig[], size_t sig_len) override;

      void update(const uint8_t msg[], size_t msg_len) override;

   private:
      /**
       * Algorithm 13: "XMSS_rootFromSig"
       * Computes a root node using an XMSS signature, a message and a seed.
       *
       * @param msg A message.
       * @param sig The XMSS signature for msg.
       * @param ards A XMSS tree address.
       * @param seed A seed.
       *
       * @return An n-byte string holding the value of the root of a tree
       *         defined by the input parameters.
       **/
      secure_vector<uint8_t> root_from_signature(
         const XMSS_Signature& sig,
         const secure_vector<uint8_t>& msg,
         XMSS_Address& ards,
         const secure_vector<uint8_t>& seed);

      /**
       * Algorithm 14: "XMSS_verify"
       * Verifies a XMSS signature using the corresponding XMSS public key.
       *
       * @param sig A XMSS signature.
       * @param msg The message signed with sig.
       * @param pub_key the public key
       *
       * @return true if signature sig is valid for msg, false otherwise.
       **/
      bool verify(const XMSS_Signature& sig,
                  const secure_vector<uint8_t>& msg,
                  const XMSS_PublicKey& pub_key);

      XMSS_PublicKey m_pub_key;
      secure_vector<uint8_t> m_msg_buf;
   };

}

namespace Botan {

/**
 * Wrapper class to pair a XMSS_WOTS_PublicKey with an XMSS Address. Since
 * the PK_Ops::Verification interface does not allow an extra address
 * parameter to be passed to the sign(RandomNumberGenerator&), the address
 * needs to be stored together with the key and passed to the
 * XMSS_WOTS_Verification_Operation() on creation.
 **/
class XMSS_WOTS_Addressed_PublicKey : public virtual Public_Key
   {
   public:
      XMSS_WOTS_Addressed_PublicKey(const XMSS_WOTS_PublicKey& public_key)
         : m_pub_key(public_key), m_adrs() {}

      XMSS_WOTS_Addressed_PublicKey(const XMSS_WOTS_PublicKey& public_key,
                                    const XMSS_Address& adrs)
         : m_pub_key(public_key), m_adrs(adrs) {}

      XMSS_WOTS_Addressed_PublicKey(XMSS_WOTS_PublicKey&& public_key)
         : m_pub_key(std::move(public_key)), m_adrs() {}

      XMSS_WOTS_Addressed_PublicKey(XMSS_WOTS_PublicKey&& public_key,
                                    XMSS_Address&& adrs)
         : m_pub_key(std::move(public_key)), m_adrs(std::move(adrs)) {}

      const XMSS_WOTS_PublicKey& public_key() const { return m_pub_key; }
      XMSS_WOTS_PublicKey& public_key()  { return m_pub_key; }

      const XMSS_Address& address() const { return m_adrs; }
      XMSS_Address& address() { return m_adrs; }

      std::string algo_name() const override
         {
         return m_pub_key.algo_name();
         }

      AlgorithmIdentifier algorithm_identifier() const override
         {
         return m_pub_key.algorithm_identifier();
         }

      bool check_key(RandomNumberGenerator& rng, bool strong) const override
         {
         return m_pub_key.check_key(rng, strong);
         }

      std::unique_ptr<PK_Ops::Verification>
      create_verification_op(const std::string& params,
                             const std::string& provider) const override
         {
         return m_pub_key.create_verification_op(params, provider);
         }

      OID get_oid() const override
         {
         return m_pub_key.get_oid();
         }

      size_t estimated_strength() const override
         {
         return m_pub_key.estimated_strength();
         }

      size_t key_length() const override
         {
         return m_pub_key.estimated_strength();
         }

      std::vector<uint8_t> public_key_bits() const override
         {
         return m_pub_key.public_key_bits();
         }

   protected:
      XMSS_WOTS_PublicKey m_pub_key;
      XMSS_Address m_adrs;
   };

}

namespace Botan {

/**
 * Wrapper class to pair an XMSS_WOTS_PrivateKey with an XMSS Address. Since
 * the PK_Ops::Signature interface does not allow an extra address
 * parameter to be passed to the sign(RandomNumberGenerator&), the address
 * needs to be stored together with the key and passed to the
 * XMSS_WOTS_Signature_Operation() on creation.
 **/
class XMSS_WOTS_Addressed_PrivateKey final :
      public virtual XMSS_WOTS_Addressed_PublicKey,
      public virtual Private_Key
   {
   public:
      XMSS_WOTS_Addressed_PrivateKey(const XMSS_WOTS_PrivateKey& private_key)
         : XMSS_WOTS_Addressed_PublicKey(private_key),
           m_priv_key(private_key) {}

      XMSS_WOTS_Addressed_PrivateKey(const XMSS_WOTS_PrivateKey& private_key,
                                     const XMSS_Address& adrs)
         : XMSS_WOTS_Addressed_PublicKey(private_key, adrs),
           m_priv_key(private_key) {}

      XMSS_WOTS_Addressed_PrivateKey(XMSS_WOTS_PrivateKey&& private_key)
         : XMSS_WOTS_Addressed_PublicKey(XMSS_WOTS_PublicKey(private_key)),
           m_priv_key(std::move(private_key)) {}

      XMSS_WOTS_Addressed_PrivateKey(XMSS_WOTS_PrivateKey&& private_key,
                                     XMSS_Address&& adrs)
         : XMSS_WOTS_Addressed_PublicKey(XMSS_WOTS_PublicKey(private_key),
                                         std::move(adrs)),
           m_priv_key(std::move(private_key)) {}

      const XMSS_WOTS_PrivateKey& private_key() const { return m_priv_key; }
      XMSS_WOTS_PrivateKey& private_key() { return m_priv_key; }

      AlgorithmIdentifier
      pkcs8_algorithm_identifier() const override
         {
         return m_priv_key.pkcs8_algorithm_identifier();
         }

      secure_vector<uint8_t> private_key_bits() const override
         {
         return m_priv_key.private_key_bits();
         }

   private:
      XMSS_WOTS_PrivateKey m_priv_key;
   };

}

#endif // BOTAN_AMALGAMATION_INTERNAL_H_
