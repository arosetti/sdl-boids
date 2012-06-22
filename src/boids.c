#include "boids.h"

boids  *bds   = NULL;
bool   quit   = false;
int    option = 0;

void draw_panel(SDL_Surface *surface, SDL_Rect *dest)
{
    char buffer[32];
    int i = 0;

    struct label
    {
        char *name;
        int  type;
        void *var;
    };

    struct label lbl[] = 
    {
        {"Move With",   0, &bds->c_moveWith_val},
        {"Move To",     0, &bds->c_moveTo_val},
        {"Move Away",   0, &bds->c_moveAway_val},
        {"Max Speed",   0, &bds->max_speed},
        {"Sight",       0, &bds->sight},
        {"Fish Number", 1, &bds->size},
        {"Shark Fear",  0, &bds->fear_shark_val},
        {"Enable Shark",2, &bds->enable_shark},
        {NULL,          0, NULL}
    };
    
    while (lbl[i].name)
    {
        sprintf(buffer, lbl[i].name);
        if (option == i)
            print_text(surface, dest, buffer, 11, (SDL_Color){220,0,0,70}, 10, 40 + i * 30);
        else
            print_text(surface, dest, buffer, 11, (SDL_Color){220,220,220,0}, 10, 40 + i * 30);

        switch (lbl[i].type)
        {
            case 0:
                sprintf(buffer, "> %f          ", *((float*)lbl[i].var));
                break;
            case 1:
                sprintf(buffer, "> %d          ", *((int*)lbl[i].var));
                break;
            case 2:
                if (*((int*)lbl[i].var) == 1)
                    sprintf(buffer, "> true         ");
                else if (*((int*)lbl[i].var) == 0)
                    sprintf(buffer, "> false        ");
                else if (*((int*)lbl[i].var) == 2)
                    sprintf(buffer, "> AUTO        ");
                else
                    sprintf(buffer, "> NOT VALID   ");
                break;
            default:
                break;
        }
           
        buffer[10] = '\0';
        if (option == i)
            print_text(surface, dest, buffer, 11, (SDL_Color){220,0,0,0}, 10, 52 + i * 30);
        else
            print_text(surface, dest, buffer, 11, (SDL_Color){220,220,220,70}, 10, 52 + i * 30);
        i++;
    }
}

/******************************** THREADS *************************************/

