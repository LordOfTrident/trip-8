#ifndef SYSTEM_AUDIO_H_HEADER_GUARD
#define SYSTEM_AUDIO_H_HEADER_GUARD

#include <stdbool.h> /* bool, true, false */
#include <stdint.h>  /* uint8_t, uint16_t, uint32_t */
#include <math.h>    /* sin */

#include "common.h"

void SetupAudio(void);
void CleanupAudio(void);

void Beep(bool enable);

#endif
