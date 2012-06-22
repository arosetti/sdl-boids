#ifndef SDL_UTILS
#define SDL_UTILS

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "sdl_mirror.h"
#include "sdl_text.h"
#include "shared.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define SCREEN_DEPTH   16
#define FPS            50

SDL_Surface  *sdl_init();
void         load_image(const char *filename, SDL_Surface **, SDL_Rect *);
void         check_fps();

#endif

