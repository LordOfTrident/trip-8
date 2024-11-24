#include "system_video.h"

#include <SDL2/SDL.h>

#define TITLE      "TRIP-8"
#define VIDEO_SIZE (VIDEO_WIDTH * VIDEO_HEIGHT)

/* Green (https://lospec.com/palette-list/knockia3310)
#define WHITE 0x72a488ff
#define BLACK 0x212c28ff
#define BACK  0x121c19ff
*/

/* Sand-ish (https://lospec.com/palette-list/ibm-51)
#define BLACK 0x323c39ff
#define WHITE 0xd3c9a1ff
#define BACK  0x1a2220ff
*/

/* Blue (https://lospec.com/palette-list/blue-monochrome-lcd) */
#define WHITE 0xafa7d4ff
#define BLACK 0x302a52ff
#define BACK  0x15102eff

static SDL_Renderer *ren;
static SDL_Window   *win;
static SDL_Texture  *scr;
static uint32_t     *buf;

static void SetupWindow(int scale) {
	Assert(scale > 0, "Scale must be greater than 0, got %i", scale);

	win = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                       VIDEO_WIDTH * scale, VIDEO_HEIGHT * scale, SDL_WINDOW_RESIZABLE);
	if (win == NULL)
		Fatal("Failed to create a window: %s", SDL_GetError());
}

static void SetupRenderer(void) {
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (ren == NULL)
		Fatal("Failed to create a renderer: %s", SDL_GetError());

	if (SDL_RenderSetLogicalSize(ren, VIDEO_WIDTH, VIDEO_HEIGHT) < 0)
		Fatal("Failed to set logical render size: %s", SDL_GetError());

	if (SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND) < 0)
		Fatal("Failed to set blend mode: %s", SDL_GetError());

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
}

static void SetupScreen(void) {
	scr = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
	                        SDL_TEXTUREACCESS_TARGET, VIDEO_WIDTH, VIDEO_HEIGHT);
	if (scr == NULL)
		Fatal("Failed to create a screen texture: %s", SDL_GetError());

	if ((buf = malloc(VIDEO_SIZE * sizeof(*buf))) == NULL)
		Fatal("Failed to allocate memory for screen buffer");

	ClearVideo();
}

static const char *RendererName(void) {
	SDL_RendererInfo info;
	SDL_GetRendererInfo(ren, &info);
	return info.name;
}

void SetupVideo(int scale) {
	SetupWindow(scale);
	Info("Created window");

	SetupRenderer();
	Info("Created renderer");
	Info("Using renderer \"%s\"", RendererName());

	SetupScreen();
	Info("Initialized video");
}

void CleanupVideo(void) {
	free(buf);

	SDL_DestroyTexture(scr);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);

	Info("Deinitialized video");
}

void DisplayVideo(void) {
	SDL_SetRenderDrawColor(ren, (BACK & 0xFF000000) >> 24,
	                            (BACK & 0x00FF0000) >> 16,
	                            (BACK & 0x0000FF00) >> 8, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(ren);
	SDL_UpdateTexture(scr, NULL, buf, VIDEO_WIDTH * sizeof(*buf));
	SDL_RenderCopy(ren, scr, NULL, NULL);
	SDL_RenderPresent(ren);
}

double VideoFrameTime(void) {
	static uint64_t last, now = 0;
	if (now == 0)
		now = SDL_GetPerformanceCounter();

	last = now;
	now  = SDL_GetPerformanceCounter();
	return (double)(now - last) * 1000 / (double)SDL_GetPerformanceFrequency();
}

void ClearVideo(void) {
	for (int i = 0; i < VIDEO_SIZE; ++ i)
		buf[i] = BLACK;
}

static bool FlipPixel(uint8_t x, uint8_t y) {
	if (x >= VIDEO_WIDTH || y >= VIDEO_HEIGHT)
		return false;

	/*
	Assert(x < VIDEO_WIDTH,  "Attempt to render beyond screen width (x = %i)",  x);
	Assert(y < VIDEO_HEIGHT, "Attempt to render beyond screen height (y = %i)", y);
	*/

	int idx  = (int)y * VIDEO_WIDTH + x;
	buf[idx] = buf[idx] == WHITE? BLACK : WHITE;
	return buf[idx] == BLACK;
}

bool DrawSprite(uint8_t x, uint8_t y, uint8_t *rows, uint8_t h) {
	x %= VIDEO_WIDTH;
	y %= VIDEO_HEIGHT;

	bool pixelWasUnset = false;
	for (int yo = 0; yo < h; ++ yo) {
		for (int xo = 0; xo < 8; ++ xo) {
			if ((rows[yo] & 1 << xo) == 0)
				continue;

			bool flippedToUnset = FlipPixel(x + (7 - xo), y + yo);
			if (!pixelWasUnset && flippedToUnset)
				pixelWasUnset = true;
		}
	}
	return pixelWasUnset;
}
