/*
 * internal.c
 * Copyright (C) 2002 Phil Karn, KA9Q
 * Copyright (C) 2019 Ferdinand Blomqvist
 *
 * This file is part of librs.
 *
 * The Reed-Solomon code is copied from Phil Karn's fec library. Bugfixes,
 * additional code and adaption to new interface by Ferdinand Blomqvist
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "internal.h"
#include "list.h"
#include <pthread.h>

struct lookup_table {
	int users;
	int mm;
	int gfpoly;
	uint16_t *alpha_to;
	uint16_t *index_of;
};

pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;

static LIST _lookup_tables = { NULL, NULL };
static LIST _codes = { NULL, NULL };

static struct lookup_table *init_lookup(int mm, int gfpoly)
{
	int nn = (1 << mm) - 1;

	struct lookup_table *tab = calloc(1, sizeof(*tab));
	if (!tab)
		return NULL;

	tab->alpha_to = malloc(sizeof(*tab->alpha_to) * 2 * (nn + 1));
	if (!tab->alpha_to)
		goto err;

	tab->users = 1;
	tab->mm = mm;
	tab->gfpoly = gfpoly;
	tab->index_of = tab->alpha_to + (nn + 1);

	/* Generate Galois field lookup tables */
	tab->index_of[0] = nn;  /* log(zero) = -inf */
	tab->alpha_to[nn] = 0;  /* alpha**-inf = 0 */
	int sr = 1;

	for (int i = 0; i < nn; i++) {
		tab->index_of[sr] = i;
		tab->alpha_to[i] = sr;
		sr <<= 1;
		if (sr & (1 << mm))
			sr ^= gfpoly;
		sr &= nn;
	}
	if (sr != 1) {
		/* field generator polynomial is not primitive! */
		goto err;
	}

	return tab;

err:
	free(tab->alpha_to);
	free(tab);
	return NULL;
}

static void free_lookup_table(struct lookup_table *tab)
{
	free(tab->alpha_to);
	free(tab);
}

static struct lookup_table *get_lookup(int mm, int gfpoly)
{
	/* Check if we already have a lookup table for the right parameters */
	LIST_NODE *node = LIST_first(&_lookup_tables);
	while (node) {
		struct lookup_table *tab = (struct lookup_table *) node->data;
		if (tab->mm == mm && tab->gfpoly == gfpoly) {
			tab->users++;
			return tab;
		}

		node = LIST_next(node);
	}

	/* Create a new lookup table */
	struct lookup_table *tab = init_lookup(mm, gfpoly);
	if (!tab)
		return NULL;

	if (!LIST_push_front(&_lookup_tables, tab))
		goto err;

	return tab;


err:
	free_lookup_table(tab);
	return NULL;
}

static void free_lookup(uint16_t *alpha_to)
{
	/* Find the correct lookup table */
	LIST_NODE *node = LIST_first(&_lookup_tables);
	while (node) {
		struct lookup_table *tab = (struct lookup_table *) node->data;
		if (tab->alpha_to == alpha_to) {
			if (--tab->users == 0) {
				free_lookup_table(tab);
				LIST_remove(&_lookup_tables, node, 1);
			}
			return;
		}

		node = LIST_next(node);
	}
}

/* Initialize a Reed-Solomon codec
 * symsize = symbol size, bits
 * gfpoly = Field generator polynomial coefficients
 * fcr = first root of RS code generator polynomial, index form
 * prim = primitive element to generate polynomial roots
 * nroots = RS code generator polynomial degree (number of roots)
 */
static struct rs_code *init_code(int symsize, int gfpoly,
				 int fcr, int prim, int nroots)
{
	struct rs_code *rs = calloc(1, sizeof(*rs));
	if (!rs)
		return NULL;

	rs->genpoly = malloc(sizeof(*rs->genpoly) * (nroots + 1));
	if (!rs->genpoly)
		goto err;

	struct lookup_table *tab = get_lookup(symsize, gfpoly);
	if (!tab)
		goto err;

	rs->alpha_to = tab->alpha_to;
	rs->index_of = tab->index_of;
	rs->mm = symsize;
	rs->nn = (1 << symsize) - 1;
	rs->nroots = nroots;
	rs->fcr = fcr;
	rs->prim = prim;
	rs->gfpoly = gfpoly;
	rs->users = 1;

	/*
	 * Form RS code generator polynomial from its roots
	 * Find prim-th root of 1, used in decoding
	 */
	uint16_t tmp;
	int iprim;
	for (iprim = 1; (iprim % prim) != 0; iprim += rs->nn)
		;
	rs->iprim = iprim / prim;

	rs->genpoly[0] = 1;
	for (int i = 0, root = fcr * prim; i < nroots; i++, root += prim) {
		rs->genpoly[i + 1] = 1;

		/* Multiply rs->genpoly[] by  @**(root + x) */
		for (int j = i; j > 0; j--) {
			if (rs->genpoly[j] != 0) {
				tmp = rs->index_of[rs->genpoly[j]] + root;
				tmp = rs->alpha_to[modnn(rs, tmp)];
				rs->genpoly[j] = rs->genpoly[j - 1] ^ tmp;
			} else {
				rs->genpoly[j] = rs->genpoly[j - 1];
			}
		}

		/* rs->genpoly[0] can never be zero */
		tmp = rs->index_of[rs->genpoly[0]] + root;
		rs->genpoly[0] = rs->alpha_to[modnn(rs, tmp)];
	}
	/* convert rs->genpoly[] to index form for quicker encoding */
	for (int i = 0; i <= nroots; i++)
		rs->genpoly[i] = rs->index_of[rs->genpoly[i]];

	return rs;

err:
	free(rs->genpoly);
	free(rs);
	return NULL;
}

static void free_code(struct rs_code *rs)
{
	free_lookup(rs->alpha_to);
	free(rs->genpoly);
	free(rs);
}

struct rs_code *rs_init_internal(int symsize, int gfpoly,
				 int fcr, int prim, int nroots)
{
	struct rs_code *rs;
	pthread_mutex_lock(&_lock);

	/* Check if we already have a code with the right parameters */
	LIST_NODE *node = LIST_first(&_codes);
	while (node) {
		rs = (struct rs_code *) node->data;
		if (rs->mm == symsize && rs->gfpoly == gfpoly
		    && rs->fcr == fcr && rs->prim == prim
		    && rs->nroots == nroots) {
			rs->users++;
			goto exit;
		}

		node = LIST_next(node);
	}

	/* Create a new code */
	rs = init_code(symsize, gfpoly, fcr, prim, nroots);
	if (!rs)
		goto err;

	if (!LIST_push_front(&_codes, rs))
		goto err;

exit:
	pthread_mutex_unlock(&_lock);
	return rs;

err:
	pthread_mutex_unlock(&_lock);
	free_code(rs);
	return NULL;
}

void rs_free_internal(struct rs_code *rs)
{
	if (!rs)
		return;

	pthread_mutex_lock(&_lock);

	/* Find the correct code from the list */
	LIST_NODE *node = LIST_first(&_codes);
	while (node) {
		struct rs_code *code = (struct rs_code *) node->data;
		if (code == rs) {
			if (--code->users == 0) {
				free_code(code);
				LIST_remove(&_codes, node, 1);
			}
			break;
		}

		node = LIST_next(node);
	}

	pthread_mutex_unlock(&_lock);
}
