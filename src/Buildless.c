#ifndef BUILDLESS_H_
#define BUILDLESS_H_

#include <stddef.h>
#include <strings.h>
#define BUILDLESS_DEBUG

#ifdef BUILDLESS_DEBUG
#define BUILDLESS_INCLUDE_IMPLEMENTATION
#define DARRAY_DEBUG
#endif // #ifdef BUILDLESS_DEBUG

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __unix__
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define DIR_DIVISOR_CHAR '/'
#define DIR_DIVISOR_CHAR_STR "/"
#define MKDIR(dir) mkdir(dir, 0777)
#endif // #ifdef __unix__

#ifdef _WIN32
#include <Windows.h>
#define DIR_DIVISOR_CHAR '\\'
#define DIR_DIVISOR_CHAR_STR "\\"
#define MKDIR(dir) _mkdir(dir)
#endif // #ifdef _WIN32

// Compiler specific:

#if defined(__GNUC__)
#define COMPILER "gcc"
#elif defined(__clang__)
#define COMPILER "clang"
#endif // #if defined(__GNUC__)

// Dynamic arrays from https://github.com/kitos2003/marcsidlibc:

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

#ifndef BUILDLESS_NODEFINE_DARRAY

#define GET_SELF(data) (Darray *)(data - sizeof(Darray))
#define GET_DATA(self) (void *)(self) + sizeof(Darray)

void *_Darray_create(size_t element_size, size_t initial_capacity,
                     malloc_t allocator, realloc_t reallocator,
                     free_t liberator);
#define Darray_create(type, capacity)                                          \
    _Darray_create(sizeof(type), capacity, malloc, darray_realloc, free)

void Darray_destroy(void *data);

void _Darray_push(void **, void *);
#define Darray_push(data_p, element)                                           \
    {                                                                          \
        __auto_type Darray_push_temp_var = element;                            \
        _Darray_push((void **)(data_p), &Darray_push_temp_var);                \
    }

void _Darray_pop(void **, void *);
#define Darray_pop(data_p, out) _Darray_pop((void **)data_p, out)

void _Darray_pop_middle(void **, size_t, void *);
#define Darray_pop_middle(data_p, index, out)                                  \
    _Darray_pop_middle((void **)data_p, index, out)

void _Darray_pop_multiple(void **, void *, size_t);
#define Darray_pop_multiple(data_p, out, n)                                    \
    _Darray_pop_multiple((void **)data_p, out, n)

void _Darray_push_multiple(void **, void *, size_t);
#define Darray_push_multiple(data_p, arr, n)                                   \
    _Darray_push_multiple((void **)data_p, arr, n);

void _Darray_merge(void **, void *);
#define Darray_merge(data_p1, data2) _Darray_merge((void **)data_p1, data2)

size_t Darray_length(void *data);

#endif // #ifndef BUILDLESS_NODEFINE_DARRAY

#define STR_ARRAY(...) _Darray_from_strings(__VA_ARGS__, NULL)
char **_Darray_from_strings(const char *first, ...);

// "Garbage collection"

#define gc_strdup(str) gc_add(strdup(str));

static void **gc_registry_internal = NULL;
static size_t *gc_free_ind_internal = NULL;

static void **gc_registry_user = NULL;
static size_t *gc_free_ind_user = NULL;

static inline void gc_collect_all();
static inline void *darray_realloc(void *, size_t);

// string utils

char *str_join(char **str_arr);
bool str_begins_with(const char *string, const char *prefix);
bool str_ends_with(const char *string, const char *postfix);
size_t str_find(const char *string, const char *substring);
size_t *str_find_char(const char *string, char c);

#define str_concat(...) _str_concat_va(__VA_ARGS__, NULL)
const char *_str_concat_va(const char *first, ...);

// generics

static bool generic_cmp(const char *generic, const char *string,
                        char ***matches_out);
const char *generic_to_literal(const char *generic, char **matches);

// buildless core

