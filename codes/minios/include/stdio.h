#ifndef LIBC_STDIO_INCLUDED
#define LIBC_STDIO_INCLUDED
#include "stdarg.h"

int vsprintf(char *buf, const char *fmt, va_list arglist);
int sprintf( char *buffer, const char *fmt, ...);
int printf(char *fmt, ...);
int puts(char *str);
char* gets(char*buf);

#endif
