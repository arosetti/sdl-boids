#include "sdl_utils.h"

static int tick = 0, interval = 1000 / FPS;

SDL_Surface *sdl_init()
{
    SDL_Surface *screen;
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    atexit(SDL_Quit);
    init_font();

    if (!(screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, SDL_DOUBLEBUF)))
    {
        printf("unable to set video mode: %s\n", SDL_GetError());
        exit(1);
    }
    
    SDL_WM_SetCaption(PACKAGE_STRING,  PACKAGE_STRING);
    SDL_ShowCursor(SDL_DISABLE);
    
    return screen;
}

void load_image(const char *filename, SDL_Surface **surface, SDL_Rect *rect)
{
    SDL_Surface *temp;
    
    temp = IMG_Load(filename);
    if (temp == 0)
    {
        printf("unable to load image: %s\n", SDL_GetError());
        return 1;
    }
    
    (*surface) = SDL_DisplayFormat(temp);
     
    rect->x = 0;
    rect->y = 0;
    rect->w = (*surface)->w;
    rect->h = (*surface)->h;
}

void check_fps()
{
    if (tick > SDL_GetTicks())
        SDL_Delay(tick - SDL_GetTicks());
    tick = SDL_GetTicks( ) + interval ;
}
