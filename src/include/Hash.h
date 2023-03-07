#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <stdlib.h>

#include <bits/wordsize.h>

typedef struct HashTableEntry {
  char *key;
  void *data;
  struct HashTableEntry *next;
} HashTableEntry;

typedef struct HashTableDarray {
  HashTableEntry *data;

  size_t n_elements;
  size_t capacity;
  size_t reserve_space;

  void *(*allocator)(size_t);
  void *(*reallocator)(void *, size_t);
  void (*liberator)(void *);

  size_t *index_stack;
  size_t *index_stack_top;
  size_t index_stack_capacity;
} HashTableDarray;

typedef struct HashTable {
  HashTableEntry **table;
  size_t table_size;
  size_t seed;

  HashTableDarray entries;
} HashTable;

HashTable _HashTable_create(size_t table_size, size_t seed,
                            void *(*allocator)(size_t),
                            void *(*reallocator)(void *, size_t),
                            void (*liberator)(void *));

#if __WORDSIZE == 32
uint32_t MurmurHash2(const void *key, int len, uint32_t seed)
#elif __WORDSIZE == 64
uint64_t MurmurHash2(const void *key, int len, uint64_t seed);
#endif

    void HashTable_destroy(HashTable *self);
void HashTable_add_entry(HashTable *self, const char *key, const void *data);
void HashTable_remove_entry(HashTable *self, char *key);
void *HashTable_get_entry(HashTable *self, const char *key);
void HashTable_print(HashTable *self);
void HashTable_resize(HashTable *self, size_t new_size, size_t new_seed);

// wrapper macros
#define HashTable_create(table_size, seed)                                     \
  _HashTable_create(table_size, seed, malloc, realloc, free)
#define HashTable_create_allocator(table_size, seed, malloc, realloc, free)    \
  _HashTable_create(table_size, seed, malloc, realloc, free)

#endif
