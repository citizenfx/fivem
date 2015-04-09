/*
* Lowest Level MPI Algorithms
* (C) 1999-2010 Jack Lloyd
*     2006 Luca Piccarreta
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MP_ASM_INTERNAL_H__
#define BOTAN_MP_ASM_INTERNAL_H__

#include <botan/internal/mp_madd.h>

namespace Botan {

extern "C" {

/*
* Word Addition
*/
inline word word_add(word x, word y, word* carry)
   {
   word z = x + y;
   word c1 = (z < x);
   z += *carry;
   *carry = c1 | (z < *carry);
   return z;
   }

/*
* Eight Word Block Addition, Two Argument
*/
inline word word8_add2(word x[8], const word y[8], word carry)
   {
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
   }

/*
* Eight Word Block Addition, Three Argument
*/
inline word word8_add3(word z[8], const word x[8], const word y[8], word carry)
   {
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
   }

/*
* Word Subtraction
*/
inline word word_sub(word x, word y, word* carry)
   {
   word t0 = x - y;
   word c1 = (t0 > x);
   word z = t0 - *carry;
   *carry = c1 | (z > t0);
   return z;
   }

/*
* Eight Word Block Subtraction, Two Argument
*/
inline word word8_sub2(word x[8], const word y[8], word carry)
   {
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
   }

/*
* Eight Word Block Subtraction, Two Argument
*/
inline word word8_sub2_rev(word x[8], const word y[8], word carry)
   {
   x[0] = word_sub(y[0], x[0], &carry);
   x[1] = word_sub(y[1], x[1], &carry);
   x[2] = word_sub(y[2], x[2], &carry);
   x[3] = word_sub(y[3], x[3], &carry);
   x[4] = word_sub(y[4], x[4], &carry);
   x[5] = word_sub(y[5], x[5], &carry);
   x[6] = word_sub(y[6], x[6], &carry);
   x[7] = word_sub(y[7], x[7], &carry);
   return carry;
   }


/*
* Eight Word Block Subtraction, Three Argument
*/
inline word word8_sub3(word z[8], const word x[8],
                       const word y[8], word carry)
   {
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
   }

/*
* Eight Word Block Linear Multiplication
*/
inline word word8_linmul2(word x[8], word y, word carry)
   {
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
   }

/*
* Eight Word Block Linear Multiplication
*/
inline word word8_muladd(word z[8], const word x[8],
                         word y, word carry)
   {
   __asm {
      mov esi,[x]
      mov ebx,[y]
      mov edi,[z]
      mov eax,[esi]     //load a
      mul ebx           //edx(hi):eax(lo)=a*b
      add eax,[carry]   //sum lo carry
      adc edx,0         //sum hi carry
      add eax,[edi]     //sum lo z
      adc edx,0         //sum hi z
      mov ecx,edx       //carry for next block = hi z
      mov [edi],eax     //save lo z

      mov eax,[esi+4]
      mul ebx
      add eax,ecx
      adc edx,0
      add eax,[edi+4]
      adc edx,0
      mov ecx,edx
      mov [edi+4],eax

      mov eax,[esi+8]
      mul ebx
      add eax,ecx
      adc edx,0
      add eax,[edi+8]
      adc edx,0
      mov ecx,edx
      mov [edi+8],eax

      mov eax,[esi+12]
      mul ebx
      add eax,ecx
      adc edx,0
      add eax,[edi+12]
      adc edx,0
      mov ecx,edx
      mov [edi+12],eax

      mov eax,[esi+16]
      mul ebx
      add eax,ecx
      adc edx,0
      add eax,[edi+16]
      adc edx,0
      mov ecx,edx
      mov [edi+16],eax

      mov eax,[esi+20]
      mul ebx
      add eax,ecx
      adc edx,0
      add eax,[edi+20]
      adc edx,0
      mov ecx,edx
      mov [edi+20],eax

      mov eax,[esi+24]
      mul ebx
      add eax,ecx
      adc edx,0
      add eax,[edi+24]
      adc edx,0
      mov ecx,edx
      mov [edi+24],eax

      mov eax,[esi+28]
      mul ebx
      add eax,ecx
      adc edx,0
      add eax,[edi+28]
      adc edx,0
      mov [edi+28],eax
      mov eax,edx
      }
   }

inline word word8_linmul3(word z[4], const word x[4], word y, word carry)
   {
   __asm {
#if 0
      //it's slower!!!
      mov edx,[z]
      mov eax,[x]
      movd mm7,[y]

      movd mm0,[eax]
      movd mm1,[eax+4]
      movd mm2,[eax+8]
      pmuludq mm0,mm7
      pmuludq mm1,mm7
      pmuludq mm2,mm7

      movd mm6,[carry]
      paddq mm0,mm6
      movd [edx],mm0

      psrlq mm0,32
      paddq mm1,mm0
      movd [edx+4],mm1

      movd mm3,[eax+12]
      psrlq mm1,32
      paddq mm2,mm1
      movd [edx+8],mm2

      pmuludq mm3,mm7
      movd mm4,[eax+16]
      psrlq mm2,32
      paddq mm3,mm2
      movd [edx+12],mm3

      pmuludq mm4,mm7
      movd mm5,[eax+20]
      psrlq mm3,32
      paddq mm4,mm3
      movd [edx+16],mm4

      pmuludq mm5,mm7
      movd mm0,[eax+24]
      psrlq mm4,32
      paddq mm5,mm4
      movd [edx+20],mm5

      pmuludq mm0,mm7
      movd mm1,[eax+28]
      psrlq mm5,32
      paddq mm0,mm5
      movd [edx+24],mm0

      pmuludq mm1,mm7
      psrlq mm0,32
      paddq mm1,mm0
      movd [edx+28],mm1
      psrlq mm1,32

      movd eax,mm1
      emms
#else
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
#endif
      }
   }

/*
* Eight Word Block Multiply/Add
*/
inline word word8_madd3(word z[8], const word x[8], word y, word carry)
   {
   z[0] = word_madd3(x[0], y, z[0], &carry);
   z[1] = word_madd3(x[1], y, z[1], &carry);
   z[2] = word_madd3(x[2], y, z[2], &carry);
   z[3] = word_madd3(x[3], y, z[3], &carry);
   z[4] = word_madd3(x[4], y, z[4], &carry);
   z[5] = word_madd3(x[5], y, z[5], &carry);
   z[6] = word_madd3(x[6], y, z[6], &carry);
   z[7] = word_madd3(x[7], y, z[7], &carry);
   return carry;
   }

/*
* Multiply-Add Accumulator
*/
inline void word3_muladd(word* w2, word* w1, word* w0, word a, word b)
   {
   word carry = *w0;
   *w0 = word_madd2(a, b, &carry);
   *w1 += carry;
   *w2 += (*w1 < carry) ? 1 : 0;
   }

/*
* Multiply-Add Accumulator
*/
inline void word3_muladd_2(word* w2, word* w1, word* w0, word a, word b)
   {
   word carry = 0;
   a = word_madd2(a, b, &carry);
   b = carry;

   word top = (b >> (BOTAN_MP_WORD_BITS-1));
   b <<= 1;
   b |= (a >> (BOTAN_MP_WORD_BITS-1));
   a <<= 1;

   carry = 0;
   *w0 = word_add(*w0, a, &carry);
   *w1 = word_add(*w1, b, &carry);
   *w2 = word_add(*w2, top, &carry);
   }

}

}

#endif
