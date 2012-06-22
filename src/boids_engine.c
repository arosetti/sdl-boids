#include "boids_engine.h"

float sign( float num ) 
{
    return num / abs( num ); /* (num >= 0)?1:-1; */
}

/************************************************/

point* point_add( point *p, const point p2 )
{
    assert(p);
    p->x += p2.x;
    p->y += p2.y;
    return p;
}

point* point_add_float( point *p, const float p2 )
{
    assert(p);
    p->x += p2;
    p->y += p2;
    return p;
}

point* point_sub( point *p, const point p2 )
{
    assert(p);
    p->x -= p2.x;
    p->y -= p2.y;
    return p;
}

point* point_mul( point *p, const point p2 )
{
    assert(p);
    p->x *= p2.x;
    p->y *= p2.y;
    return p;
}

point* point_mul_float( point *p, const float p2 )
{
    assert(p);
    p->x *= p2;
    p->y *= p2;
    return p;
}

point* point_div_float( point *p, const float p2 )
{
    assert(p);
    p->x /= p2;
    p->y /= p2;
    return p;
}

float point_abs( point p )
{
    return sqrt(p.x*p.x + p.y*p.y);
}

float point_distance( const point p1, const point p2 )
{
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return sqrt(dx*dx + dy*dy);
}

/************************************************/

void boid_init( boid *ind, boids* pop)
{
    ind->mutex = malloc(sizeof(sem_t));
    ind->priv_read = malloc(sizeof(sem_t));
    ind->priv_write = malloc(sizeof(sem_t));
    sem_init(ind->mutex,0,1);
    sem_init(ind->priv_read,0,0);
    sem_init(ind->priv_write,0,0);
    ind->active_readers = 0;
    ind->active_writers = 0;    
    ind->blocked_writers = 0;
    ind->blocked_readers = 0;

    ind->old_pos.x = ind->pos.x = ind->pos_temp.x = (float)(rand() % MAX_X);
    ind->old_pos.y = ind->pos.y = ind->pos_temp.y = 0; //rand() % MAX_Y; 
    ind->mom.x = ind->mom_temp.x = rand() / (float)RAND_MAX * 10.0f - 5.0f;
    ind->mom.y = ind->mom_temp.y = rand() / (float)RAND_MAX * 10.0f - 5.0f;
    
    ind->to_live = true;
    
    ind->pop = pop;
    
    ind->flip = false;   
}

void boid_request_read( boid *ind )
{
    sem_wait(ind->mutex);
    if ((ind->blocked_writers > 0) || (ind->active_writers))
    {
        (ind->blocked_readers)++;
        sem_post(ind->mutex);
        sem_wait(ind->priv_read);
        (ind->blocked_readers)--;
    }
    (ind->active_readers)++;   

    if (ind->blocked_readers > 0)    
        sem_post(ind->priv_read);
    else    
        sem_post(ind->mutex);
}

void boid_request_write( boid *ind )
{
    sem_wait(ind->mutex);
    if ((ind->active_readers > 0) || (ind->active_writers == 1))
    {
        (ind->blocked_writers)++;
        sem_post(ind->mutex);
        sem_wait(ind->priv_write);
        (ind->blocked_writers)--;
    }
    ind->active_writers = 1;
    sem_post(ind->mutex);
}

void boid_release_read( boid *ind )
{
    sem_wait(ind->mutex);
    (ind->active_readers)--;
    if ((ind->active_readers == 0) && (ind->blocked_writers > 0))
        sem_post(ind->priv_write);
    else
        sem_post(ind->mutex);
}

void boid_release_write( boid *ind )
{
    sem_wait(ind->mutex);
    ind->active_writers = 0;
    if (ind->blocked_readers > 0)
        sem_post(ind->priv_read);
    else if (ind->blocked_writers > 0)
        sem_post(ind->priv_write); 
    else
        sem_post(ind->mutex);
}

