/*
 * $Id$
 * gcc.wrapper - Wrapper for gcc used for apt-build
 * (c) 2005 - Julien Danjou <acid@debian.org>
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

char *
parse_options (char * file_content)
{
  unsigned int i, j;
  char *result;
  
  /* Search the first " */
  for (i = 0; i <= strlen (file_content) && file_content[i] != '"'; i++);
  file_content += i + 1;

  /* Remove the first spaces if they exist */
  while (file_content[0] == ' ')
    file_content++;
  
  /* Search the last " */
  for (j = strlen (file_content); j >= 1 && file_content[j - 1] != '"'; j--);
  
  if (j == 0)
    {
      fprintf (stderr, "Error parsing options, check configuration file.\n");
      exit (EXIT_FAILURE);
    }
  
  if (strlen (file_content) > 2)
    {
      result = strndup (file_content, j - 1);
      return result;
    }

  return NULL;
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
      if (sscanf (file_content, "%s = %s", opt, buf))
	{
	  if (!strncmp (opt, "Olevel", 6))
	    args.Olevel = strdup (buf);

	  if (!strncmp (opt, "mtune", 4))
	    args.mtune = strdup (buf);

	  if (!strncmp (opt, "options", 7))
	    args.options = parse_options (file_content);

	  if (!strncmp (opt, "make_options", 13))
	    args.make_options = parse_options (file_content);
	}
    }
  fclose (conf);
  free (file_content);

  /* Build the new command line */
  cmd_line_args =
    (char **) malloc (sizeof (char *) * (argc + MAX_ARGC + 1));

  cmd_line_args[nb_apt_build_options++] = strdup (argv[0]);

  /* make options */
  if(!strcmp(basename(argv[0]), "make"))
    {
      options = args.make_options;
    }
  else
    {
      options = args.options;
	  is_gcc = 1;
    }
	
  /* Apply options as specified by the configuration file. */
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
