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
    _Darray_create(sizeof(type), capacity, _buildless_malloc,                  \
                   _buildless_realloc, _buildless_free)

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

void Darray_free_all(void *data);

#endif // #ifndef BUILDLESS_NODEFINE_DARRAY

#define STR_ARRAY(...) _Darray_from_strings(__VA_ARGS__, NULL)
char **_Darray_from_strings(const char *first, ...);

// "Garbage collection"

static void **tracked_memory = NULL;

static inline void _buildless_track_memory();

static inline void *_buildless_malloc(size_t size);
static inline void *_buildless_realloc(void *ptr, size_t size);
static inline void _buildless_free(void *ptr);

static inline void _buildless_clean_memory();

// string utils

char *str_join(char **str_arr);
bool str_begins_with(const char *string, const char *prefix);
bool str_ends_with(const char *string, const char *postfix);
size_t str_find(const char *string, const char *substring, size_t *end);
size_t *str_find_char(const char *string, char c);
const char *str_get_path(const char *path);

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

void _buildless_init(int argc, const char **argv, const char *file);
#define buildless_init(argc, argv) _buildless_init(argc, argv, __FILE__)

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
    Darray *self = allocator(element_size * initial_capacity + sizeof(Darray));
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

#define DARRAY_FREE_ALL(array)                                                 \
    {                                                                          \
        for (size_t i = 0; i < Darray_length(array); i += 1)                   \
        {                                                                      \
            _buildless_free(array[i]);                                         \
        }                                                                      \
        Darray_destroy(array);                                                 \
    }

#endif // #ifndef BUILDLESS_NOINCLUDE_DARRAY_IMPLEMENTATION

// "garbage collection" implementation:

static inline void *_buildless_malloc(size_t size)
{
    if (tracked_memory == NULL)
    {
        return malloc(size);
    }
    fprintf(stderr, "UNREACHABLE %s\n", __FUNCTION__);
    exit(1);
}

static inline void *_buildless_realloc(void *ptr, size_t size)
{
    if (tracked_memory == NULL)
    {
        return realloc(ptr, size);
    }
    fprintf(stderr, "UNREACHABLE %s\n", __FUNCTION__);
    exit(1);
}

static inline void _buildless_free(void *ptr)
{
    if (tracked_memory == NULL)
    {
        free(ptr);
        return;
    }
    fprintf(stderr, "UNREACHABLE %s\n", __FUNCTION__);
    exit(1);
}

static inline void _buildless_track_memory()
{
    tracked_memory = Darray_create(void *, 256);
}

static inline void _buildless_clean_memory()
{
    Darray_destroy(tracked_memory);
    tracked_memory = NULL;
}

// string utils

// allocates result
char *str_join(char **str_arr)
{
    size_t str_size = 0;
    for (size_t i = 0; i < Darray_length(str_arr); i += 1)
    {
        str_size += strlen(str_arr[i]) + 1;
    }
    char *result = _buildless_malloc(str_size);
    *result = 0;
    strcat(result, str_arr[0]);
    for (size_t i = 1; i < Darray_length(str_arr); i += 1)
    {
        strcat(result, " ");
        strcat(result, str_arr[i]);
    }
    return result;
}

// allocates result (Darray)
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
size_t str_find(const char *string, const char *substring, size_t *end)
{
    size_t susbstring_index = 0;
    size_t result = 0;
    for (size_t i = 0; i <= strlen(string); i += 1)
    {
        if (substring[susbstring_index] == 0)
        {
            *end = i;
            return result;
        }
        if (string[i] != substring[susbstring_index])
        {
            result = i + 1;
            susbstring_index = 0;
        }
        else
        {
            susbstring_index += 1;
        }
    }
    *end = -1;
    return -1;
}

