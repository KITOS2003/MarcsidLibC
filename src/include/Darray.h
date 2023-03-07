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
  {                                                                            \
    __auto_type Darray_push_temp_var = element;                                \
    _Darray_push((void **)(data_p), &Darray_push_temp_var);                    \
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
  {                                                                            \
    __auto_type Darray_push_temp_var = element;                                \
    _Darray_push_middle((void **)data_p, index, &Darray_push_temp_var);        \
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
