AC_DEFUN([BASH_CHECK_GETPW_FUNCS],
[AC_MSG_CHECKING(whether getpw functions are declared in pwd.h)
AC_CACHE_VAL(bash_cv_getpw_declared,
[AC_EGREP_CPP(getpwuid,
[
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <pwd.h>
],
bash_cv_getpw_declared=yes,bash_cv_getpw_declared=no)])
AC_MSG_RESULT($bash_cv_getpw_declared)
if test $bash_cv_getpw_declared = yes; then
AC_DEFINE([HAVE_GETPW_DECLS], [], [Define if getpw functions are declared in pwd.h])
fi
])
