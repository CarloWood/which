/*
 * which v2.x -- print full path of executables
 * Copyright (C) 1999  Carlo Wood <carlo@gnu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "sys.h"
#include <stdio.h>
#include "getopt.h"
#include "tilde/tilde.h"
#include "bash.h"

static const char *progname;

static void print_usage(void)
{
  fprintf(stderr, "Usage: %s [options] [--] programname [...]\n", progname);
  fprintf(stderr, "Options: --version, -[vV] Print version and exit successfully.\n");
  fprintf(stderr, "         --help,          Print this help and exit successfully.\n");
  fprintf(stderr, "         --skip-dot       Skip directories in PATH that start with a dot.\n");
  fprintf(stderr, "         --skip-tilde     Skip directories in PATH that start with a tilde.\n");
  fprintf(stderr, "         --show-dot       Don't expand a dot to current directory in output.\n");
  fprintf(stderr, "         --show-tilde     Output a tilde for HOME directory for non-root.\n");
  fprintf(stderr, "         --tty-only       Stop processing options on the right if not on tty.\n");
  fprintf(stderr, "         --all, -a        Print all matches in PATH, not just the first\n");
  fprintf(stderr, "         --read-alias, -i Read list of aliases from stdin.\n");
  fprintf(stderr, "         --skip-alias     Ignore option --read-alias; don't read stdin.\n");
}

static void print_version(void)
{
  fprintf(stdout, "GNU which v" VERSION ", Copyright (C) 1999 Carlo Wood.\n");
  fprintf(stdout, "GNU which comes with ABSOLUTELY NO WARRANTY;\n");
  fprintf(stdout, "This program is free software; your freedom to use, change\n");
  fprintf(stdout, "and distribute this program is protected by the GPL.\n");
}

static void print_fail(const char *name, const char *path_list)
{
  fprintf(stderr, "%s: no %s in (%s)\n", progname, name, path_list);
}

static char home[256];
static size_t homelen = 0;

static int absolute_path_given;
static int found_path_starts_with_dot;
static char *abs_path;

static int skip_dot = 0, skip_tilde = 0, skip_alias = 0, read_alias = 0;
static int show_dot = 0, show_tilde = 0, show_all = 0, tty_only = 0;

static char *find_command_in_path(const char *name, const char *path_list, int *path_index)
{
  char *found = NULL, *full_path;
  int status, name_len;

  name_len = strlen(name);

  if (!absolute_program(name))
    absolute_path_given = 0;
  else
  {
    char *p;
    absolute_path_given = 1;

    if (abs_path)
      free(abs_path);

    if (*name != '.' && *name != '/' && *name != '~')
    {
      abs_path = (char *)xmalloc(3 + name_len);
      strcpy(abs_path, "./");
      strcat(abs_path, name);
    }
    else
    {
      abs_path = (char *)xmalloc(1 + name_len);
      strcpy(abs_path, name);
    }

    path_list = abs_path;
    p = strrchr(abs_path, '/');
    *p++ = 0;
    name = p;
  }

  while (path_list && path_list[*path_index])
  {
    char *path;

    if (absolute_path_given)
    {
      path = savestring(path_list);
      *path_index = strlen(path);
    }
    else
      path = get_next_path_element(path_list, path_index);

    if (!path)
      break;

    if (*path == '~')
    {
      char *t = tilde_expand(path);
      free(path);
      path = t;

      if (skip_tilde)
      {
	free(path);
	continue;
      }
    }

    if (skip_dot && *path != '/')
    {
      free(path);
      continue;
    }

    found_path_starts_with_dot = (*path == '.');

    full_path = make_full_pathname(path, name, name_len);
    free(path);

    status = file_status(full_path);

    if ((status & FS_EXISTS) && (status & FS_EXECABLE))
    {
      found = full_path;
      break;
    }

    free(full_path);
  }

  return (found);
}

static char cwd[256];
static size_t cwdlen;

static void get_current_working_directory(void)
{
  if (cwdlen)
    return;

  if (!getcwd(cwd, sizeof(cwd)))
  {
    const char *pwd = getenv("PWD");
    if (pwd && strlen(pwd) < sizeof(cwd))
      strcpy(cwd, pwd);
  }

  if (*cwd != '/')
  {
    fprintf(stderr, "Can't get current working directory\n");
    exit(-1);
  }

  cwdlen = strlen(cwd);

  if (cwd[cwdlen - 1] != '/')
  {
    cwd[cwdlen++] = '/';
    cwd[cwdlen] = 0;
  }
}

static char *path_clean_up(const char *path)
{
  static char result[256];

  const char *p1 = path;
  char *p2 = result;

  int saw_slash = 0, saw_slash_dot = 0, saw_slash_dot_dot = 0;

  if (*p1 != '/')
  {
    get_current_working_directory();
    strcpy(result, cwd);
    saw_slash = 1;
    p2 = &result[cwdlen];
  }

  do
  {
    if (!saw_slash || *p1 != '/')
      *p2++ = *p1;
    if (saw_slash_dot && (*p1 == '/'))
      p2 -= 2;
    if (saw_slash_dot_dot && (*p1 == '/'))
    {
      int cnt = 0;
      do
      {
	if (--p2 < result)
	{
	  strcpy(result, path);
	  return result;
	}
	if (*p2 == '/')
	  ++cnt;
      }
      while (cnt != 3);
      ++p2;
    }
    saw_slash_dot_dot = saw_slash_dot && (*p1 == '.');
    saw_slash_dot = saw_slash && (*p1 == '.');
    saw_slash = (*p1 == '/');
  }
  while (*p1++);

  return result;
}

int path_search(int indent, const char *cmd, const char *path_list)
{
  char *result = NULL;
  int found_something = 0;

  if (path_list && *path_list != '\0')
  {
    int next;
    int path_index = 0;
    do
    {
      next = show_all;
      result = find_command_in_path(cmd, path_list, &path_index);
      if (result)
      {
	const char *full_path = path_clean_up(result);
	int in_home = (show_tilde || skip_tilde) && !strncmp(full_path, home, homelen);
	if (indent)
	  fprintf(stdout, "\t");
	if (!(skip_tilde && in_home) && show_dot && found_path_starts_with_dot && !strncmp(full_path, cwd, cwdlen))
	{
	  full_path += cwdlen;
	  fprintf(stdout, "./");
	}
	else if (in_home)
	{
	  if (skip_tilde)
	  {
	    next = 1;
	    continue;
	  }
	  if (show_tilde)
	  {
	    full_path += homelen;
	    fprintf(stdout, "~/");
	  }
	}
	fprintf(stdout, "%s\n", full_path);
	free(result);
	found_something = 1;
      }
      else
	break;
    }
    while (next);
  }

  return found_something;
}

void process_alias(const char *str, int argc, char *argv[], const char *path_list)
{
  const char *p = str;
  int len = 0;

  while(*p == ' ' || *p == '\t')
    ++p;
  if (!strncmp("alias", p, 5))
    p += 5;
  while(*p == ' ' || *p == '\t')
    ++p;
  while(*p && *p != ' ' && *p != '\t' && *p != '=')
    ++p, ++len;

  for (; argc > 0; --argc, ++argv)
  {
    char q = 0;
    char *cmd;

    if (!*argv || len != strlen(*argv) || strncmp(*argv, &p[-len], len))
      continue;

    fputs(str, stdout);

    if (!show_all)
      *argv = NULL;

    while(*p == ' ' || *p == '\t')
      ++p;
    if (*p == '=')
      ++p;
    while(*p == ' ' || *p == '\t')
      ++p;
    if (*p == '"' || *p == '\'')
      q = *p, ++p;

    for(;;)
    {
      while(*p == ' ' || *p == '\t')
	++p;
      len = 0;
      while(*p && *p != ' ' && *p != '\t' && *p != q && *p != '|' && *p != '&')
	++p, ++len;

      cmd = (char *)xmalloc(len + 1);
      strncpy(cmd, &p[-len], len);
      cmd[len] = 0;
      if (*argv && !strcmp(cmd, *argv))
        *argv = NULL;
      path_search(2, cmd, path_list);
      free(cmd);

      while(*p && (*p != '|' || p[1] == '|') && (*p != '&' || p[1] == '&'))
        ++p;

      if (!*p)
        break;

      ++p;
    }

    break;
  }
}

enum opts {
  opt_version,
  opt_skip_dot,
  opt_skip_tilde,
  opt_skip_alias,
  opt_show_dot,
  opt_show_tilde,
  opt_tty_only,
  opt_help
};

int main(int argc, char *argv[])
{
  const char *path_list = getenv("PATH");
  int short_option, fail_count = 0;
  static int long_option;
  struct option longopts[] = {
    {"help", 0, &long_option, opt_help},
    {"version", 0, &long_option, opt_version},
    {"skip-dot", 0, &long_option, opt_skip_dot},
    {"skip-tilde", 0, &long_option, opt_skip_tilde},
    {"show-dot", 0, &long_option, opt_show_dot},
    {"show-tilde", 0, &long_option, opt_show_tilde},
    {"tty-only", 0, &long_option, opt_tty_only},
    {"all", 0, NULL, 'a'},
    {"read-alias", 0, NULL, 'i'},
    {"skip-alias", 0, &long_option, opt_skip_alias},
    {NULL, 0, NULL, 0}
  };

  progname = argv[0];
  while ((short_option = getopt_long(argc, argv, "aivV", longopts, NULL)) != -1)
  {
    switch (short_option)
    {
      case 0:
	switch (long_option)
	{
	  case opt_help:
	    print_usage();
	    return 0;
	  case opt_version:
	    print_version();
	    return 0;
	  case opt_skip_dot:
	    skip_dot = !tty_only;
	    break;
	  case opt_skip_tilde:
	    skip_tilde = !tty_only;
	    break;
	  case opt_skip_alias:
	    skip_alias = 1;
	    break;
	  case opt_show_dot:
	    show_dot = !tty_only;
	    break;
	  case opt_show_tilde:
	    show_tilde = (!tty_only && geteuid() != 0);
	    break;
	  case opt_tty_only:
	    tty_only = !isatty(1);
	    break;
	}
	break;
      case 'a':
	show_all = 1;
	break;
      case 'i':
        read_alias = 1;
	break;
      case 'v':
      case 'V':
	print_version();
	return 0;
    }
  }

  if (show_dot)
    get_current_working_directory();

  if (show_tilde || skip_tilde)
  {
    const char *h;

    if (!(h = getenv("HOME")))
    {
      fprintf(stderr, "%s: ", progname);
      if (show_tilde)
	fprintf(stderr, "--show-tilde");
      else
	fprintf(stderr, "--skip-tilde");
      fprintf(stderr, ": Environment variable HOME not set\n");
      show_tilde = skip_tilde = 0;
    }
    else
    {
      strncpy(home, h, sizeof(home));
      home[sizeof(home) - 1] = 0;
      homelen = strlen(home);
      if (home[homelen - 1] != '/' && homelen < sizeof(home) - 1)
      {
	strcat(home, "/");
	++homelen;
      }
    }
  }

  if (skip_alias)
    read_alias = 0;

  argv += optind;
  argc -= optind;

  if (argc == 0)
  {
    print_usage();
    return -1;
  }

  if (read_alias)
  {
    char buf[1024];

    if (isatty(0))
      fprintf(stderr, "%s: --read-alias, -i: Warning: stdin is a tty.\n", progname);

    while (fgets(buf, sizeof(buf), stdin))
      process_alias(buf, argc, argv, path_list);
  }

  for (; argc > 0; --argc, ++argv)
  {
    if (!*argv)
      continue;

    if (!path_search(0, *argv, path_list))
    {
      print_fail(absolute_path_given ? strrchr(*argv, '/') + 1 : *argv, absolute_path_given ? abs_path : path_list);
      ++fail_count;
    }
  }

  return fail_count;
}

#ifdef NEED_XMALLOC
void *xmalloc(size_t size)
{
  void *ptr = malloc(size);
  if (ptr == NULL)
  {
    fprintf(stderr, "%s: Out of memory", progname);
    exit(-1);
  }
  return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
  if (!ptr)
    return xmalloc(size);
  ptr = realloc(ptr, size);
  if (size > 0 && ptr == NULL)
  {
    fprintf(stderr, "%s: Out of memory\n", progname);
    exit(-1);
  }
  return ptr;
}
#endif /* NEED_XMALLOC */
