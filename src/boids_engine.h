#ifndef BOIDS_ENGINE_H
#define BOIDS_ENGINE_H

#include <math.h>
#include "vector.h"
#include "shared.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#define FISH_NUMBER 300

#define BOID_GROUP 4
#define BOID_WCET 250
#define BOID_PERIOD 60000

#define MOUSE_WCET 1000
#define MOUSE_PERIOD 30000

#define DRAW_WCET 1500
#define DRAW_PERIOD 30000

#define MIN_X  0
#define MIN_Y  0
#define MAX_X  SCREEN_WIDTH - PANEL_WIDTH
#define MAX_Y  SCREEN_HEIGHT

typedef struct boids boids;
typedef struct boid boid;

enum options 
{
   MOVE_WITH = 0,
   MOVE_TO,
   MOVE_AWAY,
   MAX_SPEED,
   /*MIN_DIST,*/
   SIGHT,
   N_BOIDS,
   SHARK_FEAR,
   ENABLE_SHARK,
   MAX_OPTIONS 
};

float sign( float num );

/*************/
/*   POINT   */
/*************/

typedef struct point
{
    float x, y;
} point;

point* point_add( point *p, const point p2 );
point* point_add_float( point *p, const float p2 );
point* point_sub( point *p, const point p2 );
point* point_mul( point *p, const point p2 );
point* point_mul_float( point *p, const float p2 );
point* point_div_float( point *p, const float p2 );
float point_abs( point p );
float point_distance( const point p1, const point p2);

/*************/
/*   BOID    */
/*************/

struct boid
{
    point pos;
    point old_pos;
    point mom; 
    
    point pos_temp; 
    point mom_temp;
    
    bool flip;
    
    SDL_Surface *img, *img_f;
    boids *pop;
    
    bool to_live;
    
    int active_readers;
    int active_writers;
    sem_t* mutex;
    sem_t* priv_read;
    sem_t* priv_write;
    int blocked_writers;
    int blocked_readers;
};

void boid_init( boid *ind, boids* pop);
void boid_request_read( boid *ind );
void boid_request_write( boid *ind );
void boid_release_read( boid *ind );
void boid_release_write( boid *ind );
void three_rules( boid *ind, boids *pop );
void flee_from_shark( boid *ind, boids *pop );
void border_constraints( boid* ind, int speed );
void boid_update( boid* ind );

/*************/
/*   BOIDS   */
/*************/

struct boids
{
    vector *vct;
    int size;
    
    boid shark;
    
    float c_moveWith;
    float c_moveTo;
    float c_moveAway;
    float max_speed;
    float min_distance;
    float fear_shark;
    float sight;
    
    float c_moveWith_val;
    float c_moveTo_val;
    float c_moveAway_val;
    float fear_shark_val;
    int enable_shark;
    
    bool pause;
    
    int active_readers;
    int active_writers;
    sem_t* mutex;
    sem_t* priv_read;
    sem_t* priv_write;
    int blocked_writers;
    int blocked_readers; 
};

void boids_init( boids *pop );
void boids_request_read( boids *pop );
bool boids_request_write( boids *pop, bool use_pause );
void boids_release_read( boids *pop );
void boids_release_write( boids *pop, bool use_pause );

void update_shark( boids *pop );
void boids_set_val( boids *bds, int key, float val );

#endif
