
#ifndef BOTAN_AMALGAMATION_INTERNAL_H__
#define BOTAN_AMALGAMATION_INTERNAL_H__

#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>


#if defined(BOTAN_TARGET_OS_HAS_THREADS)
#endif

namespace Botan {

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
// Barrier implements a barrier synchronization primitive. wait() will indicate
// how many threads to synchronize; each thread needing synchronization should
// call sync(). When sync() returns, the barrier is reset to zero, and the
// m_syncs counter is incremented. m_syncs is a counter to ensure that wait()
// can be called after a sync() even if the previously sleeping threads have
// not awoken.)
class Barrier
    {
    public:
        explicit Barrier(int value = 0) : m_value(value), m_syncs(0) {}

        void wait(unsigned delta);

        void sync();

    private:
        int m_value;
        unsigned m_syncs;
        mutex_type m_mutex;
        std::condition_variable m_cond;
    };
#endif

}


namespace Botan {

/**
* Power of 2 test. T should be an unsigned integer type
* @param arg an integer value
* @return true iff arg is 2^n for some n > 0
*/
template<typename T>
inline bool is_power_of_2(T arg)
   {
   return ((arg != 0 && arg != 1) && ((arg & (arg-1)) == 0));
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
   for(size_t i = 8*sizeof(T); i > 0; --i)
      if((n >> (i - 1)) & 0x01)
         return i;
   return 0;
   }

/**
* Return the index of the lowest set bit
* T is an unsigned integer type
* @param n an integer value
* @return index of the lowest set bit in n
*/
template<typename T>
inline size_t low_bit(T n)
   {
   for(size_t i = 0; i != 8*sizeof(T); ++i)
      if((n >> i) & 0x01)
         return (i + 1);
   return 0;
   }

/**
* Return the number of significant bytes in n
* @param n an integer value
* @return number of significant bytes in n
*/
template<typename T>
inline size_t significant_bytes(T n)
   {
   for(size_t i = 0; i != sizeof(T); ++i)
      if(get_byte(i, n))
         return sizeof(T)-i;
   return 0;
   }

/**
* Compute Hamming weights
* @param n an integer value
* @return number of bits in n set to 1
*/
template<typename T>
inline size_t hamming_weight(T n)
   {
   const uint8_t NIBBLE_WEIGHTS[] = {
      0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

   size_t weight = 0;
   for(size_t i = 0; i != 2*sizeof(T); ++i)
      weight += NIBBLE_WEIGHTS[(n >> (4*i)) & 0x0F];
   return weight;
   }

/**
* Count the trailing zero bits in n
* @param n an integer value
* @return maximum x st 2^x divides n
*/
template<typename T>
inline size_t ctz(T n)
   {
   for(size_t i = 0; i != 8*sizeof(T); ++i)
      if((n >> i) & 0x01)
         return i;
   return 8*sizeof(T);
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

void gcm_multiply_clmul(uint8_t x[16], const uint8_t H[16]);

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

/*
* T should be an unsigned machine integer type
* Expand to a mask used for other operations
* @param in an integer
* @return If n is zero, returns zero. Otherwise
* returns a T with all bits set for use as a mask with
* select.
*/
template<typename T>
inline T expand_mask(T x)
   {
   T r = x;
   // First fold r down to a single bit
   for(size_t i = 1; i != sizeof(T)*8; i *= 2)
      r |= r >> i;
   r &= 1;
   r = ~(r - 1);
   return r;
   }

template<typename T>
inline T select(T mask, T from0, T from1)
   {
   return (from0 & mask) | (from1 & ~mask);
   }

template<typename PredT, typename ValT>
inline ValT val_or_zero(PredT pred_val, ValT val)
   {
   return select(CT::expand_mask<ValT>(pred_val), val, static_cast<ValT>(0));
   }

template<typename T>
inline T is_zero(T x)
   {
   return ~expand_mask(x);
   }

template<typename T>
inline T is_equal(T x, T y)
   {
   return is_zero(x ^ y);
   }

template<typename T>
inline T is_less(T x, T y)
   {
   /*
   This expands to a constant time sequence with GCC 5.2.0 on x86-64
   but something more complicated may be needed for portable const time.
   */
   return expand_mask<T>(x < y);
   }

template<typename T>
inline T is_lte(T x, T y)
   {
   return expand_mask<T>(x <= y);
   }

template<typename T>
inline void conditional_copy_mem(T value,
                                 T* to,
                                 const T* from0,
                                 const T* from1,
                                 size_t elems)
   {
   const T mask = CT::expand_mask(value);

   for(size_t i = 0; i != elems; ++i)
      {
      to[i] = CT::select(mask, from0[i], from1[i]);
      }
   }

template<typename T>
inline void cond_zero_mem(T cond,
                          T* array,
                          size_t elems)
   {
   const T mask = CT::expand_mask(cond);
   const T zero(0);

   for(size_t i = 0; i != elems; ++i)
      {
      array[i] = CT::select(mask, zero, array[i]);
      }
   }

template<typename T>
inline T expand_top_bit(T a)
   {
   return expand_mask<T>(a >> (sizeof(T)*8-1));
   }

template<typename T>
inline T max(T a, T b)
   {
   const T a_larger = b - a; // negative if a is larger
   return select(expand_top_bit(a), a, b);
   }

template<typename T>
inline T min(T a, T b)
   {
   const T a_larger = b - a; // negative if a is larger
   return select(expand_top_bit(b), b, a);
   }

inline secure_vector<uint8_t> strip_leading_zeros(const uint8_t in[], size_t length)
   {
   size_t leading_zeros = 0;

   uint8_t only_zeros = 0xFF;

   for(size_t i = 0; i != length; ++i)
      {
      only_zeros &= CT::is_zero(in[i]);
      leading_zeros += CT::select<uint8_t>(only_zeros, 1, 0);
      }

   return secure_vector<uint8_t>(in + leading_zeros, in + length);
   }

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
class Fixed_Window_Exponentiator : public Modular_Exponentiator
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

/**
* Montgomery Exponentiator
*/
class Montgomery_Exponentiator : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&) override;
      void set_base(const BigInt&) override;
      BigInt execute() const override;

      Modular_Exponentiator* copy() const override
         { return new Montgomery_Exponentiator(*this); }

      Montgomery_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      BigInt m_exp, m_modulus, m_R_mod, m_R2_mod;
      Modular_Reducer m_reducer;
      word m_mod_prime;
      size_t m_mod_words, m_exp_bits, m_window_bits;
      Power_Mod::Usage_Hints m_hints;
      std::vector<BigInt> m_g;
   };

}


namespace Botan {

class donna128
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
         h += (l < x.l);
         h += x.h;
         return *this;
         }

      donna128& operator+=(uint64_t x)
         {
         l += x;
         h += (l < x);
         return *this;
         }

      uint64_t lo() const { return l; }
      uint64_t hi() const { return h; }
   private:
      uint64_t h = 0, l = 0;
   };

inline donna128 operator*(const donna128& x, uint64_t y)
   {
   BOTAN_ASSERT(x.hi() == 0, "High 64 bits of donna128 set to zero during multiply");

   uint64_t lo = 0, hi = 0;
   mul64x64_128(x.lo(), y, &lo, &hi);
   return donna128(lo, hi);
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
* Win32 CAPI Entropy Source
*/
class Win32_CAPI_EntropySource final : public Entropy_Source
   {
   public:
      std::string name() const override { return "win32_cryptoapi"; }

      size_t poll(RandomNumberGenerator& rng) override;

      /**
      * Win32_Capi_Entropysource Constructor
      * @param provs list of providers, separated by ':'
      */
      explicit Win32_CAPI_EntropySource(const std::string& provs = "");

      class CSP_Handle
         {
         public:
            virtual size_t gen_random(uint8_t out[], size_t n) const = 0;
         };
   private:
      std::vector<std::unique_ptr<CSP_Handle>> m_csp_provs;
   };

}


namespace Botan {

BOTAN_DLL std::vector<std::string> get_files_recursive(const std::string& dir);

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

#if (BOTAN_MP_WORD_BITS == 8)
  typedef uint16_t dword;
  #define BOTAN_HAS_MP_DWORD
#elif (BOTAN_MP_WORD_BITS == 16)
  typedef uint32_t dword;
  #define BOTAN_HAS_MP_DWORD
#elif (BOTAN_MP_WORD_BITS == 32)
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
  #error BOTAN_MP_WORD_BITS must be 8, 16, 32, or 64
#endif

#if defined(BOTAN_TARGET_ARCH_IS_X86_32) && (BOTAN_MP_WORD_BITS == 32)

