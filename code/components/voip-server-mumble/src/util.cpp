/* Copyright (C) 2014, Felix Morgner <felix.morgner@gmail.com>

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
#include "util.h"
#include "memory.h"

char* Util_addressToString(struct sockaddr_storage *address)
{
	char* addressString = NULL;

	if (address->ss_family == AF_INET) {
		addressString = (char*)Memory_safeMalloc(1, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &((struct sockaddr_in *)address)->sin_addr, addressString, INET_ADDRSTRLEN);
	} else if(address->ss_family == AF_INET6) {
		addressString = (char*)Memory_safeMalloc(1, INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, &((struct sockaddr_in6 *)address)->sin6_addr, addressString, INET6_ADDRSTRLEN);
	}

	return addressString;
}

int Util_addressToPort(struct sockaddr_storage *address)
{
	int port = 0;

	if (address->ss_family == AF_INET) {
		port = ntohs(((struct sockaddr_in *)address)->sin_port);
	} else if(address->ss_family == AF_INET6) {
		port = ntohs(((struct sockaddr_in6 *)address)->sin6_port);
	}

	return port;
}

char* Util_clientAddressToString(client_t *client)
{
	return (char*)strdup(client->remote_tcp.ToString().c_str());
}

int Util_clientAddressToPortTCP(client_t *client)
{
	return client->remote_tcp.GetPort();
}

int Util_clientAddressToPortUDP(client_t *client)
{
	return client->remote_udp.GetPort();
}

