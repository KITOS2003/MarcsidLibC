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

void _Darray_reserve(void **data_p, size_t new_reserve_space)
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

void _Darray_pop(void **data_p, void *out)
{
    Darray *self = GET_SELF(*data_p);
#ifdef DARRAY_DEBUG
    if(self->n_elements == 0)
    { 
        printf("Darray: pop function called upon empty array.\n");
        return; 
    }
#endif
    if( out != NULL )
    {
        memcpy(out, (*data_p)+(self->n_elements-1)*self->element_size, self->element_size);
    }
    Darray_check_underused_and_resize( &self, data_p, 1 );
    self->n_elements -= 1;
}

void _Darray_push_multiple(void **data_p, void *array, size_t n)
{
    Darray *self = GET_SELF(*data_p);
    Darray_check_full_and_resize(&self, data_p, n);
    memcpy((*data_p)+(self->n_elements*self->element_size), array, n*self->element_size);
    self->n_elements += n;
}

void _Darray_pop_multiple(void **data_p, void *out, size_t n)
{
    Darray *self = GET_SELF(*data_p);
#ifdef DARRAY_DEBUG
    if(self->n_elements < n)
    {
        printf("Darray: refused to carry on with call to Darray_pop_multiple, number of elements to pop is greater than number of elements in the array\n");
        return;
    }
#endif
    if( out != NULL )
    {
        memcpy(out, (*data_p)+(self->n_elements-n)*self->element_size, n*self->element_size);
    }
    Darray_check_underused_and_resize(&self, data_p, n);
    self->n_elements -= n;
}

void _Darray_push_middle(void **data_p, size_t index, void *element)
{
    Darray *self = GET_SELF(*data_p);
#ifdef DARRAY_DEBUG
    if(index > self->n_elements-1)
    {
        printf("Darray: refused to carry on with call to _Darray_push_middle, index overflows array length\n");
        return;                                                                                   Darray: pop function called upon empty array.\n
    }
#endif
    Darray_check_full_and_resize(&self, data_p, 1);
}

void _Darray_pop_middle(void **data_p, size_t index, void *out)
{

}

/* * * * CONVENIENCE * * * */

void Darray_print( void *data, const char * const format, void (*print_func)(void *value_p) )
{
    Darray *self = GET_SELF(data);
    printf("[ ");
    for(size_t i = 0; i < self->n_elements; i++)
    {
        if( print_func == NULL )
        {
            switch(self->element_size)
            {
                case 1:
                    printf( format, *(uint8_t*)(data+(i*self->element_size)) );
                    break;
                case 2:
                    printf( format, *(uint16_t*)(data+(i*self->element_size)) );
                    break;
                case 4:
                    printf( format, *(uint32_t*)(data+(i*self->element_size)) );
                    break;
                case 8:
                    printf( format, *(uint64_t*)(data+(i*self->element_size)) );
                    break;
                case 16:
                    printf( format, *(long double*)(data+(i*self->element_size)) );
                    break;
                default:
                    printf("Impossible to print array, not a standard data type\n");
                    return;
            }
        }
        else
        {
            print_func( data+(i*self->element_size) );
        }
        printf(", ");
    }
    printf("]\n");
}