void three_rules( boid *ind, boids *pop )
{   
    point dpos = {0,0}, MA_DPOS = {0,0}, MT_DPOS = {0,0}, MW_MOM = {0,0}, point_temp = {0,0};
    boid* ind_2 = NULL;
    point temp_mom_2;
    float d = 0;
    int i;
    
    assert(pop);
    assert(ind);
    
    for (i = 0 ; i < vector_size(pop->vct) ; i++)
    {
        ind_2 = vector_at(pop->vct, i);       
               
        dpos = ind->pos;
        
        boid_request_read(ind_2); 
        point_sub(&dpos, ind_2->pos);
        temp_mom_2 = ind_2->mom;
        boid_release_read(ind_2);
        
        d = point_abs(dpos);
        if ( d < pop->sight )
        {
            // separation: steer to avoid crowding local flockmates
            point_temp = dpos;
            point_mul_float(&point_temp, ( -(pop->max_speed) / pop->sight * d + pop->max_speed ));
            point_sub(&MA_DPOS, point_temp);
            // cohesion: steer to move toward the average position of local flockmates
            point_add(&MT_DPOS, dpos);
            // alignment: steer towards the average heading of local flockmates
            point_add(&MW_MOM, temp_mom_2);
        }       
    }
    
    point_temp = MA_DPOS;
    point_div_float(&point_temp, pop->c_moveAway * i);
    point_sub(&(ind->mom_temp), point_temp);
    
    point_temp = MA_DPOS;
    point_div_float(&point_temp, pop->c_moveTo * i);
    point_sub(&(ind->mom_temp), point_temp);
    
    point_temp = MW_MOM;
    point_div_float(&point_temp, pop->c_moveWith);
    point_add(&(ind->mom_temp), point_temp);
}

// flee from the shark
void flee_from_shark( boid* ind, boids *pop ) 
{
    point dpos = ind->pos;
    
    //boid_request_read(&(pop->shark));
    point_sub(&dpos, pop->shark.pos);
    //boid_release_read(&(pop->shark));
    
    float d = point_abs(dpos);
    if ( d < pop->sight )
    {
        point_mul_float(&dpos, ( - (pop->max_speed) / pop->sight * d + pop->max_speed ) * pop->fear_shark);
        point_add(&(ind->mom_temp), dpos);
    }
}

void border_constraints( boid* ind, int speed ) 
{ 
    if ( ind->pos_temp.x >= MAX_X )
        ind->pos_temp.x = 1;

    if ( ind->pos_temp.y >= MAX_Y )
        ind->pos_temp.y = 1;

    if ( ind->pos_temp.x <= 0 )
        ind->pos_temp.x = MAX_X-1;

    if ( ind->pos_temp.y <= 0 )
        ind->pos_temp.y = MAX_Y-1;

    if ( abs( ind->mom_temp.x ) > speed )
        ind->mom_temp.x = sign( ind->mom_temp.x ) * speed;
    if ( abs( ind->mom_temp.y ) > speed )
        ind->mom_temp.y = sign( ind->mom_temp.y ) * speed;
}

void border_constraints_2( boid* ind, int speed ) 
{ 
    if ( ind->pos.x >= MAX_X )
        ind->pos.x = 1;

    if ( ind->pos.y >= MAX_Y )
        ind->pos.y = 1;

    if ( ind->pos.x <= 0 )
        ind->pos.x = MAX_X-1;

    if ( ind->pos.y <= 0 )
        ind->pos.y = MAX_Y-1;

    if ( abs( ind->mom.x ) > speed )
        ind->mom.x = sign( ind->mom.x ) * speed;
    if ( abs( ind->mom.y ) > speed )
        ind->mom.y = sign( ind->mom.y ) * speed;
}

void follow_single( boid* ind, boid* victim ) 
{
    double c = 0.008;
    point dpos = victim->pos;
    point_sub(&dpos, ind->pos);
    double d = point_abs(dpos);

    if ( d < 2*ind->pop->sight ) 
    {
        point_mul_float(&dpos, c);
        point_add(&(ind->mom), dpos);
    }
}

void boid_update( boid* ind )
{
    // follow the three boid rules
    three_rules(ind, ind->pop);
    if (ind->pop->enable_shark)
        flee_from_shark(ind, ind->pop);
    border_constraints(ind, ind->pop->max_speed);

    // some randomness
    ind->mom_temp.x += 1.0 * ( rand() / (float)RAND_MAX - 0.5f );
    ind->mom_temp.y += 1.0 * ( rand() / (float)RAND_MAX - 0.5f );
    
    boid_request_write(ind);      
    // set new place
    point_add(&(ind->pos_temp),ind->mom_temp);    
    ind->mom = ind->mom_temp;    
    ind->pos = ind->pos_temp;
    boid_release_write(ind);   
}

/************************************************/

