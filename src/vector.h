#ifndef VECTOR_H 
#define VECOTR_H 

typedef struct vector_t 
{ 
    void** elem; 
    int index; 
    int size; 
} vector; 

void  vector_init(vector* v, int size);
void  vector_free(vector* v);

void* vector_at(vector* v, int i);
int   vector_empty(vector* v);

void  vector_push_back(vector* v, void* new_elem);
void* vector_pop_back(vector* v);
int   vector_size(vector* v);

int  vector_full(vector* v);
void vector_resize(vector* v, int size);

#endif  
