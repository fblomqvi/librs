/*
 * rs_tests.c
 * Copyright (C) 2019 Ferdinand Blomqvist
 *
 * This file is part of librs.
 *
 * Based on previous work by Phil Karn, KA9Q.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "librs.h"
#include "test_codes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <time.h>

enum verbosity {
	V_SILENT,
	V_PROGRESS,
	V_CSUMMARY
};

static int ewsc = 1;
static int v = 1;

struct estat {
	int dwrong;
	int irv;
	int wepos;
	int nwords;
};

struct bcstat {
	int rfail;
	int rsuccess;
	int noncw;
	int nwords;
};

struct wspace {
	uint16_t *c;            /* sent codeword */
	uint16_t *r;            /* received word */
	int *errlocs;
	int *derrlocs;
};

static double Pad[] = { 0, 0.25, 0.5, 0.75, 1.0 };

static struct wspace *alloc_ws(struct rs_code *rs)
{
	int nn = rs->nn;
	int nroots = rs->nroots;
	struct wspace *ws;

	ws = calloc(1, sizeof(*ws));
	if (!ws)
		return NULL;

	ws->c = malloc(2 * nn * sizeof(*ws->c));
	if (!ws->c)
		goto err;

	ws->r = ws->c + nn;

	ws->errlocs = malloc((nn + nroots) * sizeof(*ws->errlocs));
	if (!ws->errlocs)
		goto err;

	ws->derrlocs = ws->errlocs + nn;
	return ws;

err:
	free(ws->errlocs);
	free(ws->c);
	free(ws);
	return NULL;
}

static void free_ws(struct wspace *ws)
{
	if (!ws)
		return;

	free(ws->errlocs);
	free(ws->c);
	free(ws);
}

/*
 * Generates a random codeword and stores it in c. Generates random errors and
 * erasures, and stores the random word with errors in r. Erasure positions are
 * stored in derrlocs, while errlocs has one of three values in every position:
 *
 * 0 if there is no error in this position;
 * 1 if there is a symbol error in this position;
 * 2 if there is an erasure without symbol corruption.
 *
 * Returns the number of corrupted symbols.
 */
static int get_rcw_we(struct rs_code *rs, struct wspace *ws,
		      int len, int errs, int eras)
{
	int nn = rs->nn;
	int nroots = rs->nroots;
	uint16_t *c = ws->c;
	uint16_t *r = ws->r;
	int *errlocs = ws->errlocs;
	int *derrlocs = ws->derrlocs;
	int dlen = len - nroots;
	int errval;
	int errloc;

	/* Load c with random data and encode */
	for (int i = 0; i < dlen; i++)
		c[i] = random() & nn;

	rs_encode(rs, c, len, 1);

	/* Make copy and add errors and erasures */
	memcpy(r, c, len * sizeof(*r));
	memset(errlocs, 0, len * sizeof(*errlocs));
	memset(derrlocs, 0, nroots * sizeof(*derrlocs));

	/* Generating random errors */
	for (int i = 0; i < errs; i++) {
		do {
			/* Error value must be nonzero */
			errval = random() & nn;
		} while (errval == 0);

		do {
			/* Must not choose the same location twice */
			errloc = random() % len;
		} while (errlocs[errloc] != 0);

		errlocs[errloc] = 1;
		r[errloc] ^= errval;
	}

	/* Generating random erasures */
	for (int i = 0; i < eras; i++) {
		do {
			/* Must not choose the same location twice */
			errloc = random() % len;
		} while (errlocs[errloc] != 0);

		derrlocs[i] = errloc;

		if (ewsc && (random() & 1)) {
			/* Erasure with the symbol intact */
			errlocs[errloc] = 2;
		} else {
			/* Erasure with corrupted symbol */
			do {
				/* Error value must be nonzero */
				errval = random() & nn;
			} while (errval == 0);

			errlocs[errloc] = 1;
			r[errloc] ^= errval;
			errs++;
		}
	}

	return errs;
}

/* Test up to error correction capacity */
static void test_uc(struct rs_code *rs, int len, int errs,
		    int eras, int trials, struct estat *stat,
		    struct wspace *ws)
{
	uint16_t *c = ws->c;
	uint16_t *r = ws->r;
	int *errlocs = ws->errlocs;
	int *derrlocs = ws->derrlocs;

	for (int j = 0; j < trials; j++) {
		int nerrs = get_rcw_we(rs, ws, len, errs, eras);
		int derrs = rs_decode(rs, r, len, 1, derrlocs, eras, derrlocs);

		if (derrs != nerrs)
			stat->irv++;

		for (int i = 0; i < derrs; i++) {
			if (errlocs[derrlocs[i]] != 1)
				stat->wepos++;
		}

		if (memcmp(r, c, len * sizeof(*r)))
			stat->dwrong++;
	}
	stat->nwords += trials;
}

