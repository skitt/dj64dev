/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  char buf[1000];
  char fn[1000];
  int i;
  FILE *stubs, *as, *mk, *oh;

  /* Remove all the old files */
  for (i=0; ; i++)
  {
    sprintf(fn, "stub%04d.S", i);
    if (access(fn, F_OK))
      break;
    remove(fn);
  }
  mk = fopen("makefil2", "w");
  oh = fopen("makefile.oh", "w");
  fprintf(mk, "TOP=..\n\n");

  stubs = fopen(argv[1], "r");
  i = 0;
  while (fgets(buf, 1000, stubs))
  {
    if (strncmp(buf, "#define", 7))
      continue;
    sscanf(buf, "%*s %s", buf);
    if (strncmp(buf, "__dj_include", 10) == 0)
      continue;

    sprintf(fn, "stub%04d.S", i);
    as = fopen(fn, "w");
    fprintf(as, "\t.file \"%s.stub\"\n\t.global _%s\n_%s:\n\tjmp ___%s\n",
	    buf, buf, buf, buf);
    fclose(as);

    fprintf(mk, "SRC += %s\n", fn);
    fprintf(oh, "&/stub%04d.o ", i);

    i++;
  }
  fprintf(mk, "\ninclude $(TOP)/../makefile.inc\n");
  fprintf(oh, "\n");
  fclose(mk);
  fclose(oh);
  fclose(stubs);

  return 0;
}
