/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ed.h"
#include "debug.h"
#include "screen.h"
#include "go32.h"
#include "dos.h"
#include <debug/syms.h>
#include <sys/system.h>
#include <libc/file.h>	/* Workaround for stderr bug below */

extern char *source_path;
extern char *setupfilename;
/* ------------------------------------------------------------------------- */
static void
usage (char *myself)
{
  fprintf (stderr, "\
Usage: %s [debug args] debug-image [image args]\n\
\n\
Options:  -p path    Specify path for source files.\n\
          -d         Enable dual monitor display.\n\
          -s file    Load setup from specified file.\n\
", myself);
  exit (1);
}
/* ------------------------------------------------------------------------- */
#ifndef V2DBG
/* Snarf the environment from the child process.  */

static inline void
snarf_environment ()
{
  int i, j, k, env0;

  read_child (a_tss.tss_esp + 8, &env0, 4);
  for (j = 0; j < 2; j++)
    {
      for (i = 0; ; i++)
	{
	  int ep;

	  read_child (env0 + i * 4, &ep, 4);
	  if (!ep)
	    {
	      if (j)
		environ[i] = 0;
	      else
		environ = (char **) xmalloc ((i + 1) * sizeof (char *));
	      break;
	    }
	  if (j)
	    {
	      char ch;
	      for (k = 0; (read_child (ep + k, &ch, 1), ch); k++);
	      environ[i] = (char *) xmalloc (k + 1);
	      read_child (ep, environ[i], k + 1);
	    }
	}
    }
}
#endif
/* ------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
  int i;
  char *dotptr;
  char *fname;
  const _v2_prog_type *prog;
  char cmdline[128];
  jmp_buf start_state;
  int argno;

#ifndef V2DBG
  /* Under version 1.xx the debugger is not given an environment.  */
  if (!environ)
    snarf_environment ();
#endif

  /* Where to find source files.  */
  source_path = getenv ("FSDBPATH");

  /* Where to load and save setups.  */
  setupfilename = "fsdb.dsk";

  /* Use mono display for debugger, colour for child?  */
  dual_monitor_p = (getenv ("FSDBDUAL") != 0);

  argno = 1;
  while (argno < argc && argv[argno][0] == '-')
    {
      if (argv[argno][1] == 'p' && argv[argno][2] == 0)
	{
	  if (++argno == argc) usage (argv[0]);
	  source_path = argv[argno++];
	}
      else if (argv[argno][1] == 's' && argv[argno][2] == 0)
	{
	  if (++argno == argc) usage (argv[0]);
	  setupfilename = argv[argno++];
	}
      else if (argv[argno][1] == 'd' && argv[argno][2] == 0)
	{
	  argno++;
	  dual_monitor_p = 1;
	}
      else
	usage (argv[0]);
    }
  if (argno == argc) usage (argv[0]);

  if (dual_monitor_p && ScreenSecondary == 0)
    {
      fprintf (stderr, "%s: dual monitor requested but not available.\n",
	       argv[0]);
      exit (1);
    }

  fname = alloca(strlen(argv[argno] + 5));
  strcpy(fname,argv[argno]);

  prog = _check_v2_prog(fname,-1);
  if (!prog->valid)
    {
      /* Try adding the .exe extension to it and try again. */
      dotptr = rindex(fname,'.');
      if (_USE_LFN || dotptr == NULL)
       strcat(fname,".exe");
      else
       if (dotptr < rindex(fname,'/') || dotptr < rindex(fname,'\\'))
         strcat(fname,".exe");
    }
  prog = _check_v2_prog(fname,-1);
  if (!prog->valid || prog->object_format!=_V2_OBJECT_FORMAT_COFF)
    {
      printf("Load failed for image %s\n",argv[1]);
      exit(1);
    }
  syms_init(fname);

  cmdline[1] = 0;
  for (i = argno + 1; argv[i]; i++) {
    strcat(cmdline+1, " ");
    strcat(cmdline+1, argv[i]);
  }
  i = strlen(cmdline+1);
  cmdline[0] = i;
  cmdline[i+1] = 13;
  if (v2loadimage(fname,cmdline,start_state))
    {
      printf("Load failed for image %s\n",argv[1]);
      exit(1);
    }

  edi_init(start_state);
  stdout->_file = dup(fileno(stdout));

  debugger();

  return 0;
}