  #if defined(BOTAN_USE_GCC_INLINE_ASM)
    #define BOTAN_MP_USE_X86_32_ASM
    #define ASM(x) x "\n\t"
  #elif defined(BOTAN_TARGET_COMPILER_IS_MSVC)
    #define BOTAN_MP_USE_X86_32_MSVC_ASM
  #endif

#elif defined(BOTAN_TARGET_ARCH_IS_X86_64) && (BOTAN_MP_WORD_BITS == 64) && (BOTAN_USE_GCC_INLINE_ASM)
  #define BOTAN_MP_USE_X86_64_ASM
  #define ASM(x) x "\n\t"
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
   asm(
      ASM("mull %[y]")

      ASM("addl %[x],%[w0]")
      ASM("adcl %[y],%[w1]")
      ASM("adcl $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [x]"a"(x), [y]"d"(y), "0"(*w0), "1"(*w1), "2"(*w2)
      : "cc");

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ASM("mulq %[y]")

      ASM("addq %[x],%[w0]")
      ASM("adcq %[y],%[w1]")
      ASM("adcq $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [x]"a"(x), [y]"d"(y), "0"(*w0), "1"(*w1), "2"(*w2)
      : "cc");

#else
   word carry = *w0;
   *w0 = word_madd2(x, y, &carry);
   *w1 += carry;
   *w2 += (*w1 < carry) ? 1 : 0;
#endif
   }

/*
* Multiply-Add Accumulator
* (w2,w1,w0) += 2 * x * y
*/
inline void word3_muladd_2(word* w2, word* w1, word* w0, word x, word y)
   {
#if defined(BOTAN_MP_USE_X86_32_ASM)
   asm(
      ASM("mull %[y]")

      ASM("addl %[x],%[w0]")
      ASM("adcl %[y],%[w1]")
      ASM("adcl $0,%[w2]")

      ASM("addl %[x],%[w0]")
      ASM("adcl %[y],%[w1]")
      ASM("adcl $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [x]"a"(x), [y]"d"(y), "0"(*w0), "1"(*w1), "2"(*w2)
      : "cc");

#elif defined(BOTAN_MP_USE_X86_64_ASM)

   asm(
      ASM("mulq %[y]")

      ASM("addq %[x],%[w0]")
      ASM("adcq %[y],%[w1]")
      ASM("adcq $0,%[w2]")

      ASM("addq %[x],%[w0]")
      ASM("adcq %[y],%[w1]")
      ASM("adcq $0,%[w2]")

      : [w0]"=r"(*w0), [w1]"=r"(*w1), [w2]"=r"(*w2)
      : [x]"a"(x), [y]"d"(y), "0"(*w0), "1"(*w1), "2"(*w2)
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

/*
* The size of the word type, in bits
*/
const size_t MP_WORD_BITS = BOTAN_MP_WORD_BITS;

/*
* If cond == 0, does nothing.
* If cond > 0, swaps x[0:size] with y[0:size]
* Runs in constant time
*/
BOTAN_DLL
void bigint_cnd_swap(word cnd, word x[], word y[], size_t size);

/*
* If cond > 0 adds x[0:size] to y[0:size] and returns carry
* Runs in constant time
*/
BOTAN_DLL
word bigint_cnd_add(word cnd, word x[], const word y[], size_t size);

/*
* If cond > 0 subs x[0:size] to y[0:size] and returns borrow
* Runs in constant time
*/
BOTAN_DLL
word bigint_cnd_sub(word cnd, word x[], const word y[], size_t size);

/*
* 2s complement absolute value
* If cond > 0 sets x to ~x + 1
* Runs in constant time
*/
BOTAN_DLL
void bigint_cnd_abs(word cnd, word x[], size_t size);

/**
* Two operand addition
* @param x the first operand (and output)
* @param x_size size of x
* @param y the second operand
* @param y_size size of y (must be >= x_size)
*/
void bigint_add2(word x[], size_t x_size,
                 const word y[], size_t y_size);

/**
* Three operand addition
*/
void bigint_add3(word z[],
                 const word x[], size_t x_size,
                 const word y[], size_t y_size);

/**
* Two operand addition with carry out
*/
word bigint_add2_nc(word x[], size_t x_size, const word y[], size_t y_size);

/**
* Three operand addition with carry out
*/
word bigint_add3_nc(word z[],
                    const word x[], size_t x_size,
                    const word y[], size_t y_size);

/**
* Two operand subtraction
*/
word bigint_sub2(word x[], size_t x_size,
                 const word y[], size_t y_size);

/**
* Two operand subtraction, x = y - x; assumes y >= x
*/
void bigint_sub2_rev(word x[], const word y[], size_t y_size);

/**
* Three operand subtraction
*/
word bigint_sub3(word z[],
                 const word x[], size_t x_size,
                 const word y[], size_t y_size);

/*
* Shift Operations
*/
void bigint_shl1(word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

void bigint_shr1(word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

void bigint_shl2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

void bigint_shr2(word y[], const word x[], size_t x_size,
                 size_t word_shift, size_t bit_shift);

/*
* Linear Multiply
*/
void bigint_linmul2(word x[], size_t x_size, word y);
void bigint_linmul3(word z[], const word x[], size_t x_size, word y);

/**
* Montgomery Reduction
* @param z integer to reduce, of size exactly 2*(p_size+1).
           Output is in the first p_size+1 words, higher
           words are set to zero.
* @param p modulus
* @param p_size size of p
* @param p_dash Montgomery value
* @param workspace array of at least 2*(p_size+1) words
*/
void bigint_monty_redc(word z[],
                       const word p[], size_t p_size,
                       word p_dash,
                       word workspace[]);

/*
* Montgomery Multiplication
*/
void bigint_monty_mul(BigInt& z, const BigInt& x, const BigInt& y,
                      const word p[], size_t p_size, word p_dash,
                      word workspace[]);

/*
* Montgomery Squaring
*/
void bigint_monty_sqr(BigInt& z, const BigInt& x,
                      const word p[], size_t p_size, word p_dash,
                      word workspace[]);

/**
* Compare x and y
*/
int32_t bigint_cmp(const word x[], size_t x_size,
                  const word y[], size_t y_size);

/**
* Compute ((n1<<bits) + n0) / d
*/
word bigint_divop(word n1, word n0, word d);

/**
* Compute ((n1<<bits) + n0) % d
*/
word bigint_modop(word n1, word n0, word d);

/*
* Comba Multiplication / Squaring
*/
void bigint_comba_mul4(word z[8], const word x[4], const word y[4]);
void bigint_comba_mul6(word z[12], const word x[6], const word y[6]);
void bigint_comba_mul8(word z[16], const word x[8], const word y[8]);
void bigint_comba_mul9(word z[18], const word x[9], const word y[9]);
void bigint_comba_mul16(word z[32], const word x[16], const word y[16]);

void bigint_comba_sqr4(word out[8], const word in[4]);
void bigint_comba_sqr6(word out[12], const word in[6]);
void bigint_comba_sqr8(word out[16], const word in[8]);
void bigint_comba_sqr9(word out[18], const word in[9]);
void bigint_comba_sqr16(word out[32], const word in[16]);

/*
* High Level Multiplication/Squaring Interfaces
*/
void bigint_mul(BigInt& z, const BigInt& x, const BigInt& y, word workspace[]);

void bigint_sqr(word z[], size_t z_size, word workspace[],
                const word x[], size_t x_size, size_t x_sw);

}


namespace Botan {

namespace OS {

/**
* Returns the OS assigned process ID, if available. Otherwise throws.
*/
uint32_t get_process_id();

/**
* Return the highest resolution clock available on the system.
*
* The epoch and update rate of this clock is arbitrary and depending
* on the hardware it may not tick at a constant rate.
*
* Returns the value of the hardware cycle counter, if available.
* On Windows calls QueryPerformanceCounter.
* Under GCC or Clang on supported platforms the hardware cycle counter is queried:
*  x86, PPC, Alpha, SPARC, IA-64, S/390x, and HP-PA
* On other platforms clock_gettime is used with some monotonic timer, if available.
* As a final fallback std::chrono::high_resolution_clock is used.
*/
uint64_t get_processor_timestamp();

/**
* Returns the value of the system clock with best resolution available,
* normalized to nanoseconds resolution.
*/
uint64_t get_system_timestamp_ns();

/*
* Returns the maximum amount of memory (in bytes) we could/should
* hyptothetically allocate. Reads "BOTAN_MLOCK_POOL_SIZE" from
* environment which can be set to zero.
*/
size_t get_memory_locking_limit();

/*
* Request so many bytes of page-aligned RAM locked into memory using
* mlock, VirtualLock, or similar. Returns null on failure. The memory
* returned is zeroed. Free it with free_locked_pages.
*/
void* allocate_locked_pages(size_t length);

/*
* Free memory allocated by allocate_locked_pages
*/
void free_locked_pages(void* ptr, size_t length);

}

}


namespace Botan {

/**
* Container of output buffers for Pipe
*/
class Output_Buffers
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
      ~Output_Buffers();
   private:
      class SecureQueue* get(Pipe::message_id) const;

      std::deque<SecureQueue*> m_buffers;
      Pipe::message_id m_offset;
   };

}


namespace Botan {

namespace PK_Ops {

class Encryption_with_EME : public Encryption
   {
   public:
      size_t max_input_bits() const override;

      secure_vector<uint8_t> encrypt(const uint8_t msg[], size_t msg_len,
                                  RandomNumberGenerator& rng) override;

      ~Encryption_with_EME();
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

      ~Decryption_with_EME();
   protected:
      explicit Decryption_with_EME(const std::string& eme);
   private:
      virtual size_t max_raw_input_bits() const = 0;
      virtual secure_vector<uint8_t> raw_decrypt(const uint8_t msg[], size_t len) = 0;
      std::unique_ptr<EME> m_eme;
   };

class Verification_with_EMSA : public Verification
   {
   public:
      ~Verification_with_EMSA();

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
      virtual secure_vector<uint8_t> message_prefix() const { throw Exception( "No prefix" ); }

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

      std::unique_ptr<EMSA> m_emsa;

   private:
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
      ~Signature_with_EMSA();

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
      virtual secure_vector<uint8_t> message_prefix() const { throw Exception( "No prefix" ); }

      std::unique_ptr<EMSA> m_emsa;
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
      ~Key_Agreement_with_KDF();
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
      ~KEM_Encryption_with_KDF();
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
      ~KEM_Decryption_with_KDF();
   private:
      std::unique_ptr<KDF> m_kdf;
   };

}

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
   BOTAN_ASSERT(align_to != 0, "align_to must not be 0");

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
inline T round_down(T n, T align_to)
   {
   if(align_to == 0)
      return n;

   return (n - (n % align_to));
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

class Integer_Overflow_Detected : public Exception
   {
   public:
      Integer_Overflow_Detected(const std::string& file, int line) :
         Exception("Integer overflow detected at " + file + ":" + std::to_string(line))
         {}
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


#if defined(BOTAN_TARGET_OS_HAS_THREADS)
#endif

namespace Botan {

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
class Semaphore
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
#endif

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
   } while(0);

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
   } while(0);

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
   } while(0);

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
   } while(0);

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
   } while(0);

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
   } while(0);

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
   } while(0);

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
   } while(0);

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
      } while(0);

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
      } while(0);

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
      } while(0);

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
      } while(0);

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
      } while(0);

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
      } while(0);

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
      } while(0);

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
      } while(0);


