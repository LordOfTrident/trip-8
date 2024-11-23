#ifndef EMULATOR_H_HEADER_GUARD
#define EMULATOR_H_HEADER_GUARD

#include <stdlib.h> /* size_t, rand, srand */
#include <time.h>   /* time */
#include <stdio.h>  /* printf, fflush, stdout, fopen, fclose, fread, ftell, fseek, rewind, FILE */
#include <stdint.h> /* uint8_t, uint16_t */
#include <errno.h>  /* errno */
#include <string.h> /* strerror, memset, memcpy, strcmp */

#include "common.h"
#include "system.h"

void RunEmulator(const char *romPath);

#endif