int exercise_rs(struct rs_code *rs, struct wspace *ws,
		int len, int trials)
{
	struct estat stat = { 0, 0, 0, 0 };
	int nroots = rs->nroots;
	int retval = 0;

	if (v >= V_PROGRESS)
		printf("  Testing up to error correction capacity...\n");

	for (int errs = 0; errs <= nroots / 2; errs++)
		for (int eras = 0; eras <= nroots - 2 * errs; eras++)
			test_uc(rs, len, errs, eras, trials, &stat, ws);

	if (v >= V_CSUMMARY) {
		printf("    Decodes wrong:        %d / %d\n",
		       stat.dwrong, stat.nwords);
		printf("    Wrong return value:   %d / %d\n",
		       stat.irv, stat.nwords);
		printf("    Wrong error position: %d\n", stat.wepos);
	}

	retval = stat.dwrong + stat.wepos + stat.irv;
	if (retval && v >= V_PROGRESS)
		printf("  FAIL: %d decoding failures!\n", retval);

	return retval;
}

/* Tests for correct behaviour beyond error correction capacity */
static void test_bc(struct rs_code *rs, int len, int errs,
		    int eras, int trials, struct bcstat *stat,
		    struct wspace *ws)
{
	uint16_t *r = ws->r;
	uint16_t *c = ws->c;
	int *derrlocs = ws->derrlocs;
	int nroots = rs->nroots;
	int dlen = len - nroots;

	for (int j = 0; j < trials; j++) {
		get_rcw_we(rs, ws, len, errs, eras);
		int derrs = rs_decode(rs, r, len, 1, derrlocs, eras, derrlocs);

		if (derrs >= 0) {
			stat->rsuccess++;

			/*
			 * We check that the returned word is actually a
			 * codeword. The obious way to do this would be to
			 * compute the syndrome, but we don't want to replicate
			 * that code here. However, all the codes are in
			 * systematic form, and therefore we can encode the
			 * returned word, and see whether the parity changes or
			 * not.
			 */
			memcpy(c, r, len * sizeof(*c));
			rs_encode(rs, c, len, 1);

			if (memcmp(r + dlen, c + dlen, nroots * sizeof(*c)))
				stat->noncw++;
		} else {
			stat->rfail++;
		}
	}
	stat->nwords += trials;
}

int exercise_rs_bc(struct rs_code *rs, struct wspace *ws,
		   int len, int trials)
{
	struct bcstat stat = { 0, 0, 0, 0 };
	int nroots = rs->nroots;
	int errs, eras, cutoff;

	if (v >= V_PROGRESS)
		printf("  Testing beyond error correction capacity...\n");

	for (errs = 1; errs <= nroots; errs++) {
		eras = nroots - 2 * errs + 1;
		if (eras < 0)
			eras = 0;

		cutoff = nroots <= len - errs ? nroots : len - errs;
		for (; eras <= cutoff; eras++)
			test_bc(rs, len, errs, eras, trials, &stat, ws);
	}

	if (v >= V_CSUMMARY) {
		printf("    decoder gives up:        %d / %d\n",
		       stat.rfail, stat.nwords);
		printf("    decoder returns success: %d / %d\n",
		       stat.rsuccess, stat.nwords);
		printf("      not a codeword:        %d / %d\n",
		       stat.noncw, stat.rsuccess);
	}

	if (stat.noncw && v >= V_PROGRESS)
		printf("  FAIL: %d silent failures!\n", stat.noncw);

	return stat.noncw;
}

static int run_exercise(struct etab *e, int valgrind)
{
	struct rs_code *rs;
	struct wspace *ws;
	int nn = (1 << e->symsize) - 1;
	int kk = nn - e->nroots;
	int max_pad = kk - 1;
	int prev_pad = -1;
	int retval = -ENOMEM;

	rs = rs_init(e->symsize, e->gfpoly, e->fcr, e->prim, e->nroots);
	if (!rs)
		return retval;

	ws = alloc_ws(rs);
	if (!ws)
		goto err;

	int trials = valgrind ? 10 : e->ntrials;

	retval = 0;
	for (size_t i = 0; i < ARRAY_SIZE(Pad); i++) {
		int pad = Pad[i] * max_pad;
		int len = nn - pad;

		if (pad == prev_pad)
			continue;

		prev_pad = pad;
		if (v >= V_PROGRESS) {
			printf("Testing (%d,%d)_%d code...\n",
			       len, kk - pad, nn + 1);
		}

		retval |= exercise_rs(rs, ws, len, trials);
		retval |= exercise_rs_bc(rs, ws, len, trials);
	}

	free_ws(ws);

err:
	rs_free(rs);
	return retval;
}

int main(int argc, char **argv)
{
	int fail = 0;
	int valgrind = 0;
	size_t tab_size = ARRAY_SIZE(Tab);

	srandom(time(NULL));

	if (argc > 1) {
		if (strcmp(argv[1], "short")) {
			printf("Invalid argument: '%s'", argv[1]);
			return -1;
		}

		tab_size = tab_size < 7 ? tab_size : 7;
		valgrind = 1;
	}

	for (size_t i = 0; i < tab_size; i++) {
		int retval = run_exercise(Tab + i, valgrind);
		if (retval < 0) {
			printf("Memory allocation error\n");
			return -1;
		}

		fail |= retval;
	}

	printf("tests %s\n", fail ? "failed" : "passed");
	return fail;
}
