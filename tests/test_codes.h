/*
 * test_codes.h
 * Copyright (C) 2019 Ferdinand Blomqvist
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

#ifndef FB_LIBRS_TEST_CODES_H
#define FB_LIBRS_TEST_CODES_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct etab {
	int symsize;
	int gfpoly;
	int fcr;
	int prim;
	int nroots;
	int ntrials;
};

/* List of codes to test */
static struct etab Tab[] = {
	{ 2,  0x7,    1,   1,  1,  100000 },
	{ 3,  0xb,    1,   1,  2,  100000 },
	{ 3,  0xb,    1,   1,  3,  100000 },
	{ 3,  0xb,    2,   1,  4,  100000 },
	{ 4,  0x13,   1,   1,  5,  50000  },
	{ 5,  0x25,   1,   1,  6,  10000  },
	{ 6,  0x43,   3,   1,  8,  5000	  },
	{ 7,  0x89,   1,   1,  10, 1000	  },
	{ 8,  0x11d,  1,   1,  28, 100	  },
	{ 8,  0x187,  112, 11, 32, 100	  },
	{ 9,  0x211,  1,   1,  29, 100	  },
	{ 10, 0x409,  1,   1,  30, 100	  },
	{ 11, 0x805,  4,   1,  31, 50	  },
	{ 16, 0x1100b,  5,   1,  33, 1	  },
};

#endif /* FB_LIBRS_TEST_CODES_H */