void *event_thread(void *arg)
{
    SDL_Event *event = (SDL_Event*) arg;
    
    while (1)
    {
        SDL_WaitEvent(event); /* SDL_PollEvent(event); */
        
        switch (event->type)
        {
            case SDL_KEYDOWN:
                switch (event->key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_LEFT:
                         boids_set_val(bds, option, -1);
                        break;
                    case SDLK_RIGHT:
                        boids_set_val(bds, option, 1);
                        break;
                    case SDLK_SPACE:
                        if (!bds->pause)
                        {        
                            boids_request_write(bds, false);        
                            bds->pause = true;
                        }
                        else
                        {
                            boids_release_write(bds, false);
                            bds->pause = false;
                        }
                        break;
                    default:
                        break;

                }
                break;
            case SDL_KEYUP:
                switch (event->key.keysym.sym)
                {
                    case SDLK_UP:
                        option = !option?(MAX_OPTIONS - 1):(option - 1);
                        break;
                    case SDLK_DOWN:
                        option = (option + 1) % MAX_OPTIONS;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                break;
            case SDL_MOUSEBUTTONUP:
                break;
            case SDL_MOUSEMOTION:
                if (bds->enable_shark == 1)
                {
                    bds->shark.pos.x = event->motion.x;
                    bds->shark.pos.y = event->motion.y;
                }
                break;  
            case SDL_QUIT:
                quit = true;
                break;
            default:
                break;
        }
        SDL_Delay(1);
    }
    
    pthread_exit(NULL);
}

void *boids_thread(void *arg)
{
    boid *ind = NULL;
    int i;
    
    while (1)
    {
        for (i = 0; i < bds->size; i++)
        {
            ind = vector_at(bds->vct, i);
            if (ind)
                boid_update(ind);
        }
        SDL_Delay(1);
    }
}

/********************************* MAIN **************************************/

int main(int argc, char *argv[])
{
    SDL_Surface *screen;
    SDL_Surface *shark_img,     *shark_f_img, *background_img,
                *clownfish_img, *clownfish_f_img,
                *fish1_img,     *fish1_f_img,
                *fish2_img,     *fish2_f_img;
    SDL_Rect    dest, shark_rect, background_rect,
                clownfish_rect, fish1_rect, fish2_rect;
    SDL_Event event;
    
    int i;
    boid* ind = NULL;
    
    srand(time(NULL));
    screen = sdl_init();
    
    /* images */
    load_image("art/clownfish.png", &clownfish_img, &clownfish_rect);
    clownfish_f_img = mirror_surface_y(clownfish_img);

    load_image("art/fish1.png", &fish1_img, &fish1_rect);
    fish1_f_img = mirror_surface_y(fish1_img);

    load_image("art/fish2.png", &fish2_img, &fish2_rect);
    fish2_f_img = mirror_surface_y(fish2_img);
    
    load_image("art/shark.png", &shark_img, &shark_rect);
    shark_f_img = mirror_surface_y(shark_img);

    load_image("art/underwater.jpg", &background_img, &background_rect);

    if (!(bds = calloc(1, sizeof(boids)))); /* TODO check */
    boids_init(bds);

    for (i = 0; i < bds->size; i++)
    {
        ind = vector_at(bds->vct, i);
        if (ind)
            switch (rand()%3)
            {
                case 0:
                    ind->img = clownfish_img;
                    ind->img_f = clownfish_f_img;
                    break;
                case 1:
                    ind->img = fish1_img;
                    ind->img_f = fish1_f_img;
                    break;
                case 2:
                    ind->img = fish2_img;
                    ind->img_f = fish2_f_img;
                    break;
                default:
                    break;
            }
    }
    
    /* pthread */
    pthread_t      tid;
    pthread_attr_t tattr;
    int ret;

    ret = pthread_attr_init(&tattr);
    ret = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);

    if ((ret = pthread_create(&tid, &tattr, event_thread, &event)))
    {
        perror("pthread_create");
        exit(1);
    }

    if ((ret = pthread_create(&tid, &tattr, boids_thread, NULL)))
    {
        perror("pthread_create");
        exit(1);
    }

    while (!quit)
    {
        boids_request_read(bds);
        for (i = 0; i < bds->size; i++)
        {
            ind = vector_at(bds->vct, i);
            if (ind)
            {                
                boid_request_read(ind);
                if (ind->pos.x >= ind->old_pos.x)
                    ind->flip = true;
                else
                    ind->flip = false;
                ind->old_pos = ind->pos;
                
                dest.x = (int)ind->pos.x;
                dest.y = (int)ind->pos.y;
                
                if (ind->flip)
                    SDL_BlitSurface(ind->img_f, &clownfish_rect, screen, &dest);
                else
                    SDL_BlitSurface(ind->img, &clownfish_rect, screen, &dest);
                boid_release_read(ind);
            }
        }
        boids_release_read(bds);
        
        //boid_request_read(&(bds->shark));
        if (bds->enable_shark != 0)
        {
            if (bds->enable_shark == 2)
                update_shark(bds);
                
            dest.x = (int)bds->shark.pos.x;
            dest.y = (int)bds->shark.pos.y;
            
            if (bds->shark.pos.x != bds->shark.old_pos.x)
            {
                if (bds->shark.pos.x >= bds->shark.old_pos.x)
                    bds->shark.flip = true;
                else
                    bds->shark.flip = false;
                
                bds->shark.old_pos.x = bds->shark.pos.x;
                bds->shark.old_pos.y = bds->shark.pos.y;
            }
            if (bds->shark.flip)
                SDL_BlitSurface(shark_img, &shark_rect, screen, &dest);
            else
                SDL_BlitSurface(shark_f_img, &shark_rect, screen, &dest);
        }
        //boid_release_read(&(bds->shark));
        
        print_text(screen, &dest,  PACKAGE_STRING, 14, (SDL_Color){255,255,255,100}, 10, 10);
    
        draw_panel(screen, &dest);
    
        SDL_Flip(screen);
        
        check_fps();
        
        dest.x = 0;
        dest.y = 0; 
        SDL_BlitSurface(background_img, &background_rect, screen, &dest);
    }
    
    SDL_FreeSurface(clownfish_img);
    SDL_FreeSurface(clownfish_f_img);
    SDL_FreeSurface(fish1_img);
    SDL_FreeSurface(fish1_f_img);
    SDL_FreeSurface(fish2_img);
    SDL_FreeSurface(fish2_f_img);
    SDL_FreeSurface(shark_img);
    SDL_FreeSurface(shark_f_img);
    SDL_FreeSurface(background_img);
    //SDL_Quit();
    close_font();
    
    return 0;
}