#if defined(BOTAN_TARGET_SUPPORTS_SSE2)
  #include <emmintrin.h>
  #define BOTAN_SIMD_USE_SSE2

#elif defined(BOTAN_TARGET_SUPPORTS_ALTIVEC)
  #include <altivec.h>
  #undef vector
  #undef bool
  #define BOTAN_SIMD_USE_ALTIVEC
#endif

// TODO: NEON support

namespace Botan {

/**
* This class is not a general purpose SIMD type, and only offers
* instructions needed for evaluation of specific crypto primitives.
* For example it does not currently have equality operators of any
* kind.
*/
class SIMD_4x32
   {
   public:

      SIMD_4x32() // zero initialized
         {
#if defined(BOTAN_SIMD_USE_SSE2) || defined(BOTAN_SIMD_USE_ALTIVEC)
         ::memset(&m_reg, 0, sizeof(m_reg));
#else
         ::memset(m_reg, 0, sizeof(m_reg));
#endif
         }

      explicit SIMD_4x32(const uint32_t B[4])
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_loadu_si128(reinterpret_cast<const __m128i*>(B));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = (__vector unsigned int){B[0], B[1], B[2], B[3]};
#else
         m_reg[0] = B[0];
         m_reg[1] = B[1];
         m_reg[2] = B[2];
         m_reg[3] = B[3];
#endif
         }

      SIMD_4x32(uint32_t B0, uint32_t B1, uint32_t B2, uint32_t B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_set_epi32(B0, B1, B2, B3);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = (__vector unsigned int){B0, B1, B2, B3};
#else
         m_reg[0] = B0;
         m_reg[1] = B1;
         m_reg[2] = B2;
         m_reg[3] = B3;
#endif
         }

