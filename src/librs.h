/*
 * librs.h
 * Copyright (C) 2002 Phil Karn, KA9Q
 * Copyright (C) 2019 Ferdinand Blomqvist
 *
 * The Reed-Solomon code is copied from Phil Karn's fec library. Bugfixes,
 * additional code and adaption to new interface by Ferdinand Blomqvist
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

#ifndef FB_LIBRS_H
#define FB_LIBRS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RS_ERROR_DEG_LAMBDA_ZERO -1;
#define RS_ERROR_IMPOSSIBLE_ERR_POS -2;
#define RS_ERROR_DEG_LAMBDA_NEQ_COUNT -3;
#define RS_ERROR_NOT_A_CODEWORD -4;
#define RS_ERROR_TOO_MANY_ERASURES -5;

struct rs_code {
	uint16_t *alpha_to;     /* log lookup table */
	uint16_t *index_of;     /* Antilog lookup table */
	uint16_t *genpoly;      /* Generator polynomial */
	int mm;                 /* Bits per symbol */
	int nn;                 /* Symbols per block (= (1<<mm)-1) */
	int nroots;             /* Number of generator roots = number of parity symbols */
	int fcr;                /* First consecutive root, index form */
	int prim;               /* Primitive element, index form */
	int iprim;              /* prim-th root of 1, index form */
	int gfpoly;
	int users;
};

/* Initialize a Reed-Solomon code
 * symsize = symbol size, bits
 * gfpoly = Field generator polynomial coefficients
 * fcr = first root of RS code generator polynomial, index form
 * prim = primitive element to generate polynomial roots
 * nroots = RS code generator polynomial degree (number of roots)
 */
struct rs_code *rs_init(int symsize, int gfpoly,
			int fcr, int prim, int nroots);

void rs_free(struct rs_code *rs);

void rs_encode(struct rs_code *rs, uint16_t *data, int len, int stride);
int rs_decode(struct rs_code *rs, uint16_t *data, int len,
	      int stride, const int *eras, int no_eras, int *err_pos);
int rs_is_cword(struct rs_code *rs, uint16_t *data, int len, int stride);

/* Convenience functions */
static inline int rs_mind(struct rs_code* rs)
{ return rs->nroots + 1; }

#ifdef __cplusplus
}
#endif

#endif /* FB_LIBRS_H */
