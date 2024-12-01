# Process this file with autoconf to produce a configure script.
define(CW_VERSION_MAJOR, 2)
define(CW_VERSION_MINOR, 22)

# Because we use cwm4.
define(CW_PACKAGE_NAME, [which])
define(CW_BUGREPORT, [which-bugs@gnu.org])

# CW_COMPILE_FLAGS may contain warning flags, but also default debug flag (-g*) and/or optimization flag (-O*)
# which will be stripped when not required, and an optional -std=* flag.
define(CW_COMPILE_FLAGS, [-W -Wall -Woverloaded-virtual -Wundef -Wpointer-arith -Wwrite-strings -Winline])
# CW_THREADS can be [no] (single-threaded), [yes] (multi-threaded) or [both] (single and multi-threaded applications).
define(CW_THREADS, [no])
# CW_MAX_ERRORS is the maximum number of errors the compiler will show.
define(CW_MAX_ERRORS, 2)

# Include cwm4 header.
m4_include([cwm4/configure_ac_top.m4])

# Checks for programs.
AC_PROG_INSTALL
AC_LANG([C])

AC_CONFIG_SRCDIR(which.c)

which_cv_iberty=yes
AC_ARG_ENABLE(iberty,
  [  --disable-iberty        do not try to find and use the iberty library],
  which_cv_iberty=$enableval)

# Checks for libraries.
if test "$which_cv_iberty" = yes ; then
  # Check for xmalloc in libiberty
  AC_CHECK_LIB([iberty], [xmalloc],
    [AC_MSG_NOTICE([xmalloc function found in libiberty])
     LIBS="$LIBS -liberty"],
    [AC_DEFINE([NEED_XMALLOC], [1], [Define if xmalloc is not available in libiberty])])
fi

# Checks for header files.
AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP

AC_CHECK_HEADERS(unistd.h string.h libiberty.h)
AC_HEADER_STAT

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_TYPE_GETGROUPS
AC_CHECK_FUNCS(strchr memcpy getopt_long getcwd getwd sysconf getgroups eaccess)

# Make sure we use 64-bit file API on 32-bit systems.
AC_SYS_LARGEFILE

# Additional stuff needed for the tilde directory
AC_CHECK_PROG(AR, ar, ar, ar)
AC_PROG_RANLIB
AC_CHECK_HEADERS(stdlib.h memory.h pwd.h strings.h)
AC_CHECK_FUNCS(getpwent getpwnam getpwuid)
BASH_CHECK_GETPW_FUNCS

# Generate output files.
AC_CONFIG_FILES([maintMakefile])

# Include cwm4 footer.
m4_include([cwm4/configure_ac_bottom.m4])
