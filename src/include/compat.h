#ifndef COMPAT_H
#define COMPAT_H

/* adapted from compat.h in git-1.6.1.2 */

#if !defined(__APPLE__) && !defined(__FreeBSD__)  && !defined(__USLC__) && !defined(_M_UNIX)
#  define _XOPEN_SOURCE 600 /* glibc2 and AIX 5.3L need 500, OpenBSD needs 600 for S_ISLNK() */
#  define _XOPEN_SOURCE_EXTENDED 1 /* AIX 5.3L needs this */
#endif
#define _ALL_SOURCE 1
#define _GNU_SOURCE 1
#define _BSD_SOURCE 1

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <dirent.h>

#ifndef __WIN32__
#  include <dirent.h>
#  include <sys/utsname.h>
#  include <sys/ipc.h>
#  include <sys/msg.h>
#endif

#include "compat-mingw.h"

#ifdef __MINGW32__
#	if FLDIGI_FLTK_API_MAJOR == 1 && FLDIGI_FLTK_API_MINOR < 3
#		undef dirent
#		include <dirent.h>
#	else
#		include <dirent.h>
#	endif
#define WSA_MAJOR 2
#define WSA_MINOR 0
#else
#	include <dirent.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if SNPRINTF_RETURNS_BOGUS
#define snprintf git_snprintf
	extern int git_snprintf(char *str, size_t maxsize,
							const char *format, ...);
#define vsnprintf git_vsnprintf
	extern int git_vsnprintf(char *str, size_t maxsize,
							 const char *format, va_list ap);
#endif

#ifdef __cplusplus
}
#endif

#endif // MINGW32_H
