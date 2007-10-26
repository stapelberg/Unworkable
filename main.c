/* $Id: main.c,v 1.42 2007-10-26 06:19:53 niallo Exp $ */
/*
 * Copyright (c) 2006, 2007 Niall O'Higgins <niallo@unworkable.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <sys/time.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(USE_BOEHM_GC)
#include <gc.h>
#endif

#include "includes.h"

void usage(void);

extern char *optarg;
extern int  optind;

void
usage(void)
{
	fprintf(stderr, "unworkable: [-t tracefile] [-p port] torrent\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	int ch;
	u_int32_t i;
	struct torrent *torrent;
	struct rlimit rlp;
	struct timeval now;
#if 0
	struct torrent_piece *tpp;
	int j;
#endif

	#if defined(USE_BOEHM_GC)
	GC_INIT();
	#endif

	signal(SIGHUP, sighandler);
	signal(SIGABRT, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGQUIT, sighandler);
	/* don't die on sigpipe */
	signal(SIGPIPE, SIG_IGN);

	while ((ch = getopt(argc, argv, "t:p:")) != -1) {
		switch (ch) {
		case 't':
			unworkable_trace = xstrdup(optarg);
			break;
		case 'p':
			user_port = xstrdup(optarg);
			break;
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0)
		usage();


	if (getrlimit(RLIMIT_NOFILE, &rlp) == -1)
		err(1, "getrlimit");
	torrent = torrent_parse_file(argv[0]);
	/* a little extra info? torrent_print(torrent); */
	for (i = 0; i < torrent->num_pieces; i++) {
		torrent_piece_create(torrent, i);
		#if 0
		trace("checking");
		tpp = torrent_piece_find(torrent, i);
		torrent_piece_map(tpp);
		j = torrent_piece_checkhash(torrent, tpp);
		torrent_piece_unmap(tpp);
		if (j != 0) {
			printf("%u ", i);
			fflush(stdout);
		} else {
			torrent->good_pieces++;
		}
		#endif
	}

	if (gettimeofday(&now, NULL) == -1)
		err(1, "gettimeofday");
	srandom(now.tv_sec);
	network_init();
	network_start_torrent(torrent, rlp.rlim_cur);

	exit(0);
}