typedef void (*callback_t)(char **, const char *, void *);

typedef struct
{
    callback_t callback;
    const char *target;
    char **dependencies;
} Rule;

void _buildless_init(const char **argv, const char *file);
#define buildless_init(argv) _buildless_init(argv, __FILE__)

bool buildless_make_rule(Rule *rule, Rule *all, size_t n_rules,
                         const char *target, char **matches);

void _buildless_go(const int argc, const char *argv[], Rule *all,
                   size_t n_rules, char *file);
#define buildless_go(argc, argv, all)                                          \
    _buildless_go(argc, argv, all, sizeof all / sizeof(Rule), __FILE__)

// platform:
int source_modified_after_target(const char *source_path,
                                 const char *target_path);
int is_dir(const char *path);
char **listdir(const char *dir_path);

// utils:

void mkdir_r(const char *dir);

#endif // #ifndef BUILDLESS_H_

/*
 *  IMPLEMENTATION:
 */

#ifdef BUILDLESS_INCLUDE_IMPLEMENTATION

// Darray implementation:

#ifndef BUILDLESS_NOINCLUDE_DARRAY_IMPLEMENTATION

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

size_t Darray_length(void *data)
{
    Darray *self = GET_SELF(data);
    return self->n_elements;
}

static inline void Darray_check_full_and_resize(Darray **self_p, void **data_p,
                                                size_t offset)
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

static inline void
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

void _Darray_push_multiple(void **data_p, void *array, size_t n)
{
    Darray *self = GET_SELF(*data_p);
    Darray_check_full_and_resize(&self, data_p, n);
    memcpy((*data_p) + (self->n_elements * self->element_size), array,
           n * self->element_size);
    self->n_elements += n;
}

void _Darray_merge(void **data_p1, void *data2)
{
    size_t len = Darray_length(data2);
    _Darray_push_multiple(data_p1, data2, len);
}

char **_Darray_from_strings(const char *first, ...)
{
    char **result = Darray_create(char *, 32);
    va_list args;
    va_start(args, first);
    for (const char *next = first; next != NULL;
         next = va_arg(args, const char *))
    {
        Darray_push(&result, next);
    }
    va_end(args);
    return result;
}

#endif // #ifndef BUILDLESS_NOINCLUDE_DARRAY_IMPLEMENTATION

// "garbage collection" implementation:
static inline void gc_collect_all()
{
    /* for (size_t i = 0; i < Darray_length(gc_registry_internal); i += 1) */
    /* { */
    /*     for (size_t j = 0; j < Darray_length(gc_free_ind_internal); j += 1)
     */
    /*     { */
    /*         size_t ind = gc_free_ind_internal[j]; */
    /*         if (i == ind) */
    /*         { */
    /*             goto continue_gc_loop; */
    /*         } */
    /*     } */
    /*     /1* free(gc_registry_internal[i]); *1/ */
    /* continue_gc_loop: */
    /*     continue; */
    /* } */
    /* Darray_destroy(gc_registry_internal); */
    /* gc_registry_internal = Darray_create(void *, 32); */
}

static inline void *gc_add(void *ptr)
{
    /* if (Darray_length(gc_free_ind_internal) == 0) */
    /* { */
    /*     Darray_push(&gc_registry_internal, ptr); */
    /*     return ptr; */
    /* } */
    /* size_t ind = 0; */
    /* Darray_pop(&gc_free_ind_internal, &ind); */
    /* gc_registry_internal[ind] = ptr; */
    return ptr;
}

static inline void gc_pop(void *ptr)
{
    /* for (size_t i = 0; i < Darray_length(gc_registry_internal); i += 1) */
    /* { */
    /*     if (gc_registry_internal[i] == ptr) */
    /*     { */
    /*         Darray_push(&gc_free_ind_internal, i); */
    /*         break; */
    /*     } */
    /* } */
}

