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

#include "StdInc.h"
#include <Error.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "log.h"
#include "conf.h"
#include "util.h"

#define STRSIZE 254

void Log_init(bool_t terminal)
{
}

void Log_free()
{
}

void Log_reset()
{
}

void logthis(const char *logstring, ...)
{
	va_list argp;
	char buf[STRSIZE + 1];

	va_start(argp, logstring);
	vsnprintf(&buf[0], STRSIZE, logstring, argp);
	va_end(argp);

	trace("%s\n", buf);
}

void Log_warn(const char *logstring, ...)
{
	va_list argp;
	char buf[STRSIZE + 1];
	int offset = 0;

	offset = sprintf(buf, "WARN: ");

	va_start(argp, logstring);
	vsnprintf(&buf[offset], STRSIZE - offset, logstring, argp);
	va_end(argp);

	trace("%s\n", buf);
}

void Log_info(const char *logstring, ...)
{
	va_list argp;
	char buf[STRSIZE + 1];
	int offset = 0;

	offset = sprintf(buf, "INFO: ");

	va_start(argp, logstring);
	vsnprintf(&buf[offset], STRSIZE - offset, logstring, argp);
	va_end(argp);

	trace("%s\n", buf);
}

void Log_info_client(client_t *client, const char *logstring, ...)
{
	va_list argp;
	char buf[STRSIZE + 1];
	int offset = 0;

	offset = sprintf(buf, "INFO: ");

	va_start(argp, logstring);
	offset += vsnprintf(&buf[offset], STRSIZE - offset, logstring, argp);
	va_end(argp);

	char *clientAddressString = Util_clientAddressToString(client);
	offset += snprintf(&buf[offset], STRSIZE - offset, " - [%d] %s@%s:%d",
		client->sessionId,
		client->username == NULL ? "" : client->username,
		clientAddressString,
		Util_clientAddressToPortTCP(client));
	free(clientAddressString);

	trace("%s\n", buf);
}

void Log_debug(const char *logstring, ...)
{
	return;

	va_list argp;
	char buf[STRSIZE + 1];
	int offset = 0;

	offset = sprintf(buf, "DEBUG: ");

	va_start(argp, logstring);
	vsnprintf(&buf[offset], STRSIZE - offset, logstring, argp);
	va_end(argp);
	
	trace("%s\n", buf);
}

void Log_fatal(const char *logstring, ...)
{
	va_list argp;
	char buf[STRSIZE + 1];
	int offset = 0;

	va_start(argp, logstring);
	vsnprintf(&buf[offset], STRSIZE - offset, logstring, argp);
	va_end(argp);

	FatalError("%s", buf);

	exit(1);
}
