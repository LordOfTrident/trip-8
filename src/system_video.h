#ifndef SYSTEM_VIDEO_H_HEADER_GUARD
#define SYSTEM_VIDEO_H_HEADER_GUARD

#include <stdint.h>  /* uint8_t, uint32_t, uint64_t */
#include <stdbool.h> /* bool, true, false */

#include "common.h"

#define VIDEO_WIDTH  64
#define VIDEO_HEIGHT 32

void SetupVideo(int scale);
void CleanupVideo(void);

void   DisplayVideo(void);
double VideoFrameTime(void);

void ClearVideo(void);
bool DrawSprite(uint8_t x, uint8_t y, uint8_t *rows, uint8_t h);

#endif
