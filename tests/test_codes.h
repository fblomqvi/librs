#ifndef FB_LIBRS_TEST_CODES
#define FB_LIBRS_TEST_CODES

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
	{2,	0x7,	1,	1,	1,	100000	},
	{3,	0xb,	1,	1,	2,	100000	},
	{3,	0xb,	1,	1,	3,	100000	},
	{3,	0xb,	2,	1,	4,	100000	},
	{4,	0x13,	1,	1,	4,	50000	},
	{5,	0x25,	1,	1,	6,	10000	},
	{6,	0x43,	3,	1,	8,	5000	},
	{7,	0x89,	1,	1,	10,	1000	},
	{8,	0x11d,	1,	1,	32,	100	},
	{8,	0x187,	112,	11,	32,	100	},
	{9,	0x211,	1,	1,	32,	100	},
	{10,	0x409,	1,	1,	32,	100	},
	{11,	0x805,	1,	1,	32,	100	},
	{12,	0x1053,	1,	1,	32,	50	},
	{12,	0x1053,	1,	1,	64,	50	},
	/*
	 * {13,	0x201b,	1,	1,	32,	20	},
	 * {13,	0x201b,	1,	1,	64,	20	},
	 * {14,	0x4443,	1,	1,	32,	10	},
	 * {14,	0x4443	1,	1,	64,	10	},
	 * {15,	0x8003,	1,	1,	32,	5	},
	 * {15,	0x8003,	1,	1,	64,	5	},
	 * {16,	0x1100,	1,	1,	32,	5	},
	 */
};


#endif /* FB_LIBRS_TEST_CODES */
