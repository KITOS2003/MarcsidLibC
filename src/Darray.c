#ifndef DARRAY_H
#define DARRAY_H

#include <stdint.h>
#include <stdlib.h>

// getters

void *_Darray_create(size_t, size_t, void *(*)(size_t),
                     void *(*)(void *, size_t), void (*)(void *));
#define Darray_create(type, capacity)                                          \
    _Darray_create(sizeof(type), capacity, malloc, realloc, free)
#define Darray_create_allocator(type, capacity, malloc, realloc, free)         \
    _Darray_create(sizeof(type), capacity, malloc, realloc, free)

void Darray_destroy(void *data);

size_t Darray_length(void *data);
size_t Darray_get_capacity(void *data);
size_t Darray_get_reserve(void *data);

void _Darray_reserve(void **, size_t);
#define Darray_reserve(data_p, nrs) _Darray_reserve((void **)data_p, nrs)

void _Darray_push(void **, void *);
#define Darray_push(data_p, element)                                           \
    {                                                                          \
        __auto_type Darray_push_temp_var = element;                            \
        _Darray_push((void **)(data_p), &Darray_push_temp_var);                \
    }

void _Darray_pop(void **, void *);
#define Darray_pop(data_p, out) _Darray_pop((void **)data_p, out);

void _Darray_push_multiple(void **, void *, size_t);
#define Darray_push_multiple(data_p, arr, n)                                   \
    _Darray_push_multiple((void **)data_p, arr, n);

void _Darray_pop_multiple(void **, void *, size_t);
#define Darray_pop_multiple(data_p, out, n)                                    \
    _Darray_pop_multiple((void **)data_p, out, n)

void _Darray_push_middle(void **, size_t, void *);
#define Darray_push_middle(data_p, index, element)                             \
    {                                                                          \
        __auto_type Darray_push_temp_var = element;                            \
        _Darray_push_middle((void **)data_p, index, &Darray_push_temp_var);    \
    }

void _Darray_pop_middle(void **, size_t, void *);
#define Darray_pop_middle(data_p, index, out)                                  \
    _Darray_pop_middle((void **)data_p, index, out)

void _Darray_push_middle_multiple(void **, size_t, void *, size_t);
#define Darray_push_middle_multiple(data_p, index, arr, n)                     \
    _Darray_push_middle_multiple((void **)data_p, index, arr, n)

void _Darray_pop_middle_multiple(void **, size_t, void *, size_t);
#define Darray_pop_middle_multiple(data_p, index, out, n)                      \
    _Darray_push_middle_multiple((void **)data_p, index, arr, n)

void _Darray_push_beg(void **, void *);
#define Darray_push_beg(data_p, element)                                       \
    _Darray_push_beg((void **)data_p, element)

void _Darray_pop_beg(void **, void *);
#define Darray_pop_beg(data_p, out) _Darray_pop_beg((void **)data_p, out)

void _Darray_push_beg_multiple(void **, void *, size_t);
#define Darray_push_beg_multiple(data_p, array, n)                             \
    _Darray_push_beg_multiple((void **)data_p, array, n)

void _Darray_pop_beg_multiple(void **, void *, size_t);
#define Darray_pop_beg_multiple(data_p, out, n)                                \
    _Darray_pop_beg_multiple((void **)data_p, out, n)

void _Darray_merge(void **, void *);
#define Darray_merge(data_p1, data2) _Darray_merge((void **)data_p1, data2)

void _Darray_merge_middle(void **, void *, size_t);
#define Darray_merge_middle(data_p1, data2, index)                             \
    _Darray_merge_middle((void **)data_p1, data2, index)

void _Darray_merge_beg(void **, void *, size_t);
#define Darray_merge_beg(data_p1, data2, index)                                \
    _Darray_merge_beg((void **)data_p1, data2, index)

void *Darray_split(void *data, size_t start_index, size_t end_index);

void Darray_print(void *data, const char *const format,
                  void (*print_func)(void *));

#endif

#ifdef DARRAY_INCLUDE_IMPLEMENTATION

#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef void *(*malloc_t)(size_t);
typedef void *(*realloc_t)(void *, size_t);
typedef void (*free_t)(void *);

typedef struct Darray
{
    size_t n_elements;
    size_t element_size;
    size_t capacity;
    size_t reserve_space;

    malloc_t allocator;
    realloc_t reallocator;
    free_t liberator;
} Darray;

#define GET_SELF(data) (Darray *)(data - sizeof(Darray))
#define GET_DATA(self) (void *)(self) + sizeof(Darray)

#define DARRAY_INTERNAL static inline

