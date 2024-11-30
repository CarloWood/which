#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr(), *strrchr();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#ifdef HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
#endif

#ifdef STAT_MACROS_BROKEN
# include "posixstat.h"
#endif

#if STDC_HEADERS
# include <stdlib.h>
#endif

#if defined(HAVE_LIBIBERTY_H) && !defined(NEED_XMALLOC)
#include <libiberty.h>		/* Part of libiberty.a that comes with binutils */
#else
extern void *xmalloc(size_t);
extern void *xrealloc(void *ptr, size_t size);
#define XMALLOC_DEFINED 1
#endif

#ifndef NULL
# ifdef __STDC__
#   define NULL ((void *)0)
# else
#   define NULL (0x0)
# endif
#endif

#ifndef HAVE_GETCWD
# ifdef HAVE_GETWD
#   define getcwd(a, b) getwd(a)
# else
#   error "You either need getcwd(3) or getwd(3)"
# endif
#endif

#ifdef _WIN32

#define WIN32SYSTEM			1

#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

#define stat				_stati64
#define getcwd				_getcwd
#define strdup				_strdup
#define isatty				_isatty
#define strtok_r            strtok_s

#define PATH_SEPARATOR		';'
#define DIR_SEPARATOR		'\\'
#define IS_SLASH(c)			((c) == '/' || (c) == DIR_SEPARATOR)
#define SLASHES			"\\/"
#define HAVE_DRIVE(n)		((n)[0] && (n)[1] == ':')
#define IS_ABSOLUTE(n)        (IS_SLASH((n)[0]) || ((n)[0] && (n)[1] == ':'))
#define DEFAULT_HOMEDIR		"C:\\"
#define STRSTR stristr
#define STRCMP stricmp
#define STRNCMP strnicmp
#define CHAR_CMP(c1, c2) ( tolower(c1) == tolower(c2) )
#else
#define WIN32SYSTEM			0
#define PATH_SEPARATOR		':'
#define DIR_SEPARATOR		'/'
#define IS_SLASH(c)			((c) == DIR_SEPARATOR)
#define SLASHES			"/"
#define HAVE_DRIVE(n)         (0)
#define IS_ABSOLUTE(n)        ((n)[0] == DIR_SEPARATOR)
#define DEFAULT_HOMEDIR		"/"
#define STRSTR strstr
#define STRCMP strcmp
#define STRNCMP strncmp
#define CHAR_CMP(c1, c2) (c1 == c2)
#endif /* _WIN32  */
#define IS_DIRSEP			IS_SLASH

/* A function can be defined using prototypes and compile on both ANSI C
   and traditional C compilers with something like this:
	extern char *func PARAMS((char *, char *, int)); */

#if !defined (PARAMS)
#  if defined (__STDC__) || defined (__GNUC__) || defined (__cplusplus) || defined (PROTOTYPES)
#    define PARAMS(protos) protos
#  else
#    define PARAMS(protos) ()
#  endif
#endif