      explicit SIMD_4x32(uint32_t B)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_set1_epi32(B);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = (__vector unsigned int){B, B, B, B};
#else
         m_reg[0] = B;
         m_reg[1] = B;
         m_reg[2] = B;
         m_reg[3] = B;
#endif
         }

      static SIMD_4x32 load_le(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(in)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const uint32_t* in_32 = static_cast<const uint32_t*>(in);

         __vector unsigned int R0 = vec_ld(0, in_32);
         __vector unsigned int R1 = vec_ld(12, in_32);

         __vector unsigned char perm = vec_lvsl(0, in_32);

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
         perm = vec_xor(perm, vec_splat_u8(3)); // bswap vector
#endif

         R0 = vec_perm(R0, R1, perm);

         return SIMD_4x32(R0);
#else
         SIMD_4x32 out;
         Botan::load_le(out.m_reg, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      static SIMD_4x32 load_be(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return load_le(in).bswap();
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const uint32_t* in_32 = static_cast<const uint32_t*>(in);

         __vector unsigned int R0 = vec_ld(0, in_32);
         __vector unsigned int R1 = vec_ld(12, in_32);

         __vector unsigned char perm = vec_lvsl(0, in_32);

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
         perm = vec_xor(perm, vec_splat_u8(3)); // bswap vector
#endif

         R0 = vec_perm(R0, R1, perm);

         return SIMD_4x32(R0);

#else
         SIMD_4x32 out;
         Botan::load_be(out.m_reg, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      void store_le(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         _mm_storeu_si128(reinterpret_cast<__m128i*>(out), m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         __vector unsigned char perm = vec_lvsl(0, static_cast<uint32_t*>(nullptr));

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
         perm = vec_xor(perm, vec_splat_u8(3)); // bswap vector
#endif

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;

         vec.V = vec_perm(m_reg, m_reg, perm);

         Botan::store_be(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);
#else
         Botan::store_le(out, m_reg[0], m_reg[1], m_reg[2], m_reg[3]);
#endif
         }

      void store_be(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         bswap().store_le(out);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;

         vec.V = m_reg;

         Botan::store_be(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);
#else
         Botan::store_be(out, m_reg[0], m_reg[1], m_reg[2], m_reg[3]);
#endif
         }

      void rotate_left(size_t rot)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_or_si128(_mm_slli_epi32(m_reg, static_cast<int>(rot)),
                              _mm_srli_epi32(m_reg, static_cast<int>(32-rot)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int r = static_cast<unsigned int>(rot);
         m_reg = vec_rl(m_reg, (__vector unsigned int){r, r, r, r});

#else
         m_reg[0] = Botan::rotate_left(m_reg[0], rot);
         m_reg[1] = Botan::rotate_left(m_reg[1], rot);
         m_reg[2] = Botan::rotate_left(m_reg[2], rot);
         m_reg[3] = Botan::rotate_left(m_reg[3], rot);
#endif
         }

      void rotate_right(size_t rot)
         {
         rotate_left(32 - rot);
         }

      void operator+=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_add_epi32(m_reg, other.m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_add(m_reg, other.m_reg);
#else
         m_reg[0] += other.m_reg[0];
         m_reg[1] += other.m_reg[1];
         m_reg[2] += other.m_reg[2];
         m_reg[3] += other.m_reg[3];
#endif
         }

      SIMD_4x32 operator+(const SIMD_4x32& other) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_add_epi32(m_reg, other.m_reg));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_add(m_reg, other.m_reg));
#else
         return SIMD_4x32(m_reg[0] + other.m_reg[0],
                          m_reg[1] + other.m_reg[1],
                          m_reg[2] + other.m_reg[2],
                          m_reg[3] + other.m_reg[3]);
#endif
         }

      void operator-=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_sub_epi32(m_reg, other.m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_sub(m_reg, other.m_reg);
#else
         m_reg[0] -= other.m_reg[0];
         m_reg[1] -= other.m_reg[1];
         m_reg[2] -= other.m_reg[2];
         m_reg[3] -= other.m_reg[3];
#endif
         }

      SIMD_4x32 operator-(const SIMD_4x32& other) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_sub_epi32(m_reg, other.m_reg));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_sub(m_reg, other.m_reg));
#else
         return SIMD_4x32(m_reg[0] - other.m_reg[0],
                          m_reg[1] - other.m_reg[1],
                          m_reg[2] - other.m_reg[2],
                          m_reg[3] - other.m_reg[3]);
#endif
         }

      void operator^=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_xor_si128(m_reg, other.m_reg);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_xor(m_reg, other.m_reg);
#else
         m_reg[0] ^= other.m_reg[0];
         m_reg[1] ^= other.m_reg[1];
         m_reg[2] ^= other.m_reg[2];
         m_reg[3] ^= other.m_reg[3];
#endif
         }

      SIMD_4x32 operator^(const SIMD_4x32& other) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_xor_si128(m_reg, other.m_reg));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_xor(m_reg, other.m_reg));
#else
         return SIMD_4x32(m_reg[0] ^ other.m_reg[0],
                          m_reg[1] ^ other.m_reg[1],
                          m_reg[2] ^ other.m_reg[2],
                          m_reg[3] ^ other.m_reg[3]);
#endif
         }

      void operator|=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_or_si128(m_reg, other.m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_or(m_reg, other.m_reg);
#else
         m_reg[0] |= other.m_reg[0];
         m_reg[1] |= other.m_reg[1];
         m_reg[2] |= other.m_reg[2];
         m_reg[3] |= other.m_reg[3];
#endif
         }

      SIMD_4x32 operator&(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_and_si128(m_reg, other.m_reg));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_and(m_reg, other.m_reg));
#else
         return SIMD_4x32(m_reg[0] & other.m_reg[0],
                          m_reg[1] & other.m_reg[1],
                          m_reg[2] & other.m_reg[2],
                          m_reg[3] & other.m_reg[3]);
#endif
         }

      void operator&=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_and_si128(m_reg, other.m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_and(m_reg, other.m_reg);
#else
         m_reg[0] &= other.m_reg[0];
         m_reg[1] &= other.m_reg[1];
         m_reg[2] &= other.m_reg[2];
         m_reg[3] &= other.m_reg[3];
#endif
         }

      SIMD_4x32 operator<<(size_t shift) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_slli_epi32(m_reg, static_cast<int>(shift)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(shift);
         return SIMD_4x32(vec_sl(m_reg, (__vector unsigned int){s, s, s, s}));
#else
         return SIMD_4x32(m_reg[0] << shift,
                          m_reg[1] << shift,
                          m_reg[2] << shift,
                          m_reg[3] << shift);
#endif
         }

      SIMD_4x32 operator>>(size_t shift) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_srli_epi32(m_reg, static_cast<int>(shift)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(shift);
         return SIMD_4x32(vec_sr(m_reg, (__vector unsigned int){s, s, s, s}));
#else
         return SIMD_4x32(m_reg[0] >> shift,
                          m_reg[1] >> shift,
                          m_reg[2] >> shift,
                          m_reg[3] >> shift);

#endif
         }

      SIMD_4x32 operator~() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_xor_si128(m_reg, _mm_set1_epi32(0xFFFFFFFF)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_nor(m_reg, m_reg));