void *_Darray_create(size_t element_size, size_t initial_capacity,
                     malloc_t allocator, realloc_t reallocator,
                     free_t liberator)
{
    Darray *self = malloc(element_size * initial_capacity + sizeof(Darray));
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

/* * * *  RESIZING  * * * */
/* * * * (Internal) * * * */

DARRAY_INTERNAL void Darray_check_full_and_resize(Darray **self_p,
                                                  void **data_p, size_t offset)
{
    Darray *const self = *self_p;
    if (self->n_elements + offset >= self->capacity)
    {
        for (size_t i = 0; self->n_elements + offset >= self->capacity; i++)
        {
            self->capacity *= 2;
        }
        (*self_p) = self->reallocator(
            self, self->capacity * self->element_size + sizeof(Darray));
        (*data_p) = GET_DATA(self);
    }
}

DARRAY_INTERNAL void
Darray_check_underused_and_resize(Darray **self_p, void **data_p, size_t offset)
{
    Darray *const self = *self_p;
    if (self->n_elements - offset >= self->capacity / 4 &&
        self->reserve_space >= self->capacity / 2)
    {
        (*self_p) = self->reallocator(
            self, self->capacity * self->element_size + sizeof(Darray));
        (*data_p) = GET_DATA(self);
    }
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

size_t Darray_get_reserve(void *data)
{
    Darray *self = GET_SELF(data);
    return self->reserve_space;
}

void _Darray_reserve(void **data_p, size_t new_reserve_space)
{
    Darray *self = GET_SELF(*data_p);
    if (new_reserve_space > self->capacity)
    {
        self->capacity = new_reserve_space;
        self = self->reallocator(self, self->capacity * self->element_size +
                                           sizeof(Darray));
        (*data_p) = GET_DATA(self);
    }
    self->reserve_space = new_reserve_space;
}

/* * * * ELEMENT MANIPULATION * * * */

void _Darray_push(void **data_p, void *element)
{
    Darray *self = GET_SELF(*data_p);
    Darray_check_full_and_resize(&self, data_p, 1);
    memcpy((*data_p) + (self->n_elements * self->element_size), element,
           self->element_size);
    self->n_elements += 1;
}

void _Darray_pop(void **data_p, void *out)
{
    Darray *self = GET_SELF(*data_p);
#ifdef DARRAY_DEBUG
    if (self->n_elements == 0)
    {
        printf("Darray: pop function called upon empty array.\n");
        return;
    }
#endif
    if (out != NULL)
    {
        memcpy(out, (*data_p) + (self->n_elements - 1) * self->element_size,
               self->element_size);
    }
    Darray_check_underused_and_resize(&self, data_p, 1);
    self->n_elements -= 1;
}

void _Darray_push_multiple(void **data_p, void *array, size_t n)
{
    Darray *self = GET_SELF(*data_p);
    Darray_check_full_and_resize(&self, data_p, n);
    memcpy((*data_p) + (self->n_elements * self->element_size), array,
           n * self->element_size);
    self->n_elements += n;
}

void _Darray_pop_multiple(void **data_p, void *out, size_t n)
{
    Darray *self = GET_SELF(*data_p);
#ifdef DARRAY_DEBUG
    if (self->n_elements < n)
    {
        printf("Darray: refused to carry on with call to Darray_pop_multiple, "
               "number "
               "of elements to pop is greater than number of elements in the "
               "array\n");
        return;
    }
#endif
    if (out != NULL)
    {
        memcpy(out, (*data_p) + (self->n_elements - n) * self->element_size,
               n * self->element_size);
    }
    Darray_check_underused_and_resize(&self, data_p, n);
    self->n_elements -= n;
}

void _Darray_push_middle(void **data_p, size_t index, void *element)
{
    Darray *self = GET_SELF(*data_p);
#ifdef DARRAY_DEBUG
    if (index > self->n_elements - 1)
    {
        printf("Darray: refused to carry on with call to _Darray_push_middle, "
               "index overflows array length\n");
        return;
    }
#endif
    Darray_check_full_and_resize(&self, data_p, 1);
    void *middle_p = (*data_p) + index * self->element_size;
    memmove(middle_p + self->element_size, middle_p,
            (self->n_elements - index) * self->element_size);
    memcpy(middle_p, element, self->element_size);
    self->n_elements += 1;
}

void _Darray_pop_middle(void **data_p, size_t index, void *out)
{
    Darray *self = GET_SELF(*data_p);
#ifdef DARRAY_DEBUG
    if (index > self->n_elements - 1)
    {
        printf("Darray: refused to carry on with call to _Darray_pop_middle, "
               "index "
               "overflows array length\n");
        return;
    }
#endif
    void *middle_p = (*data_p) + index * self->element_size;
    memmove(middle_p, middle_p + self->element_size,
            (self->n_elements - index) * self->element_size);
    Darray_check_underused_and_resize(&self, data_p, 1);
    self->n_elements -= 1;
}

void _Darray_push_middle_multiple(void **data_p, size_t index, void *arr,
                                  size_t n)
{
    Darray *self = GET_SELF(*data_p);
#ifdef DARRAY_DEBUG
    if (index > self->n_elements - 1)
    {
        printf("Darray: refused to carry on with call to "
               "_Darray_push_middle_multiple, index overflows array length\n");
        return;
    }
#endif
    Darray_check_full_and_resize(&self, data_p, n);
    void *middle_p = (*data_p) + index * self->element_size;
    memmove(middle_p + n * self->element_size, middle_p,
            (self->n_elements - index) * self->element_size);
    memcpy(middle_p, arr, n * self->element_size);
    self->n_elements += n;
}

void _Darray_pop_middle_multiple(void **data_p, size_t index, void *out,
                                 size_t n)
{
    Darray *self = GET_SELF(*data_p);
#ifdef DARRAY_DEBUG
    if (index > self->n_elements - 1)
    {
        printf("Darray: refused to carry on with call to "
               "_Darray_pop_middle_multiple, index overflows array length\n");
        return;
    }
    if (n > self->n_elements)
    {
    }
#endif
    void *middle_p = (*data_p) + index * self->element_size;
    memmove(middle_p, middle_p + n * self->element_size,
            (self->n_elements - index) * self->element_size);
    Darray_check_underused_and_resize(&self, data_p, n);
    self->n_elements -= n;
}

void _Darray_push_beg(void **data_p, void *element)
{
    Darray *self = GET_SELF(*data_p);
    Darray_check_full_and_resize(&self, data_p, 1);
    memmove(*data_p + self->element_size, *data_p,
            self->n_elements * self->element_size);
    memcpy(*data_p, element, self->element_size);
    self->n_elements += 1;
}

void _Darray_pop_beg(void **data_p, void *out)
{
    Darray *self = GET_SELF(*data_p);
    if (out != NULL)
    {
        memcpy(out, *data_p, self->element_size);
    }
#ifdef DARRAY_DEBUG
    if (self->n_elements == 0)
    {
    }
#endif
    memmove(*data_p, *data_p + self->element_size,
            (self->n_elements - 1) * self->element_size);
    Darray_check_underused_and_resize(&self, data_p, 1);
    self->n_elements -= 1;
}

void _Darray_push_beg_multiple(void **data_p, void *array, size_t n)
{
    Darray *self = GET_SELF(*data_p);
    Darray_check_full_and_resize(&self, data_p, n);
    memmove(*data_p + n * self->element_size, *data_p,
            self->n_elements * self->element_size);
    memcpy(*data_p, array, n * self->element_size);
    self->n_elements += n;
}

void _Darray_pop_beg_multiple(void **data_p, void *out, size_t n)
{
    Darray *self = GET_SELF(*data_p);
    if (out != NULL)
    {
        memcpy(out, *data_p, n * self->n_elements);
    }
#ifdef DARRAY_DEBUG
#endif
    memmove(*data_p, *data_p + n * self->element_size,
            (self->n_elements - n) * self->n_elements);
    Darray_check_underused_and_resize(&self, data_p, n);
    self->n_elements -= n;
}

void _Darray_merge(void **dest, void *src)
{
    size_t len = Darray_length(src);
    _Darray_push_multiple(dest, src, len);
}

void _Darray_merge_middle(void **dest, void *src, size_t index)
{
    size_t len = Darray_length(src);
    _Darray_push_middle_multiple(dest, index, src, len);
}

void _Darray_merge_beg(void **dest, void *src, size_t index)
{
    size_t len = Darray_length(src);
    _Darray_push_beg_multiple(dest, src, len);
}

void *Darray_split(void *data, size_t start_index, size_t end_index)
{
    Darray *self = GET_SELF(data);
    size_t new_array_size = end_index - start_index;
    void *result =
        _Darray_create(self->element_size, new_array_size, self->allocator,
                       self->reallocator, self->liberator);
    _Darray_push_multiple(&result, data + start_index * self->element_size,
                          new_array_size);
    return result;
}

/* * * * CONVENIENCE * * * */

void Darray_print(void *data, const char *const format,
                  void (*print_func)(void *value_p))
{
    Darray *self = GET_SELF(data);
    printf("[ ");
    for (size_t i = 0; i < self->n_elements; i++)
    {
        if (print_func == NULL)
        {
            switch (self->element_size)
            {
            case 1:
                printf(format, *(uint8_t *)(data + (i * self->element_size)));
                break;
            case 2:
                printf(format, *(uint16_t *)(data + (i * self->element_size)));
                break;
            case 4:
                printf(format, *(uint32_t *)(data + (i * self->element_size)));
                break;
            case 8:
                printf(format, *(uint64_t *)(data + (i * self->element_size)));
                break;
            case 16:
                printf(format,
                       *(long double *)(data + (i * self->element_size)));
                break;
            default:
                printf("Impossible to print array, not a standard data type\n");
                return;
            }
        }
        else
        {
            print_func(data + (i * self->element_size));
        }
        printf(", ");
    }
    printf("]\n");
}

#endif // #ifdef DARRAY_INCLUDE_IMPLEMENTATION
