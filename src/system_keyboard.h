#ifndef SYSTEM_KEYBOARD_H_HEADER_GUARD
#define SYSTEM_KEYBOARD_H_HEADER_GUARD

#include <stdbool.h> /* bool, true, false */
#include <string.h>  /* memset */
#include <stdint.h>  /* uint8_t */

#include "common.h"

#define KEYS_COUNT 16

/* Emulator control keys */
enum {
	CKEY_HALT = 0,
	CKEY_RESET,

	CKEYS_COUNT,
};

#define CkeyIsPressed(CKEY) KeyIsPressed(KEYS_COUNT + (CKEY))

void    UpdateKeyboard(void);
bool    KeyIsPressed(uint8_t key);
uint8_t GetPressedKey(void);

#endif