#else
         return SIMD_4x32(~m_reg[0],
                          ~m_reg[1],
                          ~m_reg[2],
                          ~m_reg[3]);
#endif
         }

      // (~reg) & other
      SIMD_4x32 andc(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_andnot_si128(m_reg, other.m_reg));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         /*
         AltiVec does arg1 & ~arg2 rather than SSE's ~arg1 & arg2
         so swap the arguments
         */
         return SIMD_4x32(vec_andc(other.m_reg, m_reg));
#else
         return SIMD_4x32((~m_reg[0]) & other.m_reg[0],
                          (~m_reg[1]) & other.m_reg[1],
                          (~m_reg[2]) & other.m_reg[2],
                          (~m_reg[3]) & other.m_reg[3]);
#endif
         }

      SIMD_4x32 bswap() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         __m128i T = m_reg;

         T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
         T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));

         return SIMD_4x32(_mm_or_si128(_mm_srli_epi16(T, 8),
                                       _mm_slli_epi16(T, 8)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         __vector unsigned char perm = vec_lvsl(0, static_cast<uint32_t*>(nullptr));

         perm = vec_xor(perm, vec_splat_u8(3));

         return SIMD_4x32(vec_perm(m_reg, m_reg, perm));
#else
         return SIMD_4x32(reverse_bytes(m_reg[0]),
                          reverse_bytes(m_reg[1]),
                          reverse_bytes(m_reg[2]),
                          reverse_bytes(m_reg[3]));
#endif
         }

      static void transpose(SIMD_4x32& B0, SIMD_4x32& B1,
                            SIMD_4x32& B2, SIMD_4x32& B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         __m128i T0 = _mm_unpacklo_epi32(B0.m_reg, B1.m_reg);
         __m128i T1 = _mm_unpacklo_epi32(B2.m_reg, B3.m_reg);
         __m128i T2 = _mm_unpackhi_epi32(B0.m_reg, B1.m_reg);
         __m128i T3 = _mm_unpackhi_epi32(B2.m_reg, B3.m_reg);
         B0.m_reg = _mm_unpacklo_epi64(T0, T1);
         B1.m_reg = _mm_unpackhi_epi64(T0, T1);
         B2.m_reg = _mm_unpacklo_epi64(T2, T3);
         B3.m_reg = _mm_unpackhi_epi64(T2, T3);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         __vector unsigned int T0 = vec_mergeh(B0.m_reg, B2.m_reg);
         __vector unsigned int T1 = vec_mergel(B0.m_reg, B2.m_reg);
         __vector unsigned int T2 = vec_mergeh(B1.m_reg, B3.m_reg);
         __vector unsigned int T3 = vec_mergel(B1.m_reg, B3.m_reg);

         B0.m_reg = vec_mergeh(T0, T2);
         B1.m_reg = vec_mergel(T0, T2);
         B2.m_reg = vec_mergeh(T1, T3);
         B3.m_reg = vec_mergel(T1, T3);
#else
         SIMD_4x32 T0(B0.m_reg[0], B1.m_reg[0], B2.m_reg[0], B3.m_reg[0]);
         SIMD_4x32 T1(B0.m_reg[1], B1.m_reg[1], B2.m_reg[1], B3.m_reg[1]);
         SIMD_4x32 T2(B0.m_reg[2], B1.m_reg[2], B2.m_reg[2], B3.m_reg[2]);
         SIMD_4x32 T3(B0.m_reg[3], B1.m_reg[3], B2.m_reg[3], B3.m_reg[3]);

         B0 = T0;
         B1 = T1;
         B2 = T2;
         B3 = T3;
#endif
         }

   private:
#if defined(BOTAN_SIMD_USE_SSE2)
      explicit SIMD_4x32(__m128i in) { m_reg = in; }
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      explicit SIMD_4x32(__vector unsigned int input) { m_reg = input; }
#endif

#if defined(BOTAN_SIMD_USE_SSE2)
      __m128i m_reg;
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      __vector unsigned int m_reg;
#else
      uint32_t m_reg[4];
#endif
   };

typedef SIMD_4x32 SIMD_32;

}


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
#if defined(BOTAN_BUILD_COMPILER_IS_SUN_STUDIO)
   // Work around a strange bug in Sun Studio
   multimap.insert(std::make_pair<const K, V>(key, value));
#else
   multimap.insert(std::make_pair(key, value));
#endif
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

namespace TLS {

/**
* TLS CBC+HMAC AEAD base class (GenericBlockCipher in TLS spec)
* This is the weird TLS-specific mode, not for general consumption.
*/
class BOTAN_DLL TLS_CBC_HMAC_AEAD_Mode : public AEAD_Mode
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
      TLS_CBC_HMAC_AEAD_Mode(const std::string& cipher_name,
                             size_t cipher_keylen,
                             const std::string& mac_name,
                             size_t mac_keylen,
                             bool use_explicit_iv,
                             bool use_encrypt_then_mac);

      size_t cipher_keylen() const { return m_cipher_keylen; }
      size_t mac_keylen() const { return m_mac_keylen; }
      size_t iv_size() const { return m_iv_size; }
      size_t block_size() const { return m_block_size; }

      bool use_encrypt_then_mac() const { return m_use_encrypt_then_mac; }

      BlockCipher& cipher() const
         {
         BOTAN_ASSERT_NONNULL(m_cipher);
         return *m_cipher;
         }

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

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<MessageAuthenticationCode> m_mac;

