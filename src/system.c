#include "system.h"

#include <SDL2/SDL.h>

#define Separator() fprintf(stderr, "========================================\n");

void SetupSystem(int winScale) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		Fatal("Failed to initialize SDL2: %s", SDL_GetError());

	SetupVideo(winScale);
	SetupAudio();
	Separator();
}

void CleanupSystem(void) {
	Separator();
	CleanupAudio();
	CleanupVideo();

	SDL_Quit();
}
