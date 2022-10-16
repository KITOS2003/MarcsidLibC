#ifndef PANIM_DARRAY_H
#define PANIM_DARRAY_H

#include<stdint.h>
#include<stdlib.h>

// getters
size_t Darray_length( void *array );
size_t Darray_capacity( void *array );

// creation and destruction
void *_Darray_create
    ( 
        size_t capacity, 
        size_t element_size,
        
        void* (*allocator)( size_t ),
        void* (*reallocator)( void*, size_t ),
        void  (*liberator)( void* )
    );
void Darray_destroy( void *array );

// resizing
void Darray_reserve( void **array, size_t new_size );
void Darray_resize( void **array, size_t new_size );

// elemennt manipulation
void *_Darray_push( void *array, void *element );
void _Darray_pop( void *array, void *var );
void Darray_merge( void *array1, void *array2 );

// wrapper macros
#define Darray_create( capacity, type ) _Darray_create( capacity, sizeof(type), malloc, realloc, free )
#define Darray_create_allocator(capacity,element_size,malloc,realloc,free) _Darray_create(capacity,size,malloc,realloc,free)
#define Darray_push( array, element ) { __auto_type darray_push_temp_var = element; array = _Darray_push( array, &darray_push_temp_var ); }
#define Darray_pop( array, var ) _Darray_pop( array, &var )

#endif
