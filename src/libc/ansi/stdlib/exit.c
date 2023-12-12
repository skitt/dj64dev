/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <libc/internal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dos.h>
#include <io.h>
#include <libc/atexit.h>
#include <libc/stdiohk.h>

struct __atexit *__atexit_ptr = 0;

extern void (*__stdio_cleanup_hook)(void); /* stdio/stdiohk.c */

/* A hook to close down the file system extensions if any where opened.
   This does not force them to be linked in. */
void (*__FSEXT_exit_hook)(void) = NULL;

/* A hook to close those file descriptors with properties. */
void (*__fd_properties_cleanup_hook)(void) = NULL;

void
exit(int status)
{
//  int i;
  struct __atexit *a,*o;

  a = __atexit_ptr;
  __atexit_ptr = 0; /* to prevent infinite loops */
  while (a)
  {
    (a->__function)();
    o = a;
    a = a->__next;
    free(o);
  }
#if 0
  /* Destructors should probably be called after functions registered
     with atexit(), this is the way it happens in Linux anyway. */
  for (i=0; i<djgpp_last_dtor-djgpp_first_dtor; i++)
    _djgpp_first_dtor(i);
#endif
  /* Do this last so that everyone else may write to files
     during shutdown */
  if (__stdio_cleanup_hook)
    __stdio_cleanup_hook();

  if (__fd_properties_cleanup_hook)
    __fd_properties_cleanup_hook();

  /* Do this after the stdio cleanup to let it close off the fopen'd files */
  if (__FSEXT_exit_hook)
    __FSEXT_exit_hook();

  /* in case the program set it this way */
  setmode(0, O_TEXT);
  _exit(status);
}
