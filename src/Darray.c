#include<stddef.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

typedef void* (*malloc_t)(size_t);
typedef void* (*realloc_t)(void*, size_t);
typedef void  (*free_t)(void*);

typedef struct Darray
{
    size_t n_elements;
    size_t element_size;
    size_t capacity;
    size_t reserve_space;

    malloc_t allocator;
    realloc_t reallocator;
    free_t liberator;
}
Darray;

#define GET_SELF(data) (Darray*)(data - sizeof(Darray))
#define GET_DATA(self) (void*)(self) + sizeof(Darray)

void *_Darray_create
    (
        size_t element_size, 
        size_t initial_capacity, 
        malloc_t allocator, 
        realloc_t reallocator, 
        free_t liberator 
    )
{
    Darray *self = malloc( element_size * initial_capacity + sizeof(Darray) );
    self->n_elements = 0;
    self->element_size = element_size;
    self->capacity = initial_capacity;
    self->reserve_space = initial_capacity;
    self->allocator = allocator;
    self->reallocator = reallocator;
    self->liberator = liberator;

    return GET_DATA(self);
}

void Darray_destroy(void *data)
{
    Darray *self = GET_SELF(data);
    self->liberator(self);
}

/* * * * GETTERS * * * */

size_t Darray_length(void *data)
{
    Darray *self = GET_SELF(data);
    return self->n_elements;
}

size_t Darray_get_capacity(void *data)
{
    Darray *self = GET_SELF(data);
    return self->capacity;
}

size_t Darray_get_reserve(void* data)
{
    Darray *self = GET_SELF(data);
    return self->reserve_space;
}

/* * * * RESIZING * * * */

static inline void Darray_check_full_and_resize(Darray **self_p, void **data_p, size_t offset)
{
    Darray * const self = *self_p;
    if( self->n_elements + offset >= self->capacity )
    {
        for(size_t i = 0 ;self->n_elements+offset>=self->capacity; i++)
        {
            self->capacity *= 2;
        }
        (*self_p) = self->reallocator(self, self->capacity*self->element_size + sizeof(Darray) );
        (*data_p) = GET_DATA(self);
    }
}

static inline void Darray_check_underused_and_resize(Darray **self_p, void **data_p, size_t offset)
{
    Darray * const self = *self_p;
    if( self->n_elements - offset >= self->capacity/4 && self->reserve_space >= self->capacity/2 )
    {
        (*self_p) = self->reallocator(self, self->capacity*self->element_size + sizeof(Darray) );
        (*data_p) = GET_DATA(self);
    }
}

void Darray_reserve(void **data_p, size_t new_reserve_space)
{
    Darray *self = GET_SELF(*data_p);
    if( new_reserve_space > self->capacity )               
    {
        self->capacity = new_reserve_space;
        self = self->reallocator(self, self->capacity*self->element_size + sizeof(Darray) );
        (*data_p) = GET_DATA(self);
    }
    self->reserve_space = new_reserve_space;
}

/* * * * ELEMENT MANIPULATION * * * */

void _Darray_push(void **data_p, void *element)
{
    Darray *self = GET_SELF(*data_p);
    Darray_check_full_and_resize( &self, data_p, 1 );
    memcpy((*data_p)+(self->n_elements*self->element_size), element, self->element_size);
    self->n_elements += 1;
}

void _Darray_pop(void **data_p)
{
    Darray *self = GET_SELF(*data_p);
    Darray_check_underused_and_resize( &self, data_p, 1 );
    self->n_elements -= 1;
}

void Darray_print( void *data, const char * const format )
{
    Darray *self = GET_SELF(data);
    printf("[ ");
    for(size_t i = 0; i < self->n_elements; i++)
    {
        printf( format, data+(i*self->element_size) );
        printf(", ");
    }
    printf("]\n");
}