void boids_init( boids *pop )
{
    int i = 0;
    boid *ind = NULL;
    
    pop->mutex = malloc(sizeof(sem_t));
    pop->priv_read = malloc(sizeof(sem_t));
    pop->priv_write = malloc(sizeof(sem_t));
    sem_init(pop->mutex,0,1);
    sem_init(pop->priv_read,0,0);
    sem_init(pop->priv_write,0,0);
    pop->active_readers = 0;
    pop->active_writers = 0;    
    pop->blocked_writers = 0;
    pop->blocked_readers = 0;
    
    pop->pause = false;    
    
    pop->size = FISH_NUMBER;

    pop->vct = calloc(1, sizeof(vector));
    vector_init(pop->vct, pop->size);

    for (i = 0; i < pop->size; i++)
    {
        ind = malloc(sizeof(boid));

        boid_init(ind, pop);
        vector_push_back(pop->vct, ind);
    }
    
    pop->shark.old_pos.x = pop->shark.pos.x = MAX_X/2;
    pop->shark.old_pos.y = pop->shark.pos.y = MAX_Y/2;
    pop->shark.mom.x = 0;
    pop->shark.mom.y = 0;
    pop->shark.pop = pop;
    pop->enable_shark = 1;

    pop->c_moveWith_val = 2,5;
    pop->c_moveTo_val = 1,5;
    pop->c_moveAway_val = 10;
    pop->fear_shark_val = 12;
   
    pop->c_moveWith = 1 / exp( pop->c_moveWith_val - 9 );
    pop->c_moveTo = 1 / exp( pop->c_moveTo_val - 7 );
    pop->c_moveAway = 1 / exp( pop->c_moveAway_val - 20 );
    pop->max_speed = 3;
    pop->min_distance = 20;
    pop->fear_shark = exp(( pop->fear_shark_val - 30.0 ) / 4 );
    pop->sight = 250;
    pop->enable_shark = 1;
}

void boids_request_read( boids *pop )
{
    sem_wait(pop->mutex);
    if ((pop->blocked_writers > 0) || (pop->active_writers == 1))
    {
        (pop->blocked_readers)++;
        sem_post(pop->mutex);
        sem_wait(pop->priv_read);
        (pop->blocked_readers)--;
    }
    (pop->active_readers)++;   

    if (pop->blocked_readers > 0)    
        sem_post(pop->priv_read);
    else    
        sem_post(pop->mutex);
}

bool boids_request_write( boids *pop, bool use_pause )
{
    sem_wait(pop->mutex);
    if (use_pause && pop->pause)
    {
        sem_post(pop->mutex);
        return false;
    }
    if ((pop->active_readers > 0) || (pop->active_writers == 1))
    {
        (pop->blocked_writers)++;
        sem_post(pop->mutex);
        sem_wait(pop->priv_write);
        (pop->blocked_writers)--;
    }
    pop->active_writers = 1;
    sem_post(pop->mutex);
    return true;
}

void boids_release_read( boids *pop )
{
    sem_wait(pop->mutex);
    (pop->active_readers)--;
    if ((pop->active_readers == 0) && (pop->blocked_writers > 0))
        sem_post(pop->priv_write);
    else
        sem_post(pop->mutex);
}

void boids_release_write( boids *pop, bool use_pause )
{
    sem_wait(pop->mutex);
    if (use_pause && pop->pause)
    {
        sem_post(pop->mutex);
        return;
    }
    pop->active_writers = 0;
    if (pop->blocked_readers > 0)
        sem_post(pop->priv_read);
    else if (pop->blocked_writers > 0)
        sem_post(pop->priv_write); 
    else
        sem_post(pop->mutex);
}

void update_shark( boids *pop )
{
    boid* ind;
    
    boids_request_read(pop);
    double rnd = rand()/RAND_MAX;
    if ( rnd < 0.05 ) 
    {
        int choice = (int)(20 * rnd * pop->size);
        ind = vector_at(pop->vct, choice);
        if (ind)
        {                
            boid_request_read(ind);
            follow_single( &(pop->shark), ind );
            boid_release_read(ind);
        }
    }                       
    boids_release_read(pop);
    border_constraints_2(&(pop->shark), pop->max_speed * 2.0f);
    //boid_request_write(&(pop->shark));
    pop->shark.pos.x += pop->shark.mom.x;
    pop->shark.pos.y += pop->shark.mom.y;      
    //boid_release_write(&(pop->shark));
}

