/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSCALL_H
#define __SYSCALL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

/* Exported types ------------------------------------------------------------*/
typedef int (*SYSWRITEHANDLER)(char c);
typedef int (*SYSREADHANDLER)();

/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */ 
void SYSCALL_Init_STDOUT_Handler(SYSWRITEHANDLER handler);
void SYSCALL_Init_STDERR_Handler(SYSWRITEHANDLER handler);
void SYSCALL_Init_STDIN_Handler(SYSREADHANDLER handler);

void initialise_monitor_handles _PARAMS ((void));

caddr_t _sbrk     _PARAMS ((int));
int     _write    _PARAMS ((int, char *, int));
int     _close    _PARAMS ((int));
int     _fstat    _PARAMS ((int, struct stat *));
int     _isatty   _PARAMS ((int));
int     _lseek    _PARAMS ((int, int, int));
int     _kill     _PARAMS ((int pid, int sig));
int     _getpid   _PARAMS ((int n));
int     _read     _PARAMS ((int file, char * ptr, int len));
int _gettimeofday _PARAMS ((struct timeval * tp, void * tzvp));

#ifdef __cplusplus
}
#endif

#endif /* __SYSCALL_H */