// Allocates result and its members
char **str_split(const char *string, char c)
{
    char **result = Darray_create(char *, 16);
    size_t *indices = str_find_char(string, c);
    if (indices == NULL)
    {
        return NULL;
    }
    size_t prev_ind = 0;
    for (size_t i = 0; i <= Darray_length(indices); i += 1)
    {
        size_t ind;
        if (i == Darray_length(indices))
        {
            ind = strlen(string);
        }
        else
        {
            ind = indices[i];
        }
        char *s = _buildless_malloc((ind - prev_ind + 1) * sizeof *s);
        memcpy(s, &string[prev_ind], (ind - prev_ind) * sizeof *s);
        s[ind - prev_ind] = 0;
        Darray_push(&result, s);
        prev_ind = ind + 1;
    }
    Darray_destroy(indices);
    return result;
}

// allocates result
#define str_concat(...) _str_concat_va(__VA_ARGS__, NULL)
const char *_str_concat_va(const char *first, ...)
{
    size_t allocated_size = 512;
    char *result = _buildless_malloc(512 * sizeof *result);
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
        result = _buildless_realloc(result, allocated_size * sizeof *result);
        strcat(result, s);
    }
    va_end(args);
    /* gc_add(result); */
    return (const char *)result;
}

const char *str_get_path(const char *path)
{
    size_t *indices = str_find_char(path, DIR_DIVISOR_CHAR);
    if (indices == NULL)
    {
        return NULL;
    }
    size_t index = indices[Darray_length(indices) - 1];
    Darray_destroy(indices);
    char *result = _buildless_malloc((index + 1) * sizeof *result);
    memcpy(result, path, index * sizeof *result);
    result[index] = 0;
    return result;
}

// generics

bool generic_cmp(const char *generic, const char *string, char ***matches_out)
{
    char **split = str_split(generic, '@');
    if (split == NULL)
    {
        if (strcmp(generic, string) == 0)
        {
            return true;
        }
        return false;
    }
    if (!str_begins_with(string, split[0]) ||
        !str_ends_with(string, split[Darray_length(split) - 1]))
    {
        if (matches_out != NULL)
            *matches_out = NULL;
        return false;
    }
    size_t *start_indices = Darray_create(size_t, 16);
    size_t *end_indices = Darray_create(size_t, 16);
    const char *s = string;
    for (size_t i = 0; i < Darray_length(split); i += 1)
    {
        size_t end = 0;
        size_t ind = str_find(s, split[i], &end);
        if (ind == -1)
        {
            if (matches_out != NULL)
                *matches_out = NULL;
            Darray_destroy(start_indices);
            Darray_destroy(end_indices);
            return false;
        }
        Darray_push(&start_indices, s - string + ind);
        Darray_push(&end_indices, s - string + end);
        s += end;
    }
    if (s - string != strlen(string))
    {
        size_t ind = Darray_length(start_indices) - 1;
        start_indices[ind] = strlen(string);
        end_indices[ind] = strlen(string);
    }
    char **matches = Darray_create(char *, 16);
    for (size_t i = 0; i < Darray_length(start_indices) - 1; i += 1)
    {
        size_t match_size = start_indices[i + 1] - end_indices[i];
        char *match = _buildless_malloc((match_size + 1) * sizeof *match);
        memcpy(match, &string[end_indices[i]], match_size * sizeof *match);
        match[match_size] = 0;
        Darray_push(&matches, match);
    }
    if (matches_out != NULL)
    {
        *matches_out = matches;
    }
    else
    {
        DARRAY_FREE_ALL(matches);
    }
    DARRAY_FREE_ALL(split);
    Darray_destroy(start_indices);
    Darray_destroy(end_indices);
    return true;
}

