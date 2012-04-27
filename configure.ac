# Process this file with autoconf 2.0 or later to make a configure script.

# readgenesis autoconf file.
# Cengiz Gunay 2012-04-27

# Inherited some stuff from the gencompress project

# $Id: configure.ac 8 2007-07-18 22:55:43Z cengiz $

# Basic stuff
AC_REVISION([$Revision: 8 $])
AC_PREREQ(2.60)
AC_INIT(readgensis, 1.0b, [cengique@users.sf.net], readgenesis)

# Check for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_INCLUDES_DEFAULT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h float.h malloc.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strncasecmp strrchr])

# Check for endianness
AC_C_BIGENDIAN([ BE_CFLAGS=; LE_CFLAGS="-UWORDS_BIGENDIAN -D__DARWIN_BYTE_ORDER=__DARWIN_LITTLE_ENDIAN -U__BIG_ENDIAN__" ], 
	       [ BE_CFLAGS="-DWORDS_BIGENDIAN"; LE_CFLAGS= ])
AC_SUBST(LE_CFLAGS)
AC_SUBST(BE_CFLAGS)

# ?? AM_INIT_AUTOMAKE

# check for system
AC_CANONICAL_BUILD
AC_SUBST(build_cpu)
AC_MSG_RESULT([System build type is: $build])

# Platform-specific CC flags
# TODO: missing Mac, Mac-Intel, Mac-AMD64 and Windows 32/64 platforms here
case $build in
  i?86-*-*)	PLAT_CFLAGS="-malign-double"; MEX_EXT=mexglx;; #-m32 
  x86_64-*-*)	PLAT_CFLAGS=; MEX_EXT=mexa64;;
  *)		PLAT_CFLAGS=; AC_MSG_FAILURE([System type not recognized, assuming x86_64 compatible]);;
esac
AC_SUBST(PLAT_CFLAGS)
AC_SUBST(MEX_EXT)

# AC_SUBST(PLAT_CFLAGS, "-DWIN32")
# what about windows?

#AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
