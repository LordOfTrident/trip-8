#include "system_audio.h"

#include <SDL2/SDL.h>

#define AMPLITUDE 28000
#define FREQUENCY 44100

#ifndef M_PI
#	define M_PI 3.1415926535
#endif

#define BEEP_HZ 440

static bool          beeping;
static SDL_AudioSpec spec;
static double        v;

static void GenerateSamples(uint16_t *stream, int reqLen) {
	for (int i = 0; i < reqLen; ++ i) {
		stream[i] = AMPLITUDE * sin(v * 2 * M_PI / FREQUENCY);
		v += BEEP_HZ;
	}
}

static void Callback(void *_, uint8_t *stream, int reqLen) {
	(void)_;
	if (beeping)
		GenerateSamples((uint16_t*)stream, reqLen / 2);
}

void SetupAudio(void) {
	spec.freq     = FREQUENCY;
	spec.format   = AUDIO_S16SYS;
	spec.channels = 1;
	spec.samples  = 2048;
	spec.callback = Callback;
	spec.userdata = NULL;

	if (SDL_OpenAudio(&spec, NULL) < 0)
		Fatal("Failed to open audio: %s", SDL_GetError());

	Info("Initialized audio");
}

void CleanupAudio(void) {
	SDL_CloseAudio();

	Info("Deinitialized audio");
}

void Beep(bool enable) {
	if (beeping == enable)
		return;

	beeping = enable;
	SDL_PauseAudio(!beeping);
}
