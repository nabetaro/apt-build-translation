/*
 * config.c - Wrapper configuration for gcc used for apt-build
 * (c) 2005-2008 - Julien Danjou <acid@debian.org>
 * (c) 2012      - Dominique Lasserre <lasserre.d@gmail.com>
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

#include <ctype.h>
#include <libgen.h>

#include "config.h"
#include "apt-build.h"

char
rm_trail_whtsp (char * argument)
{
  char * tmpstr = argument;
  int l = strlen (tmpstr);

  while (* tmpstr && isspace (* tmpstr))
    tmpstr++, l--;

  memmove (argument, tmpstr, l + 1);

  /* Return 'true' if chars were moved */
  if (l == 0)
    return 0;
  else
    return 1;
}

char
rm_matching_chars (char * argument, const char c)
{
  unsigned int i, j = 0;
  for (i = 0; i < strlen (argument); i++)
    {
      if (argument[i] == c || argument[i] == '\\' )
        {
          if (argument[i] == c || (argument[i] == '\\' && argument[i+1] == c ))
            j++;
          memmove (&argument[i], &argument[i+1], strlen (argument) - i);
        }
    }
  //printf("j:%d||arg:%s\n",j,argument);

  /* Return 'true' if matching quotes were found */
  if (j % 2 == 0)
    return 1;
  else
    return 0;
}

static char *
parse_options (char * arguments)
{
  unsigned int i;
  char *result;
  char c[] = {'\"', '\''};

  for (i = 0; i < 2; i++)  // 2 = length of c[]
    if (!rm_matching_chars (arguments, c[i]))
      {
        fprintf (stderr, "[apt-build-wrapper]: Error parsing options, check configuration file.\n");
        exit (EXIT_FAILURE);
      }

  result = strdup (arguments);
  return result;
}

char **
parse_conf (unsigned int argc, char **argv)
{
  FILE *conf;
  unsigned int i = 0, nb_apt_build_options = 0;
  int is_gcc = 0;
  char *file_content;
  char buf[BUF_SIZE];
  char opt[BUF_SIZE];
  char *str;
  char **cmd_line_args;
  struct apt_build_args args;
  char *options = NULL;

  /* Reset the configuration */
  memset (&args, 0, sizeof (args));

  conf = fopen (APT_BUILD_CONF_PATH, "r");

  if (!conf)
    {
      perror ("Unable to open apt-build configuration file");
      exit (EXIT_FAILURE);
    }

  file_content = (char *) malloc (sizeof (char) * BUF_SIZE);

  while (fgets (file_content, BUF_SIZE, conf))
    {
      *buf = '\0';
      /* Remove trailing whitespaces */
      rm_trail_whtsp (file_content);
      if (*file_content == '\0')
        continue;

      if (sscanf (file_content, "%[^=]=%[^\n]",opt,buf))
        {
          //printf("||%s(%d)||%s||\n",opt,(int) strlen(opt),buf);

          /* parse options */
          if (rm_trail_whtsp (buf))
            {
              if (!strcmp (opt, "Olevel"))
                args.Olevel = parse_options (buf);
              else if (!strcmp (opt, "mtune"))
                args.mtune = parse_options (buf);
              else if (!strcmp (opt, "options"))
                args.options = parse_options (buf);
              else if (!strcmp (opt, "make_options"))
                args.make_options = parse_options (buf);
            }
        }
    }
  fclose (conf);
  free (file_content);

  /* Build the new command line */
  cmd_line_args =
    (char **) malloc (sizeof (char *) * (argc + MAX_ARGC + 1));

  cmd_line_args[nb_apt_build_options++] = strdup (argv[0]);

  /* make options */
  if(!strcmp (basename(argv[0]), "make"))
    options = args.make_options;
  else
    {
      options = args.options;
      is_gcc = 1;
    }

  /* Apply options as specified by the configuration file. */
  //TODO: proper splitting of option string - preserve quotes
  if (options &&
      strlen (options) &&
      (str = strtok (options, " ")))
    {
      do
      {
        cmd_line_args = (char **)
        realloc (cmd_line_args,
             sizeof (char *) * (argc + MAX_ARGC + nb_apt_build_options + 1));
        cmd_line_args[nb_apt_build_options++] = strdup (str);
      }
      while((str = strtok(NULL, " ")));
    }

  /* Copy the rest of the line */
  for(i = 1; i < argc; i++)
    cmd_line_args[nb_apt_build_options++] = strdup (argv[i]);

  /* Apply GCC options at the end to override the default options. */
  if (is_gcc)
    {
      if (args.Olevel)
        cmd_line_args[nb_apt_build_options++] = args.Olevel;
      if (args.mtune)
        cmd_line_args[nb_apt_build_options++] = args.mtune;
    } else {
      /* Add current path as argument to make */
      char *path = getenv("PATH");
      if(path)
        {
          size_t len = strlen(path) + 6;
          char *path_option = (char *) malloc (len * sizeof(char));
          snprintf(path_option, len, "PATH=%s",path);
          cmd_line_args[nb_apt_build_options++] = path_option;
        }
    }

  cmd_line_args[nb_apt_build_options++] = NULL;

#if 0
 for(i = 0; i < nb_apt_build_options; i++)
   printf("argv[%d] -> %s\n", i, cmd_line_args[i]);
#endif

  return cmd_line_args;
}

void
filterout_libdir_path (void)
{
  char *path;
  char *libdir;

  /* Filter out standard apt-build search directory. */
  path = getenv ("PATH");
  if (path)
  {
    libdir = strstr (path, LIBDIR);
    if (libdir)
      setenv ("PATH", libdir + strlen (LIBDIR) + 1, 1);
  }
}

// vim: set ai ts=2 sts=2 et sw=2 ft=c:
