/*
 * extra_tests.c
 * Copyright (C) 2019 Ferdinand Blomqvist
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

#include "test_codes.h"
#include "librs.h"
#include <stdio.h>
/*
ret for columns: 0 1 0 0 1 0 0
strat 0: viable: 1; { 1 4 }
strat 1: viable: 1; { }
processing strat 1
ret for rows: 1 1 1 1 0 1 0
w: 3
c:
*/

static uint16_t arr[] = {
	7, 5, 6, 4, 7, 6, 3,
	4, 5, 5, 3, 7, 2, 7,
	5, 3, 6, 5, 2, 7, 5,
	3, 5, 0, 6, 1, 7, 2,
	0, 2, 2, 3, 7, 7, 3,
	0, 5, 7, 6, 7, 7, 7,
	1, 6, 1, 2, 5, 4, 0,
};

static uint16_t t_arr[] = {
	7, 4, 5, 3, 0, 0, 1,
	5, 5, 3, 5, 2, 5, 6,
	6, 5, 6, 0, 2, 7, 1,
	4, 3, 5, 6, 3, 6, 2,
	7, 7, 2, 1, 7, 7, 5,
	6, 2, 7, 7, 7, 7, 4,
	3, 7, 5, 2, 3, 7, 0,
};

static uint16_t err[] = {
	0, 6, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 6, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 5, 0, 0, 5, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
};

static uint16_t t_err[] = {
	0, 0, 0, 0, 0, 0, 0,
	6, 0, 0, 0, 0, 5, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 6, 0, 0, 0, 5, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
};

static void add_error(uint16_t* rec, const uint16_t* cw, const uint16_t* err)
{
	for (size_t i = 0; i < ARRAY_SIZE(arr); i++)
		rec[i] = cw[i] ^ err[i];
}

static int trans_is_equal(const uint16_t* rec, const uint16_t* t_rec,
			size_t rows, size_t cols)
{
	for(size_t r = 0; r < rows; r++) {
		for(size_t c = 0; c < cols; c++) {
			if (rec[c * rows + r] != t_rec[r * cols + c])
				return 0;
		}
	}

	return 1;
}

int main(void)
{
	uint16_t rec[ARRAY_SIZE(arr)];
	uint16_t t_rec[ARRAY_SIZE(arr)];

	struct etab* e = &Tab[1];
	struct rs_code* rsc;
	int retval = -1;

	rsc = rs_init(e->symsize, e->gfpoly, e->fcr, e->prim, e->nroots);
	if (!rsc)
		return -1;


	add_error(rec, arr, err);
	add_error(t_rec, t_arr, t_err);

	for (size_t i = 0; i < 7; i++) {
		int ret_stride = rs_decode(rsc, &rec[i], 7, 7, NULL, 0, NULL);
		int ret_normal = rs_decode(rsc, &t_rec[i * 7], 7, 1, NULL, 0, NULL);

		printf("ret_stride: %d, ret_normal %d\n", ret_stride, ret_normal);
		if (ret_stride != ret_normal) {
			printf("FAIL: ret_val neq!\n");
			goto err;
		}

	}

	if (!trans_is_equal(rec, t_rec, 7, 7))
		printf("FAIL: did not decode to the same\n");
	else
		retval = 0;

err:
	rs_free(rsc);
	return retval;
}

