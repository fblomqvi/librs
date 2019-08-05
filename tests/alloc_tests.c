/*
 * alloc_tests.c
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
#include "librs.h"
#include "test_codes.h"
#include <stddef.h>
#include <stdio.h>

int main(void)
{
	size_t num = 4;
	struct rs_code* rsc[ARRAY_SIZE(Tab) * num];

	for (size_t i = 0; i < ARRAY_SIZE(Tab); i++) {
		struct etab* e = &Tab[i];
		for (size_t j = 0; j < num; j++) {
			rsc[i * num + j] = rs_init(e->symsize, e->gfpoly, e->fcr,
						e->prim, e->nroots);
			if (!rsc[i * num + j]) {
				printf("rs_init failed!\n");
				return -1;
			}
		}
	}

	for (size_t i = 0; i < ARRAY_SIZE(rsc); i++)
		rs_free(rsc[i]);

	return 0;
}
