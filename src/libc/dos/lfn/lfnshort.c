#include <libc/stubs.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>

static int
msdos_toupper_fname (int c)
{
  return (c >= 'a' && c <= 'z') ? toupper (c) : c;
}

char *
_lfn_gen_short_fname (const char *long_fname, char *short_fname)
{
  __dpmi_regs r;
  unsigned long tbuf = __tb & 0xfffff;

  r.x.ax = 0x7100;
  if (_USE_LFN)
    {
      dosmemput (long_fname, strlen (long_fname) + 1, tbuf);
      r.x.ax = 0x71a8;
      r.x.ds = tbuf >> 4;
      r.x.si = 0;
      r.x.es = r.x.ds;
      r.x.di = 260;
      r.x.dx = 0x0011;	/* DH=01 would be better, but it's buggy */
      __dpmi_int (0x21, &r);
    }

  if ((r.x.flags & 1) == 0 && r.x.ax != 0x7100)
    {
      char buf[13], *s = buf, *d = short_fname;

      dosmemget (tbuf + 260, sizeof buf, buf);

      /* The short name in BUF is ASCIZ in the FCB format.
	 Convert to 8.3 filename.  */
      while (s - buf < 8 && *s && *s != ' ')
	*d++ = *s++;
      while (*s && *s == ' ')
	s++;
      if (*s)
	{
	  *d++ = '.';
	  while (*s && *s != ' ')
	    *d++ = *s++;
	}
      *d = '\0';
    }
  else
    {
      const char *s = long_fname;
      char *d = short_fname;

      while ((*d++ = msdos_toupper_fname (*s++)))
	if (d - short_fname >= 12)
	  {
	    *d = '\0';
	    break;
	  }
    }
  return short_fname;
}

#ifdef TEST

#include <stdio.h>

int main (int argc, char *argv[])
{
  char sh[13];
  if (argc > 1)
    printf ("Orig:  %s\nShort: %s\n",
	    argv[1], _lfn_gen_short_fname(argv[1], sh));
  return 0;
}

#endif
