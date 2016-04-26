#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([redgiant], [1.0])
AC_CONFIG_SRCDIR([src/main/main.cc])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE
AM_PATH_CPPUNIT(1.13.2)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

CFLAGS+=" -std=c11"
CXXFLAGS+=" -std=c++11"

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset pow socket sqrt strstr strtol])

AC_CONFIG_FILES([Makefile
                 src/main/Makefile
                 src/main/handler/Makefile
                 src/main/service/Makefile
                 src/main/utils/Makefile
                 src/test/Makefile
                 src/test/core_index/Makefile
                 src/test/core_reader/Makefile
                 src/test/service/Makefile])
AC_OUTPUT
