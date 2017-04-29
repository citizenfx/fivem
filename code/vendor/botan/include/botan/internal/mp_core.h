/*
* MPI Algorithms
* (C) 1999-2010 Jack Lloyd
*     2006 Luca Piccarreta
*     2016 Matthias Gierlings
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MP_CORE_OPS_H__
#define BOTAN_MP_CORE_OPS_H__

#include <botan/bigint.h>
#include <botan/mp_types.h>

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

#endif
