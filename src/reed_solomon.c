/*
 * reed_solomon.c
 * Copyright (C) 2002 Phil Karn, KA9Q
 * Copyright (C) 2019 Ferdinand Blomqvist
 *
 * This file is part of librs.
 *
 * The Reed-Solomon code is copied from Phil Karn's fec library. Refactoring,
 * bugfixes, additional code and adaption to new interface by Ferdinand
 * Blomqvist
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
#include "internal.h"
#include <string.h>
#include <stdlib.h>

#undef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* Initialize a Reed-Solomon codec
 * symsize = symbol size, bits
 * gfpoly = Field generator polynomial coefficients
 * fcr = first root of RS code generator polynomial, index form
 * prim = primitive element to generate polynomial roots
 * nroots = RS code generator polynomial degree (number of roots)
 */
struct rs_code *rs_init(int symsize, int gfpoly, int fcr, int prim, int nroots)
{
	/* Check parameter ranges */
	if (symsize < 0 || (size_t) symsize > 8 * sizeof(uint16_t))
		return NULL;

	if (fcr < 0 || fcr >= (1 << symsize))
		return NULL;
	if (prim <= 0 || prim >= (1 << symsize))
		return NULL;
	if (nroots < 0 || nroots >= (1 << symsize))
		return NULL; /* Can't have more roots than symbol values! */

	struct rs_code *rs = rs_init_internal(symsize, gfpoly, fcr, prim, nroots);
	return rs;
}

void rs_free(struct rs_code *rs)
{
	rs_free_internal(rs);
}

static void encode(struct rs_code *rs, uint16_t *data, uint16_t *par,
		   int dlen, int stride)
{
	uint16_t *alpha_to = rs->alpha_to;
	uint16_t *index_of = rs->index_of;
	uint16_t *gp = rs->genpoly;
	int nroots = rs->nroots;
	int nn = rs->nn;

	memset(par, 0, nroots * sizeof(*par));

	int cutoff = dlen * stride;
	for (int i = 0; i < cutoff; i += stride) {
		uint16_t fb = index_of[data[i] ^ par[0]];
		if (fb != nn) {
			/* feedback term is non-zero */
			for (int j = 1; j < nroots; j++) {
				par[j] ^= alpha_to[modnn(rs,
						fb + gp[nroots - j])];
			}
		}

		/* Shift */
		memmove(&par[0], &par[1], sizeof(*par) * (nroots - 1));
		if (fb != nn)
			par[nroots - 1] = alpha_to[modnn(rs, fb + gp[0])];
		else
			par[nroots - 1] = 0;
	}
}

void rs_encode(struct rs_code *rs, uint16_t *data, int len, int stride)
{
	int nroots = rs->nroots;
	int dlen = len - nroots;

	if (stride == 1) {
		/* Calculate parity in-place */
		encode(rs, data, data + dlen, dlen, stride);
	} else {
		/* Calculate parity in buffer */
		uint16_t parity[nroots];
		encode(rs, data, parity, dlen, stride);

		/* Write the parity data to the real parity location */
		uint16_t *par = data + dlen * stride;
		for (int i = 0; i < nroots; i++)
			par[i * stride] = parity[i];
	}
}

static inline void update_si(struct rs_code *rs, uint16_t *s, uint16_t data, int i)
{
	uint16_t *alpha_to = rs->alpha_to;
	uint16_t *index_of = rs->index_of;
	int fcr = rs->fcr;
	int prim = rs->prim;

	if (s[i] == 0) {
		s[i] = data;
	} else {
		int tmp = index_of[s[i]] + (fcr + i) * prim;
		s[i] = data ^ alpha_to[modnn(rs, tmp)];
	}
}

/* form the syndromes; i.e., evaluate data(x) at roots of g(x) */
static void compute_syndrome(struct rs_code *rs, uint16_t *s,
			     uint16_t* data, int len, int stride)
{
	for (int i = 0; i < rs->nroots; i++)
		s[i] = data[0];

	int cutoff = len * stride;
	for (int j = stride; j < cutoff; j += stride) {
		for (int i = 0; i < rs->nroots; i++)
			update_si(rs, s, data[j], i);
	}
}

