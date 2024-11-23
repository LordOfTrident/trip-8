#include "system_keyboard.h"

#include <SDL2/SDL.h>

static bool keyboard[KEYS_COUNT + CKEYS_COUNT];

#define Ckeyboard(CKEY) (keyboard[KEYS_COUNT + CKEY])

static void UpdateKeysPress(SDL_Event *evt, bool down) {
	switch (evt->key.keysym.sym) {
	case SDLK_ESCAPE: Ckeyboard(CKEY_HALT)  = down; break;
	case SDLK_RETURN: Ckeyboard(CKEY_RESET) = down; break;

	case SDLK_x: case SDLK_KP_0:                  keyboard[0x0] = down; break;
	case SDLK_1: case SDLK_KP_1:                  keyboard[0x1] = down; break;
	case SDLK_2: case SDLK_KP_2: case SDLK_UP:    keyboard[0x2] = down; break;
	case SDLK_3: case SDLK_KP_3:                  keyboard[0x3] = down; break;
	case SDLK_q: case SDLK_KP_4: case SDLK_LEFT:  keyboard[0x4] = down; break;
	case SDLK_w: case SDLK_KP_5:                  keyboard[0x5] = down; break;
	case SDLK_e: case SDLK_KP_6: case SDLK_RIGHT: keyboard[0x6] = down; break;
	case SDLK_a: case SDLK_KP_7:                  keyboard[0x7] = down; break;
	case SDLK_s: case SDLK_KP_8: case SDLK_DOWN:  keyboard[0x8] = down; break;
	case SDLK_d: case SDLK_KP_9:                  keyboard[0x9] = down; break;
	case SDLK_z:                                  keyboard[0xA] = down; break;
	case SDLK_c:                                  keyboard[0xB] = down; break;
	case SDLK_4:                                  keyboard[0xC] = down; break;
	case SDLK_r:                                  keyboard[0xD] = down; break;
	case SDLK_f:                                  keyboard[0xE] = down; break;
	case SDLK_v:                                  keyboard[0xF] = down; break;
	}
}

void UpdateKeyboard(void) {
	SDL_Event evt;
	while (SDL_PollEvent(&evt)) {
		switch (evt.type) {
		case SDL_QUIT:    Ckeyboard(CKEY_HALT) = true;  break;
		case SDL_KEYDOWN: UpdateKeysPress(&evt, true);  break;
		case SDL_KEYUP:   UpdateKeysPress(&evt, false); break;
		default: break;
		}
	}
}

bool KeyIsPressed(uint8_t key) {
	Assert(key < sizeof(keyboard), "Attempt to check for invalid key 0x%02X", key);

	return keyboard[key];
}

uint8_t GetPressedKey(void) {
	for (uint8_t key = 0; key < KEYS_COUNT; ++ key) {
		if (keyboard[key])
			return key;
	}
	return KEYS_COUNT;
}