      secure_vector<uint8_t> m_cbc_state;
      std::vector<uint8_t> m_ad;
      secure_vector<uint8_t> m_msg;
   };

/**
* TLS_CBC_HMAC_AEAD Encryption
*/
class BOTAN_DLL TLS_CBC_HMAC_AEAD_Encryption final : public TLS_CBC_HMAC_AEAD_Mode
   {
   public:
      /**
      */
      TLS_CBC_HMAC_AEAD_Encryption(const std::string& cipher_algo,
                                   const size_t cipher_keylen,
                                   const std::string& mac_algo,
                                   const size_t mac_keylen,
                                   bool use_explicit_iv,
                                   bool use_encrypt_then_mac) :
         TLS_CBC_HMAC_AEAD_Mode(cipher_algo,
                                cipher_keylen,
                                mac_algo,
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
class BOTAN_DLL TLS_CBC_HMAC_AEAD_Decryption final : public TLS_CBC_HMAC_AEAD_Mode
   {
   public:
      /**
      */
      TLS_CBC_HMAC_AEAD_Decryption(const std::string& cipher_algo,
                                   const size_t cipher_keylen,
                                   const std::string& mac_algo,
                                   const size_t mac_keylen,
                                   bool use_explicit_iv,
                                   bool use_encrypt_then_mac) :
         TLS_CBC_HMAC_AEAD_Mode(cipher_algo,
                                cipher_keylen,
                                mac_algo,
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

}

}


namespace Botan {

namespace TLS {

/**
* TLS Handshake Hash
*/
class Handshake_Hash
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

      Handshake_IO() {}

      Handshake_IO(const Handshake_IO&) = delete;

      Handshake_IO& operator=(const Handshake_IO&) = delete;

      virtual ~Handshake_IO() {}
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

      class Handshake_Reassembly
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

      struct Message_Info
         {
         Message_Info(uint16_t e, Handshake_Type mt, const std::vector<uint8_t>& msg) :
            epoch(e), msg_type(mt), msg_bits(msg) {}

         Message_Info(const Message_Info& other) = default;

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
class Session_Keys
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

      Session_Keys() {}

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

      virtual ~Handshake_State();

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
                          const std::string& hash_algo,
                          const std::string& sig_algo,
                          bool for_client_auth,
                          const Policy& policy) const;

      std::pair<std::string, Signature_Format>
         choose_sig_format(const Private_Key& key,
                           std::string& hash_algo,
                           std::string& sig_algo,
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
class TLS_Data_Reader
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

         return std::string(reinterpret_cast<char*>(v.data()), v.size());
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
                           reinterpret_cast<const uint8_t*>(str.data()),
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
class Connection_Cipher_State
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
      bool cbc_nonce() const { return m_cbc_nonce; }

      std::chrono::seconds age() const
         {
         return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - m_start_time);
         }

   private:
      std::chrono::system_clock::time_point m_start_time;
      std::unique_ptr<AEAD_Mode> m_aead;

      std::vector<uint8_t> m_nonce;
      size_t m_nonce_bytes_from_handshake;
      size_t m_nonce_bytes_from_record;
      bool m_cbc_nonce;
   };

class Record
   {
   public:
      Record(secure_vector<uint8_t>& data,
             uint64_t* sequence,
             Protocol_Version* protocol_version,
             Record_Type* type)
         : m_data(data), m_sequence(sequence), m_protocol_version(protocol_version),
           m_type(type), m_size(data.size()) {};

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

class Record_Message
   {
   public:
      Record_Message(const uint8_t* data, size_t size)
         : m_type(0), m_sequence(0), m_data(data), m_size(size) {};
      Record_Message(uint8_t type, uint64_t sequence, const uint8_t* data, size_t size)
         : m_type(type), m_sequence(sequence), m_data(data),
           m_size(size) {};

      uint8_t& get_type() { return m_type; };
      uint64_t& get_sequence() { return m_sequence; };
      const uint8_t* get_data() { return m_data; };
      size_t& get_size() { return m_size; };

   private:
      uint8_t m_type;
      uint64_t m_sequence;
      const uint8_t* m_data;
      size_t m_size;
};

class Record_Raw_Input
   {
   public:
      Record_Raw_Input(const uint8_t* data, size_t size, size_t& consumed,
                       bool is_datagram)
         : m_data(data), m_size(size), m_consumed(consumed),
           m_is_datagram(is_datagram) {};

      const uint8_t*& get_data() { return m_data; };

      size_t& get_size() { return m_size; };

      size_t& get_consumed() { return m_consumed; };
      void set_consumed(size_t consumed) { m_consumed = consumed; }

      bool is_datagram() { return m_is_datagram; };

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
      virtual ~Connection_Sequence_Numbers() {}

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
      void new_read_cipher_state() override { m_read_seq_no = 0; m_read_epoch += 1; }
      void new_write_cipher_state() override { m_write_seq_no = 0; m_write_epoch += 1; }

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

      void new_read_cipher_state() override { m_read_epoch += 1; }

      void new_write_cipher_state() override
         {
         m_write_epoch += 1;
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
         throw Exception("DTLS uses explicit sequence numbers");
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
            const size_t offset = sequence - m_window_highest;
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

class XMSS_Signature
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
           m_tree_sig(tree_sig) {};

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
           m_tree_sig(std::move(tree_sig)) {};

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
      size_t m_leaf_idx;
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
class XMSS_Signature_Operation : public virtual PK_Ops::Signature,
                                 public XMSS_Common_Ops
   {
   public:
      XMSS_Signature_Operation(const XMSS_PrivateKey& private_key);
      virtual ~XMSS_Signature_Operation() {}

      /**
       * Creates an XMSS signature for the message provided through call to
       * update().
       *
       * @return serialized XMSS signature.
       **/
      secure_vector<uint8_t> sign(RandomNumberGenerator&) override;

      void update(const uint8_t msg[], size_t msg_len) override;

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
 class XMSS_Verification_Operation
   : public virtual PK_Ops::Verification,
     public XMSS_Common_Ops
   {
   public:
      XMSS_Verification_Operation(
         const XMSS_PublicKey& public_key);

      virtual ~XMSS_Verification_Operation() {}

      virtual bool is_valid_signature(const uint8_t sig[],
                                      size_t sig_len) override;

      void update(const uint8_t msg[], size_t msg_len) override;

   private:
      /**
       * Algorithm 13: "XMSS_rootFromSig"
       * Computes a root node using an XMSS signature, a message and a seed.
       *
       * @param msg A message.
       * @param sig The XMSS signature for msg.
       * @param adrs A XMSS tree address.
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
       * @paeam pub_key
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

      virtual std::string algo_name() const override
         {
         return m_pub_key.algo_name();
         }

      virtual AlgorithmIdentifier algorithm_identifier() const override
         {
         return m_pub_key.algorithm_identifier();
         }

      virtual bool check_key(RandomNumberGenerator& rng,
                             bool strong) const override
         {
         return m_pub_key.check_key(rng, strong);
         }

      virtual std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string& params,
                                const std::string& provider) const override
         {
         return m_pub_key.create_verification_op(params, provider);
         }

      virtual OID get_oid() const override
         {
         return m_pub_key.get_oid();
         }

      virtual size_t estimated_strength() const override
         {
         return m_pub_key.estimated_strength();
         }

      virtual size_t key_length() const override
         {
         return m_pub_key.estimated_strength();
         }

      virtual std::vector<uint8_t> public_key_bits() const override
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
class XMSS_WOTS_Addressed_PrivateKey
   : public virtual XMSS_WOTS_Addressed_PublicKey,
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

      virtual AlgorithmIdentifier
      pkcs8_algorithm_identifier() const override
         {
         return m_priv_key.pkcs8_algorithm_identifier();
         }

      virtual secure_vector<uint8_t> private_key_bits() const override
         {
         return m_priv_key.private_key_bits();
         }

   private:
      XMSS_WOTS_PrivateKey m_priv_key;
   };

}


namespace Botan {

/**
 * Operations shared by XMSS WOTS signature generation and verification
 * operations.
 **/
class XMSS_WOTS_Common_Ops
   {
   public:
      XMSS_WOTS_Common_Ops(XMSS_WOTS_Parameters::ots_algorithm_t oid)
         : m_wots_params(oid), m_hash(m_wots_params.hash_function_name()) {}


   protected:
      /**
       * Algorithm 2: Chaining Function.
       *
       * @param[out] result Contains the n-byte input string "x" upon call to chain(),
       *               that will be replaced with the value obtained by iterating
       *               the cryptographic hash function "F" steps times on the
       *               input x using the outputs of the PRNG "G".
       * @param[in] start_idx The start index.
       * @param[in] steps A number of steps.
       * @param[in] adrs An OTS Hash Address.
       * @param[in] seed A Seed.
       **/
      void chain(secure_vector<uint8_t>& result,
                 size_t start_idx,
                 size_t steps,
                 XMSS_Address& adrs,
                 const secure_vector<uint8_t>& seed);

      XMSS_WOTS_Parameters m_wots_params;
      XMSS_Hash m_hash;
   };

}


namespace Botan {

/**
 * Signature generation operation for Winternitz One Time Signatures for use
 * in Extended Hash-Based Signatures (XMSS).
 *
 * This operation is not intended for stand-alone use and thus not registered
 * in the Botan algorithm registry.
 ***/
class XMSS_WOTS_Signature_Operation : public virtual PK_Ops::Signature,
                                      public XMSS_WOTS_Common_Ops
   {
   public:
      XMSS_WOTS_Signature_Operation(
         const XMSS_WOTS_Addressed_PrivateKey& private_key);

      virtual ~XMSS_WOTS_Signature_Operation() {}

      /**
       * Creates a XMSS WOTS signature for the message provided through call
       * to update(). XMSS wots only supports one message part and a fixed
       * message size of "n" bytes where "n" equals the element size of
       * the chosen XMSS WOTS signature method. The random number generator
       * argument is supplied for interface compatibility and remains unused.
       *
       * @return serialized Winternitz One Time Signature.
       **/
      secure_vector<uint8_t> sign(RandomNumberGenerator&) override;

      void update(const uint8_t msg[], size_t msg_len) override;

   private:
      wots_keysig_t sign(const secure_vector<uint8_t>& msg,
                         const wots_keysig_t& priv_key,
                         XMSS_Address& adrs,
                         const secure_vector<uint8_t>& seed);
      XMSS_WOTS_Addressed_PrivateKey m_priv_key;
      secure_vector<uint8_t> m_msg_buf;
   };

}


namespace Botan {

/**
 * Provides signature verification capabilities for Winternitz One Time
 * Signatures used in Extended Merkle Tree Signatures (XMSS).
 *
 * This operation is not intended for stand-alone use and thus not registered
 * in the Botan algorithm registry.
 **/
class XMSS_WOTS_Verification_Operation
   : public virtual PK_Ops::Verification,
     public XMSS_WOTS_Common_Ops
   {
   public:
      XMSS_WOTS_Verification_Operation(
         const XMSS_WOTS_Addressed_PublicKey& public_key);

      virtual ~XMSS_WOTS_Verification_Operation() {}

      virtual bool is_valid_signature(const uint8_t sig[],
                                      size_t sig_len) override;

      void update(const uint8_t msg[], size_t msg_len) override;

   private:
      XMSS_WOTS_Addressed_PublicKey m_pub_key;
      secure_vector<uint8_t> m_msg_buf;
   };

}


#endif
