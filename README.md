[![Build Status](https://travis-ci.com/fblomqvi/librs.svg?branch=master)](https://travis-ci.com/fblomqvi/librs)

__librs__ is a small library for forward error correction with Reed-Solomon codes.
It is based on the Reed-Solomon code in Phil Karn's libfec, but with bugfixes,
optimizations and a slightly different interface. The encoder and decoder are
general purpose and not optimized for any particular code. The main use case is
for simulations with Reed-Solomon codes.

If you need a Reed-Solomon library for production use that is optimized for one
specific code, then the various C++ template libraries might be faster and/or
better suited for that particular application.

INSTALLATION
------------

If installing from a release tarball, the standard

    ./configure
    make
    make install

will suffice. If you're building from git then run

    autoreconf -i --force
    ./configure
    make
    make install

USAGE
-----

In short
```C

    struct rs_code* rs;

    // init Reed-Solomon code
    rs = rs_init(symsize, gfpoly, fcr, prim, nroots);
    if (!rs)
	    // Handle error!

    /* Encode your data. The data buffer must contain enough space for the
     * parity symbols. More specifically, len = 'length of data' + nroots. */
    rs_encode(rs, data, len, stride);

    // Possible data corruption...

    // Correct errors (no erasures)
    int ret_val = rs_decode(rs, data, len, stride, NULL, 0, NULL);
    if (ret_val < 0)
	// decoding failure, handle appropriately

    // Free resources
    rs_free(rs);
```

See the man page for more information.

NOTES
-----
The Reed-Solomon encoding used by the library is the BCH view, and the encoding
is systematic.
For more information, see, for instance, [Wikipedia](https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction#The_BCH_view:_The_codeword_as_a_sequence_of_coefficients).