static inline void *darray_realloc(void *ptr, size_t size)
{
    /* int32_t ind = -1; */
    /* for (size_t i = 0; i < Darray_length(gc_registry_internal); i += 1) */
    /* { */
    /*     if (ptr == gc_registry_internal[i]) */
    /*     { */
    /*         ind = i; */
    /*         break; */
    /*     } */
    /* } */
    ptr = realloc(ptr, size);
    /* if (ind != -1) */
    /* { */
    /*     gc_registry_internal[ind] = ptr; */
    /* } */
    return ptr;
}

// string utils

char *str_join(char **str_arr)
{
    size_t str_size = 0;
    for (size_t i = 0; i < Darray_length(str_arr); i += 1)
    {
        str_size += strlen(str_arr[i]) + 1;
    }
    char *result = malloc(str_size);
    *result = 0;
    strcat(result, str_arr[0]);
    for (size_t i = 1; i < Darray_length(str_arr); i += 1)
    {
        strcat(result, " ");
        strcat(result, str_arr[i]);
    }
    gc_add(result);
    return result;
}

size_t *str_find_char(const char *string, char c)
{
    size_t *result = Darray_create(size_t, 10);
    for (size_t i = 0; i < strlen(string); i += 1)
    {
        if (string[i] == c)
        {
            Darray_push(&result, i);
        }
    }
    if (Darray_length(result) == 0)
    {
        return NULL;
    }
    gc_add(GET_SELF(result));
    return result;
}

bool str_begins_with(const char *string, const char *prefix)
{
    for (size_t i = 0; i < strlen(string); i += 1)
    {
        if (prefix[i] == 0)
        {
            return true;
        }
        if (prefix[i] != string[i])
        {
            break;
        }
    }
    return false;
}

bool str_ends_with(const char *string, const char *postfix)
{
    size_t str_len = strlen(string);
    size_t postfix_len = strlen(postfix);
    for (size_t i = 0; i < str_len; i += 1)
    {
        if (i == postfix_len + 1)
        {
            return true;
        }
        if (postfix[postfix_len - i] != string[str_len - i])
        {
            break;
        }
    }
    return false;
}

// returns index of the last character of the first occurrence of substring in
// string, returns -1 if substring is not present in string
size_t str_find(const char *string, const char *substring)
{
    size_t susbstring_index = 0;
    for (size_t i = 0; i <= strlen(string); i += 1)
    {
        if (substring[susbstring_index] == 0)
        {
            return i;
        }
        if (string[i] != substring[susbstring_index])
        {
            susbstring_index = 0;
        }
        else
        {
            susbstring_index += 1;
        }
    }
    return -1;
}

char **str_split(const char *string, char c)
{
    char **result = Darray_create(char *, 10);
    char *str_copy = gc_strdup(string);
    size_t *indices = str_find_char(string, c);
    if (indices == NULL)
    {
        return NULL;
    }
    Darray_push(&indices, strlen(str_copy));
    size_t prev_ind = 0;
    for (size_t i = 0; i < Darray_length(indices); i += 1)
    {
        size_t ind = indices[i];
        str_copy[ind] = 0;
        Darray_push(&result, &(str_copy[prev_ind]));
        prev_ind = ind + 1;
    }
    gc_add(GET_SELF(result));
    return result;
}

const char *str_pop_path(const char *string, char **path_out)
{
    size_t *indices = str_find_char(string, DIR_DIVISOR_CHAR);
    if (indices == NULL)
    {
        if (path_out != NULL)
            *path_out = "";
        return string;
    }
    size_t ind = 0;
    Darray_pop(&indices, &ind);
    if (path_out != NULL)
    {
        char *buf = gc_strdup(string);
        buf[ind] = 0;
        *path_out = buf;
        return &buf[ind + 1];
    }
    char *result = malloc((strlen(string) - ind) * sizeof *result);
    strcpy(result, &string[ind]);
    gc_add(result);
    return result;
}

