/*
 * internal.h
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

#ifndef FB_LIBRS_INTERNAL_H
#define FB_LIBRS_INTERNAL_H

#include <stdint.h>
#include "librs.h"

struct rs_code *rs_init_internal(int symsize, int gfpoly,
				 int fcr, int prim, int nroots);

void rs_free_internal(struct rs_code *rs);

static inline int modnn(struct rs_code *rs, int x)
{
	while (x >= rs->nn) {
		x -= rs->nn;
		x = (x >> rs->mm) + (x & rs->nn);
	}
	return x;
}

#endif /* FB_LIBRS_INTERNAL_H */
