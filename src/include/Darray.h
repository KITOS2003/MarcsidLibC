#ifndef DARRAY_H
#define DARRAY_H

#include<stdint.h>
#include<stdlib.h>

// getters

void *_Darray_create
    (
        size_t, 
        size_t, 
        void* (*)(size_t), 
        void* (*)(void*,size_t), 
        void  (*)(void*) 
    );
void Darray_destroy(void *);

size_t Darray_length(void*);
size_t Darray_get_capacity(void*);
size_t Darray_get_reserve(void*);

void Darray_reserve(void**, size_t);
void _Darray_push(void**,void*);
void _Darray_pop(void**);

void Darray_print(void *, const char * const);

#define Darray_create(type, capacity) _Darray_create(sizeof(type), capacity, malloc, realloc, free)
#define Darray_create_allocator(type,capacity,malloc,realloc,free) _Darray_create(sizeof(type), capacity, malloc, realloc, free)

#define Darray_push(data, element) {__auto_type Darray_push_temp_var = element; _Darray_push((void**)(&data), &Darray_push_temp_var);}
#define Darray_pop(data) _Darray_pop(&data);

#endif