const char *str_pop_extension(const char *string, char **extension_out)
{
    size_t *indices_dot = str_find_char(string, '.');
    size_t *indices_div = str_find_char(string, DIR_DIVISOR_CHAR);
    if (indices_dot == NULL)
    {
        if (extension_out != NULL)
            *extension_out = "";
        return string;
    }
    size_t last_dot_ind = 0;
    if (indices_div != NULL)
    {
        last_dot_ind = indices_dot[Darray_length(indices_dot) - 1];
        if (last_dot_ind < indices_div[Darray_length(indices_div) - 1])
        {
            *extension_out = "";
            return string;
        }
    }
    if (extension_out != NULL)
    {
        char *buf = strdup(string);
        buf[last_dot_ind] = 0;
        *extension_out = &buf[last_dot_ind + 1];
        gc_add(buf);
        return buf;
    }
    char *result = malloc((strlen(string) - strlen(&string[last_dot_ind] + 1)) *
                          sizeof *result);
    memcpy(result, string, (last_dot_ind) * sizeof(char));
    result[last_dot_ind] = 0;
    gc_add(result);
    return result;
}

#define str_concat(...) _str_concat_va(__VA_ARGS__, NULL)
const char *_str_concat_va(const char *first, ...)
{
    size_t allocated_size = 512;
    char *result = malloc(512 * sizeof *result);
    *result = 0;
    va_list args;
    va_start(args, first);
    size_t total_string_size = 0;
    for (const char *s = first; s != NULL; s = va_arg(args, const char *))
    {
        total_string_size += strlen(s);
        while (total_string_size > allocated_size)
        {
            allocated_size *= 2;
        }
        result = realloc(result, allocated_size * sizeof *result);
        strcat(result, s);
    }
    va_end(args);
    gc_add(result);
    return (const char *)result;
}

// generics

bool generic_cmp(const char *generic, const char *string, char ***matches_out)
{
    char *string_copy = strdup(string);
    char **split = str_split(generic, '@');
    if (split == NULL)
    {
        if (strcmp(generic, string) == 0)
        {
            return true;
        }
        return false;
    }
    char **matches = Darray_create(char *, 16);
    if (!str_begins_with(string, split[0]) ||
        !str_ends_with(string, split[Darray_length(split) - 1]))
    {
        Darray_destroy(matches);
        if (matches_out != NULL)
            *matches_out = NULL;
        free(string_copy);
        return false;
    }
    char *s = string_copy;
    for (size_t i = 0; i < Darray_length(split); i += 1)
    {
        size_t ind = str_find(s, split[i]);
        if (ind == -1)
        {
            Darray_destroy(matches);
            if (matches_out != NULL)
                *matches_out = NULL;
            free(string_copy);
            return false;
        }
        s += ind;
        Darray_push(&matches, s);
        if (strlen(split[i]) != 0) // TODO is this ok??
            *(s - strlen(split[i])) = 0;
    }
    Darray_pop(&matches, NULL);
    gc_add(string_copy);
    if (matches_out != NULL)
        *matches_out = matches;
    return true;
}

const char *generic_to_literal(const char *generic, char **matches)
{
    if (matches == NULL)
    {
        return strdup(generic);
    }
    size_t *indices = str_find_char(generic, '@');
    if (Darray_length(indices) != Darray_length(matches))
    {
        fprintf(
            stderr,
            "[ERROR:] Cant convert generic %s to literal given matches %s\n",
            generic, str_join(matches));
    }
    size_t matches_total_size = 0;
    for (size_t i = 0; i < Darray_length(matches); i += 1)
    {
        matches_total_size += strlen(matches[i]);
    }
    char *result =
        malloc((strlen(generic) + matches_total_size) * sizeof *result);
    *result = 0;
    char **split = str_split(generic, '@');
    for (size_t i = 0; i < Darray_length(matches); i += 1)
    {
        strcat(result, split[i]);
        strcat(result, matches[i]);
    }
    strcat(result, split[Darray_length(split) - 1]);
    return result;
}

// buildless core