int rs_decode(struct rs_code *rs, uint16_t *data, int len,
	      int stride, const int *eras, int no_eras, int *err_pos)
{
	uint16_t *alpha_to = rs->alpha_to;
	uint16_t *index_of = rs->index_of;
	int nn = rs->nn;
	int nroots = rs->nroots;
	int fcr = rs->fcr;
	int prim = rs->prim;
	int iprim = rs->iprim;
	int pad = nn - len;

	uint16_t s[nroots], si[nroots];
	uint16_t root[nroots], loc[nroots];
	uint16_t lambda[nroots + 1];	/* Error and erasure locator poly */
	uint16_t omega[nroots + 1];	/* Error and erasure evaluator poly */
	uint16_t b[nroots + 1], t[nroots + 1];	/* workspace */

	if (no_eras > nroots)
		return RS_ERROR_TOO_MANY_ERASURES;

	compute_syndrome(rs, s, data, len, stride);

	/* Convert syndromes to index form, checking for nonzero condition */
	int syn_error = 0;
	for (int i = 0; i < nroots; i++) {
		syn_error |= s[i];
		si[i] = index_of[s[i]];
	}

	if (!syn_error) {
		/* if syndrome is zero, data[] is a codeword and there are no
		 * errors to correct. So return data[] unmodified
		 */
		return 0;
	}

	memset(&lambda[1], 0, nroots * sizeof(lambda[0]));
	lambda[0] = 1;

	if (no_eras > 0) {
		/* Init lambda to be the erasure locator polynomial */
		lambda[1] = alpha_to[modnn(rs, prim * (nn - 1 - (eras[0] + pad)))];
		for (int i = 1; i < no_eras; i++) {
			uint16_t u = modnn(rs, prim * (nn - 1 - (eras[i] + pad)));
			for (int j = i + 1; j > 0; j--) {
				uint16_t tmp = index_of[lambda[j - 1]];
				if (tmp != nn)
					lambda[j] ^= alpha_to[modnn(rs, u + tmp)];
			}
		}
	}

	for (int i = 0; i < nroots + 1; i++)
		b[i] = index_of[lambda[i]];

	/*
	 * Begin Berlekamp-Massey algorithm to determine error+erasure
	 * locator polynomial
	 */
	int r = no_eras;
	int el = no_eras;
	while (++r <= nroots) { /* r is the step number */
		/* Compute discrepancy at the r-th step in poly-form */
		uint16_t discr_r = 0;
		for (int i = 0; i < r; i++)
			if ((lambda[i] != 0) && (si[r - i - 1] != nn)) {
				discr_r ^= alpha_to[modnn(rs,
						index_of[lambda[i]] + si[r - i - 1])];
			}

		discr_r = index_of[discr_r]; /* Index form */

		if (discr_r == nn) {
			/* 2 lines below: B(x) <-- x*B(x) */
			memmove(&b[1], b, nroots * sizeof(b[0]));
			b[0] = nn;
		} else {
			/* 9 lines below: T(x) <-- lambda(x) - discr_r*x*b(x) */
			t[0] = lambda[0];
			for (int i = 0; i < nroots; i++) {
				if (b[i] != nn) {
					t[i + 1] = lambda[i + 1]
						^ alpha_to[modnn(rs, discr_r + b[i])];
				} else {
					t[i + 1] = lambda[i + 1];
				}
			}

			if (2 * el <= r + no_eras - 1) {
				el = r + no_eras - el;
				/*
				 * 5 lines below: B(x) <-- inv(discr_r) *
				 * lambda(x)
				 */
				for (int i = 0; i <= nroots; i++) {
					b[i] = (lambda[i] == 0) ? nn
					       : modnn(rs, index_of[
							lambda[i]] - discr_r + nn);
				}
			} else {
				/* 2 lines below: B(x) <-- x*B(x) */
				memmove(&b[1], b, nroots * sizeof(b[0]));
				b[0] = nn;
			}
			memcpy(lambda, t, (nroots + 1) * sizeof(t[0]));
		}
	}

	/* Convert lambda to index form and compute deg(lambda(x)) */
	int deg_lambda = 0;
	for (int i = 0; i < nroots + 1; i++) {
		lambda[i] = index_of[lambda[i]];
		if (lambda[i] != nn)
			deg_lambda = i;
	}

	if (deg_lambda == 0) {
		/* deg(lambda) is zero even though the syndrome is non-zero
		 * => uncorrectable error detected
		 */
		return RS_ERROR_DEG_LAMBDA_ZERO;
	}

	/* Find roots of the error+erasure locator polynomial by Chien search */
	memcpy(&b[1], &lambda[1], nroots * sizeof(b[0]));
	int count = 0;          /* Number of roots of lambda(x) */
	for (int i = 1, k = iprim - 1; i <= nn; i++, k = modnn(rs, k + iprim)) {
		uint16_t q = 1; /* lambda[0] is always 0 */
		for (int j = deg_lambda; j > 0; j--) {
			if (b[j] != nn) {
				b[j] = modnn(rs, b[j] + j);
				q ^= alpha_to[b[j]];
			}
		}
		if (q != 0)
			continue; /* Not a root */

		if (k < pad)
			/* Impossible error location. Uncorrectable error. */
			return RS_ERROR_IMPOSSIBLE_ERR_POS;

		/* store root (index-form) and error location number */
		root[count] = i;
		loc[count] = k;
		/* If we've already found max possible roots,
		 * abort the search to save time
		 */
		if (++count == deg_lambda)
			break;
	}

	if (deg_lambda != count) {
		/*
		 * deg(lambda) unequal to number of roots => uncorrectable
		 * error detected
		 */
		return RS_ERROR_DEG_LAMBDA_NEQ_COUNT;
	}

	/*
	 * Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) (modulo
	 * x**nroots). in index form. Also find deg(omega).
	 */
	int deg_omega = deg_lambda - 1;
	for (int i = 0; i <= deg_omega; i++) {
		uint16_t tmp = 0;
		for (int j = i; j >= 0; j--) {
			if ((si[i - j] != nn) && (lambda[j] != nn))
				tmp ^= alpha_to[modnn(rs, si[i - j] + lambda[j])];
		}

		omega[i] = index_of[tmp];
	}

	/* We reuse the buffer for b with a more appropriate name */
	uint16_t *const cor = b;
	int num_corrected = 0;

	/*
	 * Compute error values in poly-form. num1 = omega(inv(X(l))), num2 =
	 * inv(X(l))**(fcr-1) and den = lambda_pr(inv(X(l))) all in poly-form
	 */
	for (int j = 0; j < count; j++) {
		uint16_t num1 = 0;
		for (int i = deg_omega; i >= 0; i--) {
			if (omega[i] != nn)
				num1 ^= alpha_to[modnn(rs, omega[i] + i * root[j])];
		}

		if (num1 == 0)
			continue;

		num1 = index_of[num1];
		uint16_t num2 = modnn(rs, root[j] * (fcr - 1) + nn);
		uint16_t den = 0;

		/* lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i] */
		for (int i = MIN(deg_lambda, nroots - 1) & ~1; i >= 0; i -= 2) {
			if (lambda[i + 1] != nn)
				den ^= alpha_to[modnn(rs, lambda[i + 1] + i * root[j])];
		}

		den = index_of[den];
		cor[num_corrected] = modnn(rs, num1 + num2 + nn - den);
		loc[num_corrected++] = loc[j];
	}

	/* We compute the syndrome of the 'error' and check that it matches the
	 * syndrome of the received word */
	for (int i = 0; i < nroots; i++) {
		uint16_t tmp = 0;
		for (int j = 0; j < num_corrected; j++) {
			int k = (fcr + i) * prim * (nn - loc[j] - 1);
			tmp ^= alpha_to[modnn(rs, cor[j] + k)];
		}

		if (tmp != s[i])
			return RS_ERROR_NOT_A_CODEWORD;
	}

	/* Apply error to data */
	for (int i = 0; i < num_corrected; i++)
		data[(loc[i] - pad) * stride] ^= alpha_to[cor[i]];

	/* Return the error positions if the caller wants them */
	if (err_pos != NULL) {
		for (int i = 0; i < num_corrected; i++)
			err_pos[i] = loc[i] - pad;
	}

	return num_corrected;
}

int rs_is_cword(struct rs_code *rs, uint16_t *data, int len, int stride)
{
	uint16_t s[rs->nroots];

	compute_syndrome(rs, s, data, len, stride);

	/* Check if non-zero */
	for (int i = 0; i < rs->nroots; i++)
		if (s[i])
			return 0;

	return 1;
}