const char *generic_to_literal(const char *generic, char **matches)
{
    if (matches == NULL)
    {
        return strdup(generic);
    }
    size_t *indices = str_find_char(generic, '@');
    if (indices == NULL)
    {
        return generic;
    }
    if (Darray_length(indices) != Darray_length(matches))
    {
        fprintf(stderr,
                "[ERROR:] Cant convert generic to literal since the number of "
                "@s and the number of matches are different: %lu %lu\n",
                Darray_length(indices), Darray_length(matches));
        exit(1); // TODO
    }
    size_t matches_total_size = 0;
    for (size_t i = 0; i < Darray_length(matches); i += 1)
    {
        matches_total_size += strlen(matches[i]);
    }
    char *result = _buildless_malloc((strlen(generic) + matches_total_size) *
                                     sizeof *result);
    *result = 0;
    char **split = str_split(generic, '@');
    for (size_t i = 0; i < Darray_length(matches); i += 1)
    {
        strcat(result, split[i]);
        strcat(result, matches[i]);
    }
    strcat(result, split[Darray_length(split) - 1]);
    Darray_destroy(indices);
    DARRAY_FREE_ALL(split);
    return result;
}

/* * * buildless core * * */

// options passed through command line
struct
{
    bool force;
} OPTS;

void _buildless_process_opt(const char *opt)
{
    if (strcmp(opt, "--force") == 0 || strcmp(opt, "-f") == 0)
    {
        OPTS.force = true;
    }
}

void command_va(char *format, ...)
{
    char *cmd_string = _buildless_malloc(4000);
    va_list args;
    va_start(args, format);
    vsprintf(cmd_string, format, args);
    va_end(args);
    printf("[CMD:] %s\n", cmd_string);
    fflush(stdout);
    system(cmd_string);
    _buildless_free(cmd_string);
}

void _buildless_init(int argc, const char **argv, const char *file)
{
#ifndef BUILDLESS_DEBUG
    if (source_modified_after_target(file, argv[0]) == 1)
    {
        const char *exec_name = argv[0];
        argc -= 1;
        argv += 1;
        size_t str_size = 0;
        for (int i = 0; i < argc; i += 1)
        {
            str_size += strlen(argv[i]) + 1;
        }
        char *args = _buildless_malloc((str_size + 1) * sizeof *args);
        *args = 0;
        for (int i = 0; i < argc; i += 1)
        {
            strcat(args, argv[i]);
            strcat(args, " ");
        }
        command_va("%s %s -o %s", COMPILER, file, exec_name);
        command_va("%s --force %s", exec_name, args);
        exit(0);
    }
#endif // #ifndef BUILDLESS_DEBUG
}

void _buildless_go(int argc, const char *argv[], Rule *all, size_t n_rules,
                   char *file)
{
    argc -= 1;
    argv += 1;

    for (int i = 0; i < argc; i += 1)
    {
        if (argv[i][0] == '-')
        {
            _buildless_process_opt(argv[0]);
            argc -= 1;
            argv += 1;
        }
        else
        {
            break;
        }
    }

    if (argc == 0)
    {
        buildless_make_rule(all, all, n_rules, NULL, NULL);
    }
    if (argc > 0)
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
                if (matches != NULL)
                    DARRAY_FREE_ALL(matches);
            }
        }
    }
}

bool buildless_make_rule(Rule *rule, Rule *all, size_t n_rules,
                         const char *target, char **matches)
{
    bool should_run = OPTS.force;
    if (target == NULL)
    {
        target = rule->target;
    }
    {
        const char *path = str_get_path(target);
        if (path != NULL)
        {
            mkdir_r(path);
            _buildless_free((void *)path);
        }
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
                    DARRAY_FREE_ALL(new_matches);
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
        /* gc_collect_all(); */
        return true;
    }
    /* gc_collect_all(); // TODO FIX GC!!! */
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
    Darray_destroy(indices);
    _buildless_free(dir_copy);
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
        new_path = _buildless_malloc((strlen(dir_path) + strlen(ls[i]) + 1) *
                                     sizeof *new_path);
        *new_path = 0;
        strcat(new_path, dir_path);
        strcat(new_path, DIR_DIVISOR_CHAR_STR);
        strcat(new_path, ls[i]);
        if (is_dir(new_path) == 1)
        {
            char **ls_r = listdir_r(new_path);
            Darray_merge(&result, ls_r);
            Darray_destroy(ls_r);
            _buildless_free(new_path);
        }
        else
        {
            Darray_push(&result, new_path);
        }
        _buildless_free(ls[i]);
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