void command_va(char *format, ...)
{
    char *cmd_string = malloc(4000);
    va_list args;
    va_start(args, format);
    vsprintf(cmd_string, format, args);
    va_end(args);
    printf("[CMD:] %s\n", cmd_string);
    fflush(stdout);
    system(cmd_string);
    free(cmd_string);
}

void _buildless_init(const char **argv, const char *file)
{
    gc_registry_internal = Darray_create(void *, 128);
    gc_free_ind_internal = Darray_create(size_t, 32);

#ifndef BUILDLESS_DEBUG
    if (source_modified_after_target(file, argv[0] + 2) == 1)
    {
        command_va("gcc %s -o %s", file, argv[0] + 2);
        command_va("%s", argv[0]);
        exit(0);
    }
#endif // #ifndef BUILDLESS_DEBUG
}

void _buildless_go(const int argc, const char *argv[], Rule *all,
                   size_t n_rules, char *file)
{
    gc_registry_user = gc_registry_internal;
    gc_free_ind_user = gc_free_ind_internal;

    gc_registry_internal = Darray_create(void *, 128);
    gc_free_ind_internal = Darray_create(size_t, 32);

    if (argc == 1)
    {
        buildless_make_rule(all, all, n_rules, NULL, NULL);
    }
    if (argc > 1)
    {
        for (int i = 1; i < argc; i += 1)
        {
            for (size_t j = 0; j < n_rules; j += 1)
            {
                char **matches = NULL;
                if (generic_cmp(all[j].target, argv[i], &matches))
                {
                    buildless_make_rule(&all[j], all, n_rules, argv[i],
                                        matches);
                }
            }
        }
    }
}

bool buildless_make_rule(Rule *rule, Rule *all, size_t n_rules,
                         const char *target, char **matches)
{
    bool should_run = false;
    if (target == NULL)
    {
        target = rule->target;
    }
    {
        char *path = NULL;
        str_pop_path(target, &path);
        mkdir_r(path);
    }
    char **dependencies = Darray_create(char *, 32);
    for (size_t i = 0; i < Darray_length(rule->dependencies); i += 1)
    {
        const char *dependency =
            generic_to_literal(rule->dependencies[i], matches);
        int modified = source_modified_after_target(dependency, target);
        if (modified == 1)
        {
            should_run = true;
        }
        // TODO memory leak
        Darray_push(&dependencies, dependency);
        // Check if there is a rule to make dependency
        for (size_t j = 0; j < n_rules; j += 1)
        {
            const char *aux = all[j].target;
            if (generic_cmp(all[j].target, target, NULL))
            {
                continue;
            }
            char **new_matches = NULL;
            if (generic_cmp(all[j].target, dependency, &new_matches))
            {
                should_run |= buildless_make_rule(&all[j], all, n_rules,
                                                  dependency, new_matches);
                // TODO fix infinite recursion on circular dependency
                if (new_matches != NULL)
                    Darray_destroy(new_matches);
                goto continue_dep_loop;
            }
        }
        // Check if dependency is an existing file
        if (modified == -1)
        {
            fprintf(stderr,
                    "[ERROR:] no rule to make dependency %s needed to make "
                    "target %s\n",
                    dependency, target);
            exit(1);
        }
    continue_dep_loop:
        continue;
    }

    if (should_run)
    {
        if (rule->callback != NULL)
            rule->callback(dependencies, target, NULL); // TODO args
        gc_collect_all();
        return true;
    }
    gc_collect_all(); // TODO FIX GC!!!
    return false;
}

// utils:

void mkdir_r(const char *dir)
{
    if (dir == NULL)
    {
        return;
    }
    if (strcmp(dir, "") == 0)
    {
        return;
    }
    size_t *indices = str_find_char(dir, DIR_DIVISOR_CHAR);
    if (indices == NULL)
    {
        MKDIR(dir);
        return;
    }
    Darray_push(&indices, strlen(dir));
    char *dir_copy = strdup(dir);
    for (size_t i = 0; i < Darray_length(indices); i += 1)
    {
        size_t ind = indices[i];
        dir_copy[ind] = 0;
        MKDIR(dir_copy);
        dir_copy[ind] = DIR_DIVISOR_CHAR;
    }
    free(dir_copy);
}

