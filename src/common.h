#ifndef COMMON_H_HEADER_GUARD
#define COMMON_H_HEADER_GUARD

#include <stdio.h>  /* fprintf, stderr */
#include <stdlib.h> /* size_t, malloc, realloc, free, exit, EXIT_FAILURE */
#include <stdarg.h> /* va_list, va_start, va_end, va_arg */

#define Fatal(...)  FatalFrom(__FILE__, __LINE__, __VA_ARGS__)
#define Info(...)   InfoFrom(__FILE__, __LINE__, __VA_ARGS__)

#ifdef DEBUG
#	define Assert(X, ...)                                         \
	do {                                                          \
		if (!(X))                                                 \
			Fatal("Assertion \"" #X "\" failed: " __VA_ARGS__); \
	} while (0)
#else
#	define Assert(X, ...)
#endif

void FatalFrom(const char *path, int line, const char *fmt, ...);
void InfoFrom (const char *path, int line, const char *fmt, ...);

#endif
