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
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
#if (_POSIX_PRIORITY_SCHEDULING > 0)
#define POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#endif
#endif
#include "server.h"
#include "ssl.h"
#include "channel.h"
#include "log.h"
#include "client.h"
#include "conf.h"
#include "version.h"
#include "sharedmemory.h"
#include "ban.h"

char system_string[64], version_string[64];
int bindport;
int bindport6;
char *bindaddr;
char *bindaddr6;

void printhelp()
{
	printf("uMurmur version %s ('%s'). Mumble protocol %d.%d.%d\n", UMURMUR_VERSION,
		UMURMUR_CODENAME, PROTVER_MAJOR, PROTVER_MINOR, PROTVER_PATCH);
	printf("Usage: umurmurd [-d] [-r] [-h] [-p <pidfile>] [-t] [-c <conf file>] [-a <addr>] [-b <port>]\n");
	printf("       -d             - Do not daemonize - run in foreground.\n");
#ifdef POSIX_PRIORITY_SCHEDULING
	printf("       -r             - Run with realtime priority\n");
#endif
	printf("       -p <pidfile>   - Write PID to this file\n");
	printf("       -c <conf file> - Specify configuration file (default %s)\n", "n");
	printf("       -t             - Test config. Error message to stderr + non-zero exit code on error\n");
	printf("       -a <address>   - Bind to IP address\n");
	printf("       -A <address>   - Bind to IPv6 address\n");
	printf("       -b <port>      - Bind to port\n");
	printf("       -B <port>      - Bind to port (IPv6)\n");
	printf("       -h             - Print this help\n");
	exit(0);
}

#if 0
int main(int argc, char **argv)
{
	bool_t nodaemon = false;
#ifdef POSIX_PRIORITY_SCHEDULING
	bool_t realtime = false;
#endif
	bool_t testconfig = false;
	char *conffile = NULL, *pidfile = NULL;
	int c;
	struct utsname utsbuf;

	/* Arguments */
#ifdef POSIX_PRIORITY_SCHEDULING
	while ((c = getopt(argc, argv, "drp:c:a:A:b:B:ht")) != EOF) {
#else
		while ((c = getopt(argc, argv, "dp:c:a:A:b:B:ht")) != EOF) {
#endif
			switch(c) {
				case 'c':
					conffile = optarg;
					break;
				case 'p':
					pidfile = optarg;
					break;
				case 'a':
					bindaddr = optarg;
					break;
				case 'A':
					bindaddr6 = optarg;
					break;
				case 'b':
					bindport = atoi(optarg);
					break;
				case 'B':
					bindport6 = atoi(optarg);
					break;
				case 'd':
					nodaemon = true;
					break;
				case 'h':
					printhelp();
					break;
				case 't':
					testconfig = true;
					break;
#ifdef POSIX_PRIORITY_SCHEDULING
				case 'r':
					realtime = true;
					break;
#endif
				default:
					fprintf(stderr, "Unrecognized option\n");
					printhelp();
					break;
			}
		}

		if (testconfig) {
			if (!Conf_ok(conffile))
				exit(1);
			else
				exit(0);
		}

		/* Initialize the config subsystem early;
		 * switch_user() will need to read some config variables as well as logging.
		 */
		Conf_init(conffile);

		/* Logging to terminal if not daemonizing, otherwise to syslog or log file.
		*/
		if (!nodaemon) {
			daemonize();
			Log_init(false);
			if (pidfile != NULL)
				lockfile(pidfile);
#ifdef POSIX_PRIORITY_SCHEDULING
			/* Set the scheduling policy, has to be called after daemonizing
			 * but before we drop privileges */
			if (realtime)
				setscheduler();
#endif

		}
		else Log_init(true);

#ifdef POSIX_PRIORITY_SCHEDULING
		/* We still want to set scheduling policy if nodaemon is specified,
		 * but if we are daemonizing setscheduler() will be called above */
		if (nodaemon) {
			if (realtime)
				setscheduler();
		}
#endif

		signal(SIGCHLD, SIG_IGN); /* ignore child */
		signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
		signal(SIGTTOU, SIG_IGN);
		signal(SIGTTIN, SIG_IGN);
		signal(SIGPIPE, SIG_IGN);
		signal(SIGHUP, signal_handler); /* catch hangup signal */
		signal(SIGTERM, signal_handler); /* catch kill signal */

		/* Build system string */
		if (uname(&utsbuf) == 0) {
			snprintf(system_string, 64, "%s %s", utsbuf.sysname, utsbuf.machine);
			snprintf(version_string, 64, "%s", utsbuf.release);
		}
		else {
			snprintf(system_string, 64, "unknown unknown");
			snprintf(version_string, 64, "unknown");
		}

		/* Initializing */
		SSLi_init();
		Chan_init();
		Client_init();
		Ban_init();

#ifdef USE_SHAREDMEMORY_API
    Sharedmemory_init( bindport, bindport6 );
#endif

		if(!nodaemon) {
			/* SSL and scheduling is setup, we can drop privileges now */
			switch_user();

			/* Reopen log file. If user switch results in access denied, we catch
			 * it early.
			 */
			Log_reset();
		}

		Server_run();

#ifdef USE_SHAREDMEMORY_API
    Sharedmemory_deinit();
#endif

		Ban_deinit();
		SSLi_deinit();
		Chan_free();
		Log_free();
		Conf_deinit();

		if (pidfile != NULL)
			unlink(pidfile);

		return 0;
	}
}
#endif