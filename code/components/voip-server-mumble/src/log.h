/* Copyright (C) 2009-2014, Martin Johansson <martin@fatbob.nu>
   Copyright (C) 2005-2014, Thorvald Natvig <thorvald@natvig.com>

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the Developers nor the names of its contributors may
     be used to endorse or promote products derived from this software without
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef LOG_H_2435642356
#define LOG_H_2435642356

#include "types.h"
#include "client.h"

#define doAssert(_cond_) do {					\
		if (!(_cond_)) {												\
			logthis("Assertion failed in %s: Line: %d Function: %s", __FILE__, __LINE__, __FUNCTION__); \
			abort();													\
		}																\
	} while (0)


void logthis(const char *logstring, ...);

//#ifdef DEBUG
void Log_debug(const char *logstring, ...);
//#else
//#define Log_debug(_args_, ...) ((void)0)
//#endif

void Log_warn(const char *logstring, ...);
void Log_info(const char *logstring, ...);
void Log_info_client(client_t *client, const char *logstring, ...);
void Log_fatal(const char *logstring, ...);

void Log_init(bool_t terminal);
void Log_reset();
void Log_free();

#endif

