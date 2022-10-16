#include<stddef.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

typedef struct Darray
{
    size_t n_elements;
    size_t element_size;
    size_t capacity;
    size_t reserve_space;

    void* (*allocator)( size_t );
    void* (*reallocator)( void*, size_t );
    void  (*liberator)( void* );
}
Darray;

#define GET_SELF(array) (Darray*)((char*)(array) - sizeof(Darray))

/**********************************/ 
/************GETTERS***************/
/**********************************/ 

size_t Darray_length( void *array )
{
    Darray *self = GET_SELF(array);
    return self->n_elements;
}

size_t Darray_capacity( void *array )
{
    Darray *self = GET_SELF(array);
    return self->capacity;
}

size_t Darray_get_reserve( void *array )
{
    Darray *self = GET_SELF(array);
    return self->reserve_space;
}

/**********************************/ 
/*****CREATION AND DESTRUCTION*****/
/**********************************/ 

void *_Darray_create
    ( 
        size_t capacity, 
        size_t element_size,
        
        void* (*allocator)( size_t ),
        void* (*reallocator)( void*, size_t ),
        void  (*liberator)( void* )
    )
{
    void *memory = allocator( capacity * element_size + sizeof(Darray) );

    Darray *array = memory;

    array->n_elements = 0;
    array->element_size = element_size;
    array->capacity = capacity;
    array->reserve_space = capacity;
    array->allocator = allocator;
    array->reallocator = reallocator;
    array->liberator = liberator;

    return ((char*)(memory)+sizeof(Darray));
}

void Darray_destroy( void *array )
{
    Darray *self = GET_SELF(array);
    self->liberator(array);
}

/**********************************/ 
/************RESIZING**************/
/**********************************/ 

void Darray_reserve( void **array, size_t new_size )
{
    Darray *self = GET_SELF(*array);
    self->reserve_space = new_size;
    if( self->capacity >= new_size )
    {
        return;
    }
    *array = self->reallocator(array, new_size);
}

void Darray_resize( void **array, size_t new_size )
{
    Darray *self = GET_SELF(*array);
    if( self->reserve_space > new_size )
    {
        if( self->capacity > self->reserve_space )
        {
            self->capacity = self->reserve_space;
            *array = self->reallocator(array, self->capacity);
        }
        return;
    }
    self->capacity = new_size;
    *array = self->reallocator(array, new_size);
}

/**********************************/ 
/*******ELEMENT MANIPULATION*******/
/**********************************/ 

void *_Darray_push( void *array, void *element )
{
    Darray *self = GET_SELF(array);
    if( self->n_elements >= self->capacity )
    {
        self->capacity *= 2;
        array = self->reallocator( array, self->capacity );
        self = GET_SELF(array);
    }
    memcpy( ((char*)(array) + self->n_elements*self->element_size), element, self->element_size );
    self->n_elements += 1;
    return array;
}

void _Darray_push_middle( void *array, void *element )
{
    Darray *self = GET_SELF(array);

}

void _Darray_pop( void *array, void *var )
{
    Darray *self = GET_SELF(array);
    self->n_elements -= 1;
    memcpy( var, ((char*)(array) + self->n_elements*self->element_size), self->element_size );
    if( self->n_elements < self->capacity/4 && self->capacity/2 > self->reserve_space )
    {
        self->capacity /= 2;
        array = self->reallocator( array, self->capacity );
    }
}

void Darray_merge( void *array1, void *array2 )
{
    Darray *dest   = GET_SELF(array1);
    Darray *source = GET_SELF(array2);

    size_t occupied_space_dest   = dest->n_elements*dest->element_size;
    size_t occupied_space_source = source->n_elements*source->element_size;

    size_t initial_capacity_dest = dest->capacity;
    while( dest->capacity - occupied_space_dest < occupied_space_source )
    {
        dest->capacity *= 2;
    }

    if( dest->capacity != initial_capacity_dest )
    {
        array1 = dest->reallocator( array1, dest->capacity );
        dest = GET_SELF(array1);
    }
    memcpy( ((char*)(array1) + occupied_space_dest), array2, occupied_space_source );
    dest->n_elements += source->n_elements;
}


