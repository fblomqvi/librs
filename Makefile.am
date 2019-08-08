ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -Wall -Wextra -pedantic -I$(srcdir)/src/ $(PTHREAD_CFLAGS)

lib_LTLIBRARIES = librs.la
include_HEADERS = src/librs.h
librs_la_SOURCES = src/internal.c src/internal.h src/list.h src/list.c src/reed_solomon.c

dist_man_MANS = librs.3

TESTS = tests/alloc_tests tests/extra_tests tests/rs_tests
check_PROGRAMS = $(TESTS)
check_HEADERS = src/librs.h tests/test_codes.h

tests_alloc_tests_SOURCES = tests/alloc_tests.c tests/test_codes.h src/librs.h
tests_alloc_tests_LDADD = librs.la
tests_alloc_tests_LDFLAGS = -no-install $(PTHREAD_CFLAGS) $(PTHREADS_LIBS)

tests_extra_tests_SOURCES = tests/extra_tests.c tests/test_codes.h src/librs.h
tests_extra_tests_LDADD = librs.la
tests_extra_tests_LDFLAGS = -no-install $(PTHREAD_CFLAGS) $(PTHREADS_LIBS)

tests_rs_tests_SOURCES = tests/rs_tests.c tests/test_codes.h src/librs.h
tests_rs_tests_LDADD = librs.la
tests_rs_tests_LDFLAGS = -no-install $(PTHREAD_CFLAGS) $(PTHREADS_LIBS)

EXTRA_DIST = LICENSE
dist-hook:
	cp $(srcdir)/README.md $(distdir)/README.md
