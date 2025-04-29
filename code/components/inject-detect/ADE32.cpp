//#include "r3dPCH.h"
#include "StdInc.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <algorithm>
#include "ADE32.h"

// ADE32 version 2.02c -- C edition
//#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

DWORD	ade32_table[512] =
{
	/* 00 */
	C_MODRM,

	/* 01 */
	C_MODRM,

	/* 02 */
	C_MODRM,

	/* 03 */
	C_MODRM,

	/* 04 */
	C_DATA1,

	/* 05 */
	C_DATA66,

	/* 06 */
	C_BAD,

	/* 07 */
	C_BAD,

	/* 08 */
	C_MODRM,

	/* 09 */
	C_MODRM,

	/* 0A */
	C_MODRM,

	/* 0B */
	C_MODRM,

	/* 0C */
	C_DATA1,

	/* 0D */
	C_DATA66,

	/* 0E */
	C_BAD,

	/* 0F */
	C_OPCODE2,

	/* 10 */
	C_MODRM + C_BAD,

	/* 11 */
	C_MODRM,

	/* 12 */
	C_MODRM + C_BAD,

	/* 13 */
	C_MODRM,

	/* 14 */
	C_DATA1 + C_BAD,

	/* 15 */
	C_DATA66 + C_BAD,

	/* 16 */
	C_BAD,

	/* 17 */
	C_BAD,

	/* 18 */
	C_MODRM + C_BAD,

	/* 19 */
	C_MODRM,

	/* 1A */
	C_MODRM,

	/* 1B */
	C_MODRM,

	/* 1C */
	C_DATA1 + C_BAD,

	/* 1D */
	C_DATA66 + C_BAD,

	/* 1E */
	C_BAD,

	/* 1F */
	C_BAD,

	/* 20 */
	C_MODRM,

	/* 21 */
	C_MODRM,

	/* 22 */
	C_MODRM,

	/* 23 */
	C_MODRM,

	/* 24 */
	C_DATA1,

	/* 25 */
	C_DATA66,

	/* 26 */
	C_SEG + C_BAD,

	/* 27 */
	C_BAD,

	/* 28 */
	C_MODRM,

	/* 29 */
	C_MODRM,

	/* 2A */
	C_MODRM,

	/* 2B */
	C_MODRM,

	/* 2C */
	C_DATA1,

	/* 2D */
	C_DATA66,

	/* 2E */
	C_SEG + C_BAD,

	/* 2F */
	C_BAD,

	/* 30 */
	C_MODRM,

	/* 31 */
	C_MODRM,

	/* 32 */
	C_MODRM,

	/* 33 */
	C_MODRM,

	/* 34 */
	C_DATA1,

	/* 35 */
	C_DATA66,

	/* 36 */
	C_SEG + C_BAD,

	/* 37 */
	C_BAD,

	/* 38 */
	C_MODRM,

	/* 39 */
	C_MODRM,

	/* 3A */
	C_MODRM,

	/* 3B */
	C_MODRM,

	/* 3C */
	C_DATA1,

	/* 3D */
	C_DATA66,

	/* 3E */
	C_SEG + C_BAD,

	/* 3F */
	C_BAD,

	/* 40 */
	0,

	/* 41 */
	0,

	/* 42 */
	0,

	/* 43 */
	0,

	/* 44 */
	C_BAD,

	/* 45 */
	0,

	/* 46 */
	0,

	/* 47 */
	0,

	/* 48 */
	0,

	/* 49 */
	0,

	/* 4A */
	0,

	/* 4B */
	0,

	/* 4C */
	C_BAD,

	/* 4D */
	0,

	/* 4E */
	0,

	/* 4F */
	0,

	/* 50 */
	0,

	/* 51 */
	0,

	/* 52 */
	0,

	/* 53 */
	0,

	/* 54 */
	0,

	/* 55 */
	0,

	/* 56 */
	0,

	/* 57 */
	0,

	/* 58 */
	0,

	/* 59 */
	0,

	/* 5A */
	0,

	/* 5B */
	0,

	/* 5C */
	C_BAD,

	/* 5D */
	0,

	/* 5E */
	0,

	/* 5F */
	0,

	/* 60 */
	C_BAD,

	/* 61 */
	C_BAD,

	/* 62 */
	C_MODRM + C_BAD,

	/* 63 */
	C_MODRM + C_BAD,

	/* 64 */
	C_SEG,

	/* 65 */
	C_SEG + C_BAD,

	/* 66 */
	C_66,

	/* 67 */
	C_67,

	/* 68 */
	C_DATA66,

	/* 69 */
	C_MODRM + C_DATA66,

	/* 6A */
	C_DATA1,

	/* 6B */
	C_MODRM + C_DATA1,

	/* 6C */
	C_BAD,

	/* 6D */
	C_BAD,

	/* 6E */
	C_BAD,

	/* 6F */
	C_BAD,

	/* 70 */
	C_DATA1 + C_REL + C_BAD,

	/* 71 */
	C_DATA1 + C_REL + C_BAD,

	/* 72 */
	C_DATA1 + C_REL,

	/* 73 */
	C_DATA1 + C_REL,

	/* 74 */
	C_DATA1 + C_REL,

	/* 75 */
	C_DATA1 + C_REL,

	/* 76 */
	C_DATA1 + C_REL,

	/* 77 */
	C_DATA1 + C_REL,

	/* 78 */
	C_DATA1 + C_REL,

	/* 79 */
	C_DATA1 + C_REL,

	/* 7A */
	C_DATA1 + C_REL + C_BAD,

	/* 7B */
	C_DATA1 + C_REL + C_BAD,

	/* 7C */
	C_DATA1 + C_REL,

	/* 7D */
	C_DATA1 + C_REL,

	/* 7E */
	C_DATA1 + C_REL,

	/* 7F */
	C_DATA1 + C_REL,

	/* 80 */
	C_MODRM + C_DATA1,

	/* 81 */
	C_MODRM + C_DATA66,

	/* 82 */
	C_MODRM + C_DATA1 + C_BAD,

	/* 83 */
	C_MODRM + C_DATA1,

	/* 84 */
	C_MODRM,

	/* 85 */
	C_MODRM,

	/* 86 */
	C_MODRM,

	/* 87 */
	C_MODRM,

	/* 88 */
	C_MODRM,

	/* 89 */
	C_MODRM,

	/* 8A */
	C_MODRM,

	/* 8B */
	C_MODRM,

	/* 8C */
	C_MODRM + C_BAD,

	/* 8D */
	C_MODRM,

	/* 8E */
	C_MODRM + C_BAD,

	/* 8F */
	C_MODRM,

	/* 90 */
	0,

	/* 91 */
	0,

	/* 92 */
	0,

	/* 93 */
	C_BAD,

	/* 94 */
	C_BAD,

	/* 95 */
	C_BAD,

	/* 96 */
	C_BAD,

	/* 97 */
	C_BAD,

	/* 98 */
	C_BAD,

	/* 99 */
	0,

	/* 9A */
	C_DATA66 + C_DATA2 + C_BAD,

	/* 9B */
	0,

	/* 9C */
	C_BAD,

	/* 9D */
	C_BAD,

	/* 9E */
	C_BAD,

	/* 9F */
	C_BAD,

	/* A0 */
	C_ADDR67,

	/* A1 */
	C_ADDR67,

	/* A2 */
	C_ADDR67,

	/* A3 */
	C_ADDR67,

	/* A4 */
	0,

	/* A5 */
	0,

	/* A6 */
	0,

	/* A7 */
	0,

	/* A8 */
	C_DATA1,

	/* A9 */
	C_DATA66,

	/* AA */
	0,

	/* AB */
	0,

	/* AC */
	0,

	/* AD */
	C_BAD,

	/* AE */
	0,

	/* AF */
	C_BAD,

	/* B0 */
	C_DATA1,

	/* B1 */
	C_DATA1,

	/* B2 */
	C_DATA1,

	/* B3 */
	C_DATA1,

	/* B4 */
	C_DATA1,

	/* B5 */
	C_DATA1,

	/* B6 */
	C_DATA1 + C_BAD,

	/* B7 */
	C_DATA1 + C_BAD,

	/* B8 */
	C_DATA66,

	/* B9 */
	C_DATA66,

	/* BA */
	C_DATA66,

	/* BB */
	C_DATA66,

	/* BC */
	C_DATA66 + C_BAD,

	/* BD */
	C_DATA66,

	/* BE */
	C_DATA66,

	/* BF */
	C_DATA66,

	/* C0 */
	C_MODRM + C_DATA1,

	/* C1 */
	C_MODRM + C_DATA1,

	/* C2 */
	C_DATA2 + C_STOP,

	/* C3 */
	C_STOP,

	/* C4 */
	C_MODRM + C_BAD,

	/* C5 */
	C_MODRM + C_BAD,

	/* C6 */
	C_MODRM + C_DATA1,

	/* C7 */
	C_MODRM + C_DATA66,

	/* C8 */
	C_DATA2 + C_DATA1,

	/* C9 */
	0,

	/* CA */
	C_DATA2 + C_STOP + C_BAD,

	/* CB */
	C_STOP + C_BAD,

	/* CC */
	C_BAD,

	/* CD */
	C_BAD,

	/* CE */
	C_BAD,

	/* CF */
	C_STOP + C_BAD,

	/* D0 */
	C_MODRM,

	/* D1 */
	C_MODRM,

	/* D2 */
	C_MODRM,

	/* D3 */
	C_MODRM,

	/* D4 */
	C_DATA1 + C_BAD,

	/* D5 */
	C_DATA1 + C_BAD,

	/* D6 */
	C_BAD,

	/* D7 */
	C_BAD,

	/* D8 */
	C_MODRM,

	/* D9 */
	C_MODRM,

	/* DA */
	C_MODRM,

	/* DB */
	C_MODRM,

	/* DC */
	C_MODRM,

	/* DD */
	C_MODRM,

	/* DE */
	C_MODRM,

	/* DF */
	C_MODRM,

	/* E0 */
	C_DATA1 + C_REL + C_BAD,

	/* E1 */
	C_DATA1 + C_REL + C_BAD,

	/* E2 */
	C_DATA1 + C_REL,

	/* E3 */
	C_DATA1 + C_REL,

	/* E4 */
	C_DATA1 + C_BAD,

	/* E5 */
	C_DATA1 + C_BAD,

	/* E6 */
	C_DATA1 + C_BAD,

	/* E7 */
	C_DATA1 + C_BAD,

	/* E8 */
	C_DATA66 + C_REL,

	/* E9 */
	C_DATA66 + C_REL + C_STOP,

	/* EA */
	C_DATA66 + C_DATA2 + C_BAD,

	/* EB */
	C_DATA1 + C_REL + C_STOP,

	/* EC */
	C_BAD,

	/* ED */
	C_BAD,

	/* EE */
	C_BAD,

	/* EF */
	C_BAD,

	/* F0 */
	C_LOCK + C_BAD,

	/* F1 */
	C_BAD,

	/* F2 */
	C_REP,

	/* F3 */
	C_REP,

	/* F4 */
	C_BAD,

	/* F5 */
	C_BAD,

	/* F6 */
	C_MODRM,

	/* F7 */
	C_MODRM,

	/* F8 */
	0,

	/* F9 */
	0,

	/* FA */
	C_BAD,

	/* FB */
	C_BAD,

	/* FC */
	0,

	/* FD */
	0,

	/* FE */
	C_MODRM,

	/* FF */
	C_MODRM,

	/* 00 */
	C_MODRM,

	/* 01 */
	C_MODRM,

	/* 02 */
	C_MODRM,

	/* 03 */
	C_MODRM,

	/* 04 */
	C_ERROR,

	/* 05 */
	C_ERROR,

	/* 06 */
	0,

	/* 07 */
	C_ERROR,

	/* 08 */
	0,

	/* 09 */
	0,

	/* 0A */
	0,

	/* 0B */
	0,

	/* 0C */
	C_ERROR,

	/* 0D */
	C_ERROR,

	/* 0E */
	C_ERROR,

	/* 0F */
	C_ERROR,

	/* 10 */
	C_ERROR,

	/* 11 */
	C_ERROR,

	/* 12 */
	C_ERROR,

	/* 13 */
	C_ERROR,

	/* 14 */
	C_ERROR,

	/* 15 */
	C_ERROR,

	/* 16 */
	C_ERROR,

	/* 17 */
	C_ERROR,

	/* 18 */
	C_ERROR,

	/* 19 */
	C_ERROR,

	/* 1A */
	C_ERROR,

	/* 1B */
	C_ERROR,

	/* 1C */
	C_ERROR,

	/* 1D */
	C_ERROR,

	/* 1E */
	C_ERROR,

	/* 1F */
	C_ERROR,

	/* 20 */
	C_ERROR,

	/* 21 */
	C_ERROR,

	/* 22 */
	C_ERROR,

	/* 23 */
	C_ERROR,

	/* 24 */
	C_ERROR,

	/* 25 */
	C_ERROR,

	/* 26 */
	C_ERROR,

	/* 27 */
	C_ERROR,

	/* 28 */
	C_ERROR,

	/* 29 */
	C_ERROR,

	/* 2A */
	C_ERROR,

	/* 2B */
	C_ERROR,

	/* 2C */
	C_ERROR,

	/* 2D */
	C_ERROR,

	/* 2E */
	C_ERROR,

	/* 2F */
	C_ERROR,

	/* 30 */
	C_ERROR,

	/* 31 */
	C_ERROR,

	/* 32 */
	C_ERROR,

	/* 33 */
	C_ERROR,

	/* 34 */
	C_ERROR,

	/* 35 */
	C_ERROR,

	/* 36 */
	C_ERROR,

	/* 37 */
	C_ERROR,

	/* 38 */
	C_ERROR,

	/* 39 */
	C_ERROR,

	/* 3A */
	C_ERROR,

	/* 3B */
	C_ERROR,

	/* 3C */
	C_ERROR,

	/* 3D */
	C_ERROR,

	/* 3E */
	C_ERROR,

	/* 3F */
	C_ERROR,

	/* 40 */
	C_ERROR,

	/* 41 */
	C_ERROR,

	/* 42 */
	C_ERROR,

	/* 43 */
	C_ERROR,

	/* 44 */
	C_ERROR,

	/* 45 */
	C_ERROR,

	/* 46 */
	C_ERROR,

	/* 47 */
	C_ERROR,

	/* 48 */
	C_ERROR,

	/* 49 */
	C_ERROR,

	/* 4A */
	C_ERROR,

	/* 4B */
	C_ERROR,

	/* 4C */
	C_ERROR,

	/* 4D */
	C_ERROR,

	/* 4E */
	C_ERROR,

	/* 4F */
	C_ERROR,

	/* 50 */
	C_ERROR,

	/* 51 */
	C_ERROR,

	/* 52 */
	C_ERROR,

	/* 53 */
	C_ERROR,

	/* 54 */
	C_ERROR,

	/* 55 */
	C_ERROR,

	/* 56 */
	C_ERROR,

	/* 57 */
	C_ERROR,

	/* 58 */
	C_ERROR,

	/* 59 */
	C_ERROR,

	/* 5A */
	C_ERROR,

	/* 5B */
	C_ERROR,

	/* 5C */
	C_ERROR,

	/* 5D */
	C_ERROR,

	/* 5E */
	C_ERROR,

	/* 5F */
	C_ERROR,

	/* 60 */
	C_ERROR,

	/* 61 */
	C_ERROR,

	/* 62 */
	C_ERROR,

	/* 63 */
	C_ERROR,

	/* 64 */
	C_ERROR,

	/* 65 */
	C_ERROR,

	/* 66 */
	C_ERROR,

	/* 67 */
	C_ERROR,

	/* 68 */
	C_ERROR,

	/* 69 */
	C_ERROR,

	/* 6A */
	C_ERROR,

	/* 6B */
	C_ERROR,

	/* 6C */
	C_ERROR,

	/* 6D */
	C_ERROR,

	/* 6E */
	C_ERROR,

	/* 6F */
	C_ERROR,

	/* 70 */
	C_ERROR,

	/* 71 */
	C_ERROR,

	/* 72 */
	C_ERROR,

	/* 73 */
	C_ERROR,

	/* 74 */
	C_ERROR,

	/* 75 */
	C_ERROR,

	/* 76 */
	C_ERROR,

	/* 77 */
	C_ERROR,

	/* 78 */
	C_ERROR,

	/* 79 */
	C_ERROR,

	/* 7A */
	C_ERROR,

	/* 7B */
	C_ERROR,

	/* 7C */
	C_ERROR,

	/* 7D */
	C_ERROR,

	/* 7E */
	C_ERROR,

	/* 7F */
	C_ERROR,

	/* 80 */
	C_DATA66 + C_REL,

	/* 81 */
	C_DATA66 + C_REL,

	/* 82 */
	C_DATA66 + C_REL,

	/* 83 */
	C_DATA66 + C_REL,

	/* 84 */
	C_DATA66 + C_REL,

	/* 85 */
	C_DATA66 + C_REL,

	/* 86 */
	C_DATA66 + C_REL,

	/* 87 */
	C_DATA66 + C_REL,

	/* 88 */
	C_DATA66 + C_REL,

	/* 89 */
	C_DATA66 + C_REL,

	/* 8A */
	C_DATA66 + C_REL,

	/* 8B */
	C_DATA66 + C_REL,

	/* 8C */
	C_DATA66 + C_REL,

	/* 8D */
	C_DATA66 + C_REL,

	/* 8E */
	C_DATA66 + C_REL,

	/* 8F */
	C_DATA66 + C_REL,

	/* 90 */
	C_MODRM,

	/* 91 */
	C_MODRM,

	/* 92 */
	C_MODRM,

	/* 93 */
	C_MODRM,

	/* 94 */
	C_MODRM,

	/* 95 */
	C_MODRM,

	/* 96 */
	C_MODRM,

	/* 97 */
	C_MODRM,

	/* 98 */
	C_MODRM,

	/* 99 */
	C_MODRM,

	/* 9A */
	C_MODRM,

	/* 9B */
	C_MODRM,

	/* 9C */
	C_MODRM,

	/* 9D */
	C_MODRM,

	/* 9E */
	C_MODRM,

	/* 9F */
	C_MODRM,

	/* A0 */
	0,

	/* A1 */
	0,

	/* A2 */
	0,

	/* A3 */
	C_MODRM,

	/* A4 */
	C_MODRM + C_DATA1,

	/* A5 */
	C_MODRM,

	/* A6 */
	C_ERROR,

	/* A7 */
	C_ERROR,

	/* A8 */
	0,

	/* A9 */
	0,

	/* AA */
	0,

	/* AB */
	C_MODRM,

	/* AC */
	C_MODRM + C_DATA1,

	/* AD */
	C_MODRM,

	/* AE */
	C_ERROR,

	/* AF */
	C_MODRM,

	/* B0 */
	C_MODRM,

	/* B1 */
	C_MODRM,

	/* B2 */
	C_MODRM,

	/* B3 */
	C_MODRM,

	/* B4 */
	C_MODRM,

	/* B5 */
	C_MODRM,

	/* B6 */
	C_MODRM,

	/* B7 */
	C_MODRM,

	/* B8 */
	C_ERROR,

	/* B9 */
	C_ERROR,

	/* BA */
	C_MODRM + C_DATA1,

	/* BB */
	C_MODRM,

	/* BC */
	C_MODRM,

	/* BD */
	C_MODRM,

	/* BE */
	C_MODRM,

	/* BF */
	C_MODRM,

	/* C0 */
	C_MODRM,

	/* C1 */
	C_MODRM,

	/* C2 */
	C_ERROR,

	/* C3 */
	C_ERROR,

	/* C4 */
	C_ERROR,

	/* C5 */
	C_ERROR,

	/* C6 */
	C_ERROR,

	/* C7 */
	C_ERROR,

	/* C8 */
	0,

	/* C9 */
	0,

	/* CA */
	0,

	/* CB */
	0,

	/* CC */
	0,

	/* CD */
	C_DATA1,

	/* CE */
	0,

	/* CF */
	0,

	/* D0 */
	C_ERROR,

	/* D1 */
	C_ERROR,

	/* D2 */
	C_ERROR,

	/* D3 */
	C_ERROR,

	/* D4 */
	C_ERROR,

	/* D5 */
	C_ERROR,

	/* D6 */
	C_ERROR,

	/* D7 */
	C_ERROR,

	/* D8 */
	C_ERROR,

	/* D9 */
	C_ERROR,

	/* DA */
	C_ERROR,

	/* DB */
	C_ERROR,

	/* DC */
	C_ERROR,

	/* DD */
	C_ERROR,

	/* DE */
	C_ERROR,

	/* DF */
	C_ERROR,

	/* E0 */
	C_ERROR,

	/* E1 */
	C_ERROR,

	/* E2 */
	C_ERROR,

	/* E3 */
	C_ERROR,

	/* E4 */
	C_ERROR,

	/* E5 */
	C_ERROR,

	/* E6 */
	C_ERROR,

	/* E7 */
	C_ERROR,

	/* E8 */
	C_ERROR,

	/* E9 */
	C_ERROR,

	/* EA */
	C_ERROR,

	/* EB */
	C_ERROR,

	/* EC */
	C_ERROR,

	/* ED */
	C_ERROR,

	/* EE */
	C_ERROR,

	/* EF */
	C_ERROR,

	/* F0 */
	C_ERROR,

	/* F1 */
	C_ERROR,

	/* F2 */
	C_ERROR,

	/* F3 */
	C_ERROR,

	/* F4 */
	C_ERROR,

	/* F5 */
	C_ERROR,

	/* F6 */
	C_ERROR,

	/* F7 */
	C_ERROR,

	/* F8 */
	C_ERROR,

	/* F9 */
	C_ERROR,

	/* FA */
	C_ERROR,

	/* FB */
	C_ERROR,

	/* FC */
	C_ERROR,

	/* FD */
	C_ERROR,

	/* FE */
	C_ERROR,

	/* FF */
	C_ERROR
};		// ade32_table[]
int disasm ( BYTE *opcode0, disasm_struct *diza )
{
	BYTE	*opcode = opcode0;

	memset( diza, 0x00, sizeof(disasm_struct) );
	diza->disasm_defdata = 4;
	diza->disasm_defaddr = 4;

	if ( *(WORD *)opcode == 0x0000 )
		return 0;
	if ( *(WORD *)opcode == 0xFFFF )
		return 0;

	DWORD	flag = 0;

repeat_prefix:
	BYTE	c = *opcode++;
	DWORD	t = ade32_table[c];

	if ( t & C_ANYPREFIX )
	{
		if ( flag & t )
			return 0;

		flag |= t;

		if ( t & C_67 )
			diza->disasm_defaddr ^= 2 ^ 4;
		else if ( t & C_66 )
			diza->disasm_defdata ^= 2 ^ 4;
		else if ( t & C_SEG )
			diza->disasm_seg = c;
		else if ( t & C_REP )
			diza->disasm_rep = c;

		// LOCK
		goto repeat_prefix;
	}	// C_ANYPREFIX

	flag |= t;
	diza->disasm_opcode = c;

	if ( c == 0x0F )
	{
		c = *opcode++;
		diza->disasm_opcode2 = c;
		flag |= ade32_table[256 + c];	// 2nd flagtable half
		if ( flag == C_ERROR )
			return 0;
	}
	else if ( c == 0xF7 )
	{
		if ( ((*opcode) & 0x38) == 0 )
			flag |= C_DATA66;
	}
	else if ( c == 0xF6 )
	{
		if ( ((*opcode) & 0x38) == 0 )
			flag |= C_DATA1;
	}
	else if ( c == 0xCD )
	{
		if ( *opcode == 0x20 )
			flag |= C_DATA4;
	}

	if ( flag & C_MODRM )
	{
		c = *opcode++;
		diza->disasm_modrm = c;

		if ( ((c & 0x38) == 0x20) && (diza->disasm_opcode == 0xFF) )
			flag |= C_STOP;

		BYTE	mod = c & 0xC0;
		BYTE	rm = c & 0x07;

		if ( mod != 0xC0 )
		{
			if ( diza->disasm_defaddr == 4 )
			{
				if ( rm == 4 )
				{
					flag |= C_SIB;
					c = *opcode++;
					diza->disasm_sib = c;
					rm = c & 0x07;
				}

				if ( mod == 0x40 )
					flag |= C_ADDR1;
				else if ( mod == 0x80 )
					flag |= C_ADDR4;
				else if ( rm == 5 )
					flag |= C_ADDR4;
			}
			else
			{	// MODRM 16-bit
				if ( mod == 0x40 )
					flag |= C_ADDR1;
				else if ( mod == 0x80 )
					flag |= C_ADDR2;
				else if ( rm == 6 )
					flag |= C_ADDR2;
			}
		}
	}			// C_MODRM

	diza->disasm_flag = flag;

	DWORD	a = flag & ( C_ADDR1 | C_ADDR2 | C_ADDR4 );
	DWORD	d = ( flag & (C_DATA1 | C_DATA2 | C_DATA4) ) >> 8;

	if ( flag & C_ADDR67 )
		a += diza->disasm_defaddr;
	if ( flag & C_DATA66 )
		d += diza->disasm_defdata;

	diza->disasm_addrsize = a;
	diza->disasm_datasize = d;

	DWORD	i;
	for ( i = 0; i < a; i++ )
		diza->disasm_addr_b[i] = *opcode++;

	for ( i = 0; i < d; i++ )
		diza->disasm_data_b[i] = *opcode++;

	diza->disasm_len = opcode - opcode0;

	return diza->disasm_len;
}	// disasm()

int oplen ( BYTE *opcode )
{
	disasm_struct	diza;
	memset( &diza, 0, sizeof(diza) );

	disasm( (BYTE *)opcode, &diza );

	if ( (diza.disasm_flag == C_ERROR)
	 ||	 ((diza.disasm_flag & C_STOP) == C_STOP)
	 ||	 ((diza.disasm_flag & C_REL) == C_REL)
	 ||	 ((diza.disasm_flag & C_BAD) == C_BAD) ) return -1;

	return diza.disasm_len;
}
