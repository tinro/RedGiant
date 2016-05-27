#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([redgiant], [1.0])
AC_CONFIG_SRCDIR([src/main/main.cc])

AM_INIT_AUTOMAKE
AM_PATH_CPPUNIT(1.13)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

CFLAGS+=" -std=c11 -Werror"
CXXFLAGS+=" -std=c++11 -Werror"

# Checks for libraries.
AC_CHECK_LIB(['event'], ['event_init'])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
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
                 src/main/data/Makefile
                 src/main/handler/Makefile
                 src/main/index/Makefile
                 src/main/query/Makefile
                 src/main/ranking/Makefile
                 src/main/service/Makefile
                 src/test/Makefile
                 src/test/core_index/Makefile
                 src/test/core_reader/Makefile
                 src/test/data/Makefile
                 src/test/handler/Makefile
                 src/test/index/Makefile
                 src/test/query/Makefile
                 src/test/ranking/Makefile
                 src/test/service/Makefile
                 src/test/utils/Makefile])
AC_OUTPUT
