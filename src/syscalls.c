/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
//#include <sys/times.h>
#include <errno.h>
#include <reent.h>
// #include <unistd.h>
// #include <sys/wait.h>
#include "syscall.h"

#define SWI_Exit                   0x11

/* Forward prototypes.  
int     _system     _PARAMS ((const char *));
int     _rename     _PARAMS ((const char *, const char *));
int     _isatty		_PARAMS ((int));
clock_t _times		_PARAMS ((struct tms *));
int     _gettimeofday	_PARAMS ((struct timeval *, void *));
int     _unlink		_PARAMS ((const char *));
int     _link 		_PARAMS ((void));
int     _stat 		_PARAMS ((const char *, struct stat *));
int     _fstat 		_PARAMS ((int, struct stat *));
int	_swistat	_PARAMS ((int fd, struct stat * st));
caddr_t _sbrk		_PARAMS ((int));
int     _getpid		_PARAMS ((int));
int     _close		_PARAMS ((int));
clock_t _clock		_PARAMS ((void));
int     _swiclose	_PARAMS ((int));
int     _open		_PARAMS ((const char *, int, ...));
int     _swiopen	_PARAMS ((const char *, int));
int     _write 		_PARAMS ((int, char *, int));
int     _swiwrite	_PARAMS ((int, char *, int));
int     _lseek		_PARAMS ((int, int, int));
int     _swilseek	_PARAMS ((int, int, int));
int     _read		_PARAMS ((int, char *, int));
int     _swiread	_PARAMS ((int, char *, int));
*/

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr __asm__ ("sp"); 

static SYSWRITEHANDLER _stdout_handler;
static SYSWRITEHANDLER _stderr_handler;
static SYSREADHANDLER _stdin_handler;

void
SYSCALL_Init_STDOUT_Handler(SYSWRITEHANDLER handler) {
  _stdout_handler = handler;
}

void
SYSCALL_Init_STDERR_Handler(SYSWRITEHANDLER handler) {
  _stderr_handler = handler;
}

void
SYSCALL_Init_STDIN_Handler(SYSREADHANDLER handler) {
  _stdin_handler = handler;
}

int
_kill (int pid, int sig)
{
  (void)pid; (void)sig;
  __asm__ ("swi %a0" :: "i" (SWI_Exit));
  return 0;
}

int
_getpid (int n)
{
  return 1;
  n = n;
}

int
_read (int fd,
       char * ptr,
       int len)
{
  if (fd != 0 || _stdin_handler == NULL) {
    errno = EBADF;
    return -1;
  }
  for (int i = 0; i < len; i++) {
    int res = _stdin_handler(ptr[i]);
    if (res == -1) {
      return i;
    }
    ptr[i] = (char)res;
  }
  return len;
}

/* fd, is a user file descriptor. */
int
_lseek (int fd,
  int ptr,
  int dir)
{
  return ptr;
}

/* fd, is a user file descriptor. */
int
_write (int    fd,
	char * ptr,
	int    len)
{
  SYSWRITEHANDLER handler = NULL;
  if (fd == 1) {
    handler = _stdout_handler;
  } else if (fd == 2) {
    handler = _stderr_handler;
  }
  if (handler == NULL) {
    errno = EBADF;
    return -1;    
  }
  int res;
  for (int i = 0; i < len; i++) {
    res = handler(ptr[i]);
    if (res == -1) {
      errno = EIO;
      return i;
    }
  }
  return len;
}

int
_open (const char * path, int flags, ...)
{
  // FIXME
  return -1;
}

/* fd, is a user file descriptor. */
int
_close (int fd)
{
  if (fd < 0 || fd > 2) {
    errno = EBADF;
    return -1;
  }
  /* Handle stderr == stdout. */
  if (fd == 0) {
    _stdin_handler = NULL;
  } else {
    int samedest = (_stderr_handler == _stdout_handler);
    if (fd == 1 || (fd == 2 && samedest)) {
      _stdout_handler = NULL;
    }
    if (fd == 2 || (fd == 1 && samedest)) {
      _stderr_handler = NULL;    
    }
  }
  return 0;
}

int __attribute__((weak))
_fstat (int fd, struct stat * st)
{
  memset (st, 0, sizeof (* st));
  if (fd < 0 || fd > 2) {
    errno = EBADF;
    return -1;
  }

  /* Always assume a character device,
     with 1024 byte blocks. */
  st->st_mode |= S_IFCHR;
  st->st_blksize = 1024;
  st->st_size = 0;
  return 0;
}

caddr_t
_sbrk (int incr)
{
  extern char end __asm__ ("end"); /* Defined by the linker.  */
  extern char limit __asm__ ("_heap_limit");
  static char * heap_end;
  static char * heap_limit;
  char * prev_heap_end;

  if (heap_end == NULL)
    heap_end = & end;
  if (heap_limit == NULL)
    heap_limit = & limit;

  prev_heap_end = heap_end;
  
  if (heap_end + incr > heap_limit)
    {
      extern void abort (void);
      _write (1, "_sbrk: Heap and stack collision\n", 32);
      abort ();
    }
  
  heap_end += incr;

  return (caddr_t) prev_heap_end;
}

int
_isatty (int fd)
{
  if (fd < 0 || fd > 2) {
    errno = EBADF;
    return -1;
  }
  return 1;
}

int
_gettimeofday (struct timeval * tp, void * tzvp)
{
  struct timezone *tzp = tzvp;
  if (tp)
    {
      tp->tv_sec = 0;
      tp->tv_usec = 0;
    }

  /* Return fixed data for the timezone.  */
  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }

  return 0;
}


