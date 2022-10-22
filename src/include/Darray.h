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
#define Darray_create(type, capacity) _Darray_create(sizeof(type), capacity, malloc, realloc, free)
#define Darray_create_allocator(type,capacity,malloc,realloc,free) _Darray_create(sizeof(type), capacity, malloc, realloc, free)

void Darray_destroy(void *);

size_t Darray_length(void*);
size_t Darray_get_capacity(void*);
size_t Darray_get_reserve(void*);

void _Darray_reserve(void**, size_t);
#define Darray_reserve(data_p, nrs) _Darray_reserve((void**)data_p, nrs)

void _Darray_push(void**,void*);
#define Darray_push(data_p, element) {__auto_type Darray_push_temp_var = element; _Darray_push((void**)(data_p), &Darray_push_temp_var);}

void _Darray_pop(void**, void*);
#define Darray_pop(data_p, out) _Darray_pop((void**)data_p, out);

void _Darray_push_multiple(void**, void*, size_t);
#define Darray_push_multiple(data_p, arr, n) _Darray_push_multiple((void**)data_p, arr, n);

void _Darray_pop_multiple(void**, void*, size_t);
#define Darray_pop_multiple(data_p, out, n) _Darray_pop_multiple((void**)data_p, out, n)

void Darray_print(void *, const char * const, void (*)(void*) );

#endif
