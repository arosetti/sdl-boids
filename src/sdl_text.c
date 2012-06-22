#include "sdl_text.h"

void init_font()
{
    if (TTF_Init() == -1)
       printf("unable to initialize SDL_ttf: %s \n", TTF_GetError());
}

void close_font()
{
    TTF_Quit();
}

void print_text(SDL_Surface *surface, SDL_Rect *dest,
                const char *msg, int size, SDL_Color color,
                int x, int y)
{
    SDL_Surface *text;
    SDL_Rect     text_rect;
    TTF_Font    *font;
    
    font = TTF_OpenFont(FONT_NAME, size);
    if (font == NULL)
       printf("unable to load font: %s %s \n", FONT_NAME, TTF_GetError());
    
    text = TTF_RenderText_Blended(font, msg, color);
    text_rect.x = 0;
    text_rect.y = 0;
    text_rect.w = text->w;
    text_rect.h = text->h;
    dest->x = x;
    dest->y = y; 
    SDL_BlitSurface(text, &text_rect, surface, dest);
    SDL_FreeSurface(text);
    
    TTF_CloseFont(font);
}


