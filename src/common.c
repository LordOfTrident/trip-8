#include "common.h"

#include <SDL2/SDL.h>

void FatalFrom(const char *path, int line, const char *fmt, ...) {
	char    msg[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg), fmt, args);
	va_end(args);

	fprintf(stderr, "[FATAL] %s:%i: %s\n", path, line, msg);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", msg, NULL);
	exit(EXIT_FAILURE);
}

void InfoFrom(const char *path, int line, const char *fmt, ...) {
	char    msg[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg), fmt, args);
	va_end(args);

	fprintf(stderr, "[INFO] %s:%i: %s\n", path, line, msg);
}
