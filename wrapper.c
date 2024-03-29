/*
 * wrapper.c - Wrapper for gcc and make used for apt-build
 * (c) 2004-2008 - Julien Danjou <acid@debian.org>
 * (c) 2005 - Raphael Bossek <bossekr@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "apt-build.h"
#include "config.h"

static int
gcc_real (char **argv)
{
  filterout_libdir_path();
  argv[0] = basename (argv[0]) ;
  return execvp (argv[0], argv);
}

static int
gcc_apt_build (int argc, char **argv)
{
  return gcc_real (parse_conf (argc, argv));
}

int
main (int argc, char **argv)
{
  if (getenv (APT_BUILD_STRING_ENV))
    return gcc_apt_build (argc, argv);

  return gcc_real (argv);
}

// vim: set ai ts=2 sts=2 et sw=2 ft=c:
