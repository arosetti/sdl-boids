#include "vector.h" 

#include <stdio.h> 
#include <stdlib.h> 
#include <assert.h> 

#define RESIZE  5

void vector_init(vector *v, int size) 
{
    v->index = 0; 
    v->size = size; 
    v->elem = (void**) calloc(v->size, sizeof(void*)); 
} 

int vector_empty(vector *v)
{
    return !v->index;
} 

void vector_free(vector *v)
{
    free(v->elem);
} 

int vector_full(vector *v)
{
    return v->index == v->size;
} 

void vector_resize(vector *v, int size) 
{ 
    v->size += size; 
    v->elem = (void**) realloc(v->elem, v->size * sizeof(void*)); 
} 

int vector_size(vector *v)
{
    return v->size;
} 

void* vector_at(vector *v, int i) 
{ 
    return v->elem[i]; 
} 

void vector_push_back(vector *v, void* new_elem) 
{ 
    if (vector_full(v))
        vector_resize(v, RESIZE); 
    v->elem[v->index++] = new_elem; 
} 

void* vector_pop_back(vector *v) 
{ 
    if ( vector_empty(v) )  
        return NULL;

    void* elem = vector_at(v, --(v->index)); 
     
    /*
    if ( v->size - v->index >= RESIZE )
        vector_resize(v, -RESIZE);
    */

    return elem; 
}