// return NULL if dir does not exist
char **listdir_r(const char *dir_path)
{
    char **ls = listdir(dir_path);
    char **result = Darray_create(char *, 32);
    char *new_path = NULL;
    if (ls == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < Darray_length(ls); i += 1)
    {
        new_path =
            malloc((strlen(dir_path) + strlen(ls[i]) + 1) * sizeof *new_path);
        *new_path = 0;
        strcat(new_path, dir_path);
        strcat(new_path, DIR_DIVISOR_CHAR_STR);
        strcat(new_path, ls[i]);
        if (is_dir(new_path) == 1)
        {
            char **ls_r = listdir_r(new_path);
            Darray_merge(&result, ls_r);
            Darray_destroy(ls_r);
            free(new_path);
        }
        else
        {
            Darray_push(&result, new_path);
        }
        free(ls[i]);
    }
    Darray_destroy(ls);
    return result;
}

// platform:

#ifdef __unix__

// returns -1 if source does not exist
// returns  0 if both source and target exist and target was modified last
// returns  1 if either target does not exist or source was modified after
// target
int source_modified_after_target(const char *source_path,
                                 const char *target_path)
{
    struct stat source_info = {};
    if (stat(source_path, &source_info) == -1)
    {
        return -1;
    }
    struct timespec source_mod_time = source_info.st_mtim;

    struct stat target_info = {};
    if (stat(target_path, &target_info) == -1)
    {
        return 1;
    }
    struct timespec target_mod_time = target_info.st_mtim;

    if (source_mod_time.tv_sec == target_mod_time.tv_sec)
    {
        if (source_mod_time.tv_nsec > target_mod_time.tv_nsec)
        {
            return 1;
        }
    }
    if (source_mod_time.tv_sec > target_mod_time.tv_sec)
    {
        return 1;
    }
    return 0;
}

// returns -1 if path doesnt exist (neither file nor directory)
// returns 0 if path exists and is a regular file
// returns 1 if path is a directory
int is_dir(const char *path)
{
    struct stat path_stat;
    if (stat(path, &path_stat) == -1)
    {
        return -1;
    }
    return !S_ISREG(path_stat.st_mode);
}

// returns NULL if dir does not exist
char **listdir(const char *dir_path)
{
    DIR *dir;
    struct dirent *dir_ent;
    dir = opendir(dir_path);
    if (dir == NULL)
    {
        return NULL;
    }
    char **result = Darray_create(char *, 16);
    while ((dir_ent = readdir(dir)) != NULL)
    {
        if (strcmp(dir_ent->d_name, ".") == 0 ||
            strcmp(dir_ent->d_name, "..") == 0)
        {
            continue;
        }
        Darray_push(&result, strdup(dir_ent->d_name));
    }
    return result;
}

#endif // #ifdef __unix__

#ifdef _WIN32

// returns -1 if source does not exist
// returns  0 if both source and target exist and target was modified last
// returns  1 if either target does not exist or source was modified after
// target
int source_modified_after_target(const char *source_path,
                                 const char *target_path)
{
    fprintf(stderr, "Windows is not yet supported\n");
    exit(1);
}

// returns -1 if path doesnt exist (neither file nor directory)
// returns 0 if path exists and is a regular file
// returns 1 if path is a directory
int is_dir(const char *path)
{
    fprintf(stderr, "Windows is not yet supported\n");
    exit(1);
}

// returns NULL if dir does not exist
char **listdir(const char *dir_path)
{
    fprintf(stderr, "Windows is not yet supported\n");
    exit(1);
}

#endif // #ifdef _WIN32

#endif // #ifdef BUILDLESS_INCLUDE_IMPLEMENTATION
