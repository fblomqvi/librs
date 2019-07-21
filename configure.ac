# Must init the autoconf setup
# The first parameter is project name
# second is version number
# third is bug report address
AC_INIT([librs], [0.1], [https://github.com/fblomqvi/librs])

# Force autoconf to be at least this version number:
AC_PREREQ([2.69])

# Safety check - list a source file that wouldn't be in other directories:
AC_CONFIG_SRCDIR([src/reed_solomon.c])

# Put autotools auxiliary files in subdirectories to reduce clutter:
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

# Init automake, and specify this program use relaxed structures.
# i.e. this program doesn't follow the gnu coding standards, and doesn't have
# ChangeLog, COPYING, AUTHORS, INSTALL, README etc. files.
AM_INIT_AUTOMAKE([1.11 -Wall -Werror foreign subdir-objects silent-rules])
AM_SILENT_RULES([yes])

# Check for C compiler
AC_PROG_CC
# We can add more checks in this section
#AC_PROG_RANLIB
AM_PROG_AR
LT_INIT

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h string.h])
if [test -z "$HAVE_stddef.h"] ; then
    AC_MSG_ERROR([Cannot find stddef.h!])
fi
if [test -z "$HAVE_stdint.h"] ; then
    AC_MSG_ERROR([Cannot find stdint.h!])
fi
if [test -z "$HAVE_stdlib.h"] ; then
    AC_MSG_ERROR([Cannot find stdlib.h!])
fi
if [test -z "$HAVE_string.h"] ; then
    AC_MSG_ERROR([Cannot find string.h])
fi

AX_PTHREAD()
if [test "x$ax_pthread_ok" != "xyes"] ; then
    AC_MSG_ERROR([Cannot find POSIX threads!])]
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove memset])

# Tells automake to create a Makefile
# See https://www.gnu.org/software/automake/manual/html_node/Requirements.html
AC_CONFIG_FILES([Makefile])

# Generate the output
AC_OUTPUT

