/******************************************************************************
* Fast DXT - a realtime DXT compression tool
*
* Author : Luc Renambot
*
* Copyright (C) 2007 Electronic Visualization Laboratory,
* University of Illinois at Chicago
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either Version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
* License for more details.
*
* You should have received a copy of the GNU Lesser Public License along
* with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
*****************************************************************************/

// From:
//    Real-Time DXT Compression
//    May 20th 2006 J.M.P. van Waveren
//    (c) 2006, Id Software, Inc.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <math.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

#define C565_5_MASK 0xF8 // 0xFF minus last three bits
#define C565_6_MASK 0xFC // 0xFF minus last two bits

#define INSET_SHIFT 4 // inset the bounding box with ( range >> shift )

#if !defined(MAX_INT)
#define       MAX_INT         2147483647      /* max value for an int 32 */
#define       MIN_INT         (-2147483647-1) /* min value for an int 32 */
#endif


#if defined(__GNUC__)
#define   ALIGN16(_x)   _x __attribute((aligned(16)))
#else
#define   ALIGN16( x ) __declspec(align(16)) x
#endif


// Compress to DXT1 format
void CompressImageDXT1(const byte *inBuf, byte *outBuf, int width, int height, int &outputBytes);

// Compress to DXT5 format
void CompressImageDXT5(const byte *inBuf, byte *outBuf, int width, int height, int &outputBytes);

// Compress to DXT5 format, first convert to YCoCg color space
void CompressImageDXT5YCoCg(const byte *inBuf, byte *outBuf, int width, int height, int &outputBytes);

// Compute error between two images
double ComputeError(const byte *original, const byte *dxt, int width, int height);
