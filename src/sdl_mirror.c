#include "sdl_mirror.h"

uint8_t get_alpha(SDL_Surface* surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    uint8_t* p = (uint8_t*)surface->pixels + y * surface->pitch + x * bpp;
    uint32_t pixelColor;
     
    switch(bpp)
    {
        case(1):
            pixelColor = *p;
            break;
        case(2):
            pixelColor = *(uint16_t*)p;
            break;
        case(3):
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixelColor = p[0] << 16 | p[1] << 8 | p[2];
            else
                pixelColor = p[0] | p[1] << 8 | p[2] << 16;
            break;
        case(4):
            pixelColor = *(uint32_t*)p;
            break;
    }
     
    uint8_t red, green, blue, alpha;
    SDL_GetRGBA(pixelColor, surface->format, &red, &green, &blue, &alpha);
 
    return alpha;
}

void setpixel(SDL_Surface *surface, int x, int y, uint32_t newcolor )
{
    uint8_t *ubuff8;
    uint16_t*ubuff16;
    uint32_t *ubuff32;
    uint32_t color = newcolor;
    char c1, c2, c3;

    if(SDL_MUSTLOCK(surface)) 
        if(SDL_LockSurface(surface) < 0) 
            return;

    switch(surface->format->BytesPerPixel) 
    {
        case 1: 
            ubuff8 = (uint8_t*) surface->pixels;
            ubuff8 += (y * surface->pitch) + x; 
            *ubuff8 = (uint8_t) color;
        break;

        case 2:
            ubuff8 = (uint8_t*) surface->pixels;
            ubuff8 += (y * surface->pitch) + (x*2);
            ubuff16 = (uint16_t*) ubuff8;
            *ubuff16 = (uint16_t) color;
            
        break;  

        case 3:
            ubuff8 = (uint8_t*) surface->pixels;
            ubuff8 += (y * surface->pitch) + (x*3);

            if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
            {
                c1 = (color & 0xFF0000) >> 16;
                c2 = (color & 0x00FF00) >> 8;
                c3 = (color & 0x0000FF);
            }
            else
            {
                c3 = (color & 0xFF0000) >> 16;
                c2 = (color & 0x00FF00) >> 8;
                c1 = (color & 0x0000FF);
            }

            ubuff8[0] = c3;
            ubuff8[1] = c2;
            ubuff8[2] = c1;
        break;

        case 4:
            ubuff8 = (uint8_t*) surface->pixels;
            ubuff8 += (y*surface->pitch) + (x*4);
            ubuff32 = (uint32_t*)ubuff8;
            *ubuff32 = color;
        break;

        default:
            fprintf(stderr, "error: Unknown bitdepth!\n");
    }

    if(SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

}

uint32_t getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp)
    {
        case 1:
            return *p;
        case 2:
            return *(uint16_t*)p;
        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
        case 4:
            return *(uint32_t *)p;
        default:
            return 0;
    }
}

SDL_Surface *mirror_surface_x( SDL_Surface *surface )
{
	SDL_Surface* mirror = SDL_CreateRGBSurface( SDL_HWSURFACE, surface->w, surface->h,
	                                            surface->format->BitsPerPixel,
	                                            surface->format->Rmask,
	                                            surface->format->Gmask,
	                                            surface->format->Bmask,
	                                            surface->format->Amask);

	for( int y = 0; y < surface->h; y++ )
		for( int x = 0; x < surface->w; x++ )
			setpixel( mirror, x, y, getpixel( surface, x, surface->h - y - 1 ) );

    SDL_SetColorKey( mirror, SDL_SRCCOLORKEY, surface->format->colorkey); 

	return mirror;
}

SDL_Surface *mirror_surface_y( SDL_Surface *surface )
{
	SDL_Surface* mirror = SDL_CreateRGBSurface( SDL_HWSURFACE, surface->w, surface->h,
	                                            surface->format->BitsPerPixel,
	                                            surface->format->Rmask,
	                                            surface->format->Gmask,
	                                            surface->format->Bmask,
	                                            surface->format->Amask);

	for( int y = 0; y < surface->h; y++ )
		for( int x = 0; x < surface->w; x++ )
			setpixel( mirror, x, y, getpixel( surface, surface->w - x - 1, y ) );

    SDL_SetColorKey( mirror, SDL_SRCCOLORKEY, surface->format->colorkey); 

	return mirror;
}