void boids_set_val( boids *bds, int key, float val ) 
{
    switch ( key ) 
    {
        case MOVE_WITH:
            if (val > 0 && bds->c_moveWith_val < 10.0)
            {
                boids_request_write(bds, true);
                bds->c_moveWith_val += 0.2;
                if (bds->c_moveWith_val > 10.0)
                    bds->c_moveWith_val = 10.0;
                bds->c_moveWith = 1 / exp( bds->c_moveWith_val - 9 );
                boids_release_write(bds, true);
            }
            else if (val < 0 && bds->c_moveWith_val > -20.0)
            {
                boids_request_write(bds, true);
                bds->c_moveWith_val -= 0.2;
                if (bds->c_moveWith_val < -20.0)
                    bds->c_moveWith_val = -20.0;
                bds->c_moveWith = 1 / exp( bds->c_moveWith_val - 9 );
                boids_release_write(bds, true);
            }        
            break;
        case MOVE_TO:
            if (val > 0 && bds->c_moveTo_val < 4.0)
            {
                boids_request_write(bds, true);
                bds->c_moveTo_val += 0.2;
                if (bds->c_moveTo_val > 4.0)
                    bds->c_moveTo_val = 4.0;
                bds->c_moveTo = 1 / exp( bds->c_moveTo_val - 7 );
                boids_release_write(bds, true);
            }
            else if (val < 0 && bds->c_moveTo_val > 0.0)
            {
                boids_request_write(bds, true);
                bds->c_moveTo_val -= 0.2;
                if (bds->c_moveTo_val < 0.0)
                    bds->c_moveTo_val = 0.0;
                bds->c_moveTo = 1 / exp( bds->c_moveTo_val - 7 );
                boids_release_write(bds, true);
            }        
            break;
        case MOVE_AWAY:
            if (val > 0 && bds->c_moveAway_val < 20.0)
            {
                boids_request_write(bds, true);
                bds->c_moveAway_val += 0.2;     
                if (bds->c_moveAway_val > 20.0)
                    bds->c_moveAway_val = 20.0;
                bds->c_moveAway = 1 / exp( bds->c_moveAway_val - 20 );
                boids_release_write(bds, true);
            }
            else if (val < 0 && bds->c_moveAway_val > 0.0)
            {
                boids_request_write(bds, true);
                bds->c_moveAway_val -= 0.2;
                if (bds->c_moveAway_val < 0.0)
                    bds->c_moveAway_val = 0.0;
                bds->c_moveAway = 1 / exp( bds->c_moveAway_val - 20 );
                boids_release_write(bds, true);
            }        
            break;
        case MAX_SPEED:
            if (val > 0 && bds->max_speed < 20.0)
            {
                boids_request_write(bds, true);            
                bds->max_speed += 0.2;
                if (bds->max_speed > 20.0)
                    bds->max_speed = 20.0;
                boids_release_write(bds, true);
            }
            else if (val < 0 && bds->max_speed > 0.0)
            {
                boids_request_write(bds, true);
                bds->max_speed -= 0.2;
                if (bds->max_speed < 0.0)
                    bds->max_speed = 0.0;
                boids_release_write(bds, true);
            }
            break;
        /*
        case MIN_DIST:
            bds->min_distance += val;
            break;
        */
        case SIGHT:
            if (val > 0 && bds->sight < MAX_X)
            {
                boids_request_write(bds, true);
                bds->sight += 1;
                boids_release_write(bds, true);
            }
            else if (val < 0 && bds->sight > 1)
            {
                boids_request_write(bds, true);
                bds->sight -= 1;
                boids_release_write(bds, true);
            }
            break;
        case N_BOIDS:
            if (val < 0 && bds->size > 0)
            {
                boids_request_write(bds, true);
                (bds->size)--;
                boid* ind = vector_at(bds->vct, bds->size);
                if (ind)
                {
                    ind->to_live = false;
                    vector_pop_back(bds->vct);
                }
                boids_release_write(bds, true);
            }
            else if (val > 0 && bds->size < 125)
            {
                /*boids_request_write(bds, true);
                boid* ind = malloc(sizeof(boid));
                if (!ind)
                {
                    return;
                }
                boid_init(ind, bds);
                vector_push_back(bds->vct, ind);
                boid_create(ind, true);
                (bds->size)++;
                boids_release_write(bds, true);*/
            }
            break;
        case SHARK_FEAR:
            if (val > 0 && bds->fear_shark_val < 15.0f)
            {
                boids_request_write(bds, true);
                bds->fear_shark_val += 0.2;
                if (bds->fear_shark_val > 15.0f)
                    bds->fear_shark_val = 15.0f;
                bds->fear_shark = exp(( bds->fear_shark_val - 30.0 ) / 4 );
                boids_release_write(bds, true);
            }
            else if (val < 0 && bds->fear_shark_val > 0.0f)
            {
                boids_request_write(bds, true);
                bds->fear_shark_val -= 0.2;
                if (bds->fear_shark_val < 0.0)
                    bds->fear_shark_val = 0.0;
                bds->fear_shark = exp(( bds->fear_shark_val - 30.0 ) / 4 );
                boids_release_write(bds, true);
            }
            break;
        case ENABLE_SHARK:
            boids_request_write(bds, true);                        
            if (val == -1)
                bds->enable_shark = (bds->enable_shark == 0)?2:(bds->enable_shark - 1);
            else
                bds->enable_shark = (bds->enable_shark + 1) % 3;
            boids_release_write(bds, true);
            break;
        default:
            break;
    }
}

