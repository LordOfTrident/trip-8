#include <stdio.h>  /* printf, fprintf, stderr */
#include <stdlib.h> /* EXIT_FAILURE */
#include <string.h> /* strcmp */

#include "emulator.h"

#define VERSION "1.0.0"

static const char *argv0;

static void Usage(void) {
	printf("  _____ ___ ___ ___     ___\n"
	       " |_   _| _ \\_ _| _ \\___( _ )\n"
	       "   | | |   /| ||  _/___/ _ \\\n"
	       "   |_| |_|_\\___|_|     \\___/\n"
	       "  Trident's CHIP-8 emulator\n"
	       "\n"
	       "Source code: https://github.com/LordOfTrident/trip-8\n"
	       "Usage: %s <ROM_PATH> | -h | -v\n"
	       "Options:\n"
	       "  -h    Print this message\n"
	       "  -v    Print version information\n", argv0);
}

static void Version(void) {
	printf("TRIP-8 version %s compiled on %s\n", VERSION, __DATE__);
}

static char *Shift(int *argc, char ***argv) {
	char *arg = **argv;
	-- *argc;
	++ *argv;
	return arg;
}

int main(int argc, char **argv) {
	argv0 = Shift(&argc, &argv);
	if (argc != 1) {
		Usage();
		return EXIT_FAILURE;
	}

	char *arg = Shift(&argc, &argv);
	if (strcmp(arg, "-h") == 0) {
		Usage();
		return 0;
	} else if (strcmp(arg, "-v") == 0) {
		Version();
		return 0;
	}

	RunEmulator(arg);
	return 0;
}
