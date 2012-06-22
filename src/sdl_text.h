#ifndef SDL_TEXT
#define SDL_TEXT

#include <SDL_ttf.h>

#define FONT_NAME "fonts/roboto.ttf"

void init_font();
void close_font();

void print_text(SDL_Surface *surface, SDL_Rect *dest,
                const char *msg, int size, SDL_Color color,
                int x, int y);

#endif

