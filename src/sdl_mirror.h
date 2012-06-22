#ifndef SDL_MIRROR_H
#define SDL_MIRROR_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdint.h>

void     setpixel(SDL_Surface *, int, int, uint32_t);
uint32_t getpixel(SDL_Surface *, int, int);

SDL_Surface *mirror_surface_x(SDL_Surface *);
SDL_Surface *mirror_surface_y(SDL_Surface *);



#endif
