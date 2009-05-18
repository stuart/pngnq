# Autoconf for pngnq

AC_INIT([pngnq],[1.0], [stuart.coyle@gmail.com])
AM_INIT_AUTOMAKE[-Wall -Werror]
AC_COPYRIGHT([Copyright 2008-2009, Stuart Coyle])

# check for progs
AC_CHECK_PROG([HAS_LIBPNG_CONFIG],[libpng-config],[1])

# checks for libraries

# checks for header files
AC_CHECK_HEADERS([ctype.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/time.h]) 
AC_CHECK_HEADERS([valgrind/callgrind.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([unistd.h])
                   
# checks for compiler characteristics
AC_PROG_CC
AC_C_CONST
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_HEADER_STDBOOL

# checks for library functions
AC_CHECK_FUNCS([getopt])
AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset]) 
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strrchr])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/Makefile freegetopt/Makefile])
AC_OUTPUT
