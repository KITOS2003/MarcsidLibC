#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<string.h>

#include<bits/wordsize.h>

#include"include/Hash.h"

#define INITIAL_INDEX_STACK_CAPACITY 256
#define HASH_TABLE_DARRAY_MIN_CAPACITY 64


/***********************************/  
/**DARRAY CREATION AND DESTRUCTION**/
/***********************************/

static inline HashTableDarray HashTableDarray_create
    ( 
        size_t capacity,
        void* (*allocator)( size_t ),
        void* (*reallocator)( void*, size_t ),
       void  (*liberator)( void* )
    )
{
    HashTableEntry *data = allocator( capacity * sizeof(HashTableEntry) );
    size_t *index_stack = allocator( INITIAL_INDEX_STACK_CAPACITY * sizeof(size_t) );

    *index_stack = 0;

    HashTableDarray result = 
    {
        .data = data,
        .n_elements = 0,
        .capacity = capacity,
        .reserve_space = capacity,
        .allocator = allocator,
        .reallocator = reallocator,
        .liberator = liberator,
        .index_stack = index_stack,
        .index_stack_top = index_stack,
        .index_stack_capacity = INITIAL_INDEX_STACK_CAPACITY,
    };
    return result;
}

static inline void HashTableDarray_destroy( HashTableDarray *self )
{
    self->liberator(self->data);
    self->liberator(self->index_stack);
    memset( self, 0, sizeof(HashTableDarray) );
}

/***********************************/  
/**DARRAY INDEX STACK MANIPULATION**/
/***********************************/

static inline void HashTableDarray_index_stack_push( HashTableDarray *self, size_t index )
{
    self->index_stack_top += 1;
    size_t index_stack_n_elements = self->index_stack_top - self->index_stack + 1;
    if( index_stack_n_elements >= self->index_stack_capacity )
    {
        self->index_stack_capacity *= 2;
        self->index_stack = self->reallocator( self->index_stack, self->index_stack_capacity*sizeof(size_t));
        self->index_stack_top = self->index_stack + index_stack_n_elements-1;
    }
    *self->index_stack_top = index;
}

static inline size_t HashTableDarray_index_stack_pop( HashTableDarray *self )
{
    size_t result = *self->index_stack_top;
    size_t index_stack_n_elements = self->index_stack_top - self->index_stack + 1;
    if( index_stack_n_elements == 1 )
    {
        *self->index_stack += 1;
        return result;
    }
    if
        (
            index_stack_n_elements < self->index_stack_capacity/4
                &&
            self->index_stack_capacity/4 >= INITIAL_INDEX_STACK_CAPACITY
        )
    {
        self->index_stack_capacity /= 2;
        self->index_stack = self->reallocator(self->index_stack, self->index_stack_capacity*sizeof(size_t));
        self->index_stack_top = self->index_stack + index_stack_n_elements-1;
    }
    self->index_stack_top -= 1;
    return result;
}

/***********************************/  
/****DARRAY ELEMENT MANIPULATION****/
/***********************************/

static inline HashTableEntry *HashTableDarray_push( HashTableDarray *self, HashTableEntry *entry )
{
    size_t index = HashTableDarray_index_stack_pop( self );
    if( index+1 >= self->capacity )
    {
        self->capacity *= 2;
        self->data = self->reallocator(self->data, self->capacity*sizeof(HashTableEntry));
    }
    self->data[index] = *entry;

    return &(self->data[index]);
}

static inline void HashTableDarray_pop( HashTableDarray *self, HashTableEntry *entry )
{
    if( entry - self->data < self->capacity-1 )
    {
        HashTableDarray_index_stack_push(self, entry-self->data);
        memset( entry, 0, sizeof(HashTableEntry) );
    }
}

/***************************************/  
/**HASH TABLE CREATION AND DESTRUCTION**/
/***************************************/

HashTable _HashTable_create
    (
        size_t table_size, 
        size_t seed,
        void* (*allocator)(size_t),
        void* (*reallocator)(void*,size_t),
        void  (*liberator)(void*)
    )
{
    size_t hash_table_darray_initial_capacity = table_size/2;
    if( hash_table_darray_initial_capacity < HASH_TABLE_DARRAY_MIN_CAPACITY )
    {
        hash_table_darray_initial_capacity = HASH_TABLE_DARRAY_MIN_CAPACITY;
    }
    HashTableDarray darray = HashTableDarray_create(hash_table_darray_initial_capacity,allocator,reallocator,liberator);

    HashTableEntry **table = allocator( table_size * sizeof(HashTableEntry*) );

    memset(table, 0, table_size);

    HashTable result = 
    {
        .table_size = table_size,
        .table = table,
        .entries = darray,
        .seed = seed,
    };
    return result;
}

void HashTable_destroy( HashTable *self )
{
    self->entries.liberator(self->table);
    HashTableDarray_destroy(&self->entries);
    memset( self, 0, sizeof(HashTable) );
}

/***************************************/  
/*****HASH TABLE ENTRY MANIPULATION*****/
/***************************************/

void HashTable_add_entry( HashTable *self, const char *key, const void *data )
{
    HashTableEntry write = 
    {
        .key = (char*)key,
        .data = (void*)data,
        .next = NULL,
    };
    size_t hash = MurmurHash2( key, strlen(key), self->seed ) % self->table_size;
    HashTableEntry *entry = self->table[hash];
    if( entry == NULL )
    {
        self->table[hash] = HashTableDarray_push(&self->entries, &write);
        return;
    }
    HashTableEntry *next = entry->next;
    while( 1 )
    {
        if( strcmp( key, entry->key ) == 0 )
        {
            entry->data = (void*)data;
            return;
        }
        if( next == NULL )
        {
            break;
        }
        entry = entry->next;
        next = next->next;
    }
    entry->next = HashTableDarray_push(&self->entries, &write);
}

void HashTable_remove_entry( HashTable *self, char *key )
{
    size_t hash = MurmurHash2( key, strlen(key), self->seed ) % self->table_size;
    HashTableEntry **entry = &(self->table[hash]);
    while( (*entry) != NULL )
    {
        if( strcmp(key, (*entry)->key) == 0 )
        {
            HashTableEntry *temp = *entry;
            (*entry) = (*entry)->next;
            HashTableDarray_pop( &self->entries, temp );
            break;
        }
        entry = &((*entry)->next);
    }
}

void *HashTable_get_entry( HashTable *self, const char *key )
{
    size_t hash = MurmurHash2( key, strlen(key), self->seed ) % self->table_size;
    HashTableEntry *entry = self->table[hash];
    while( entry != NULL )
    {
        if( strcmp(key, entry->key) == 0 )
        {
            return entry->data;
        }
        entry = entry->next;
    }
    return NULL;
}

/***************************************/  
/*****************OTHER*****************/
/***************************************/

void HashTable_print( HashTable *self )
{
    printf("################################\n");
    for( uint32_t i = 0; i < self->table_size; i++ )
    {
        if( self->table[i] == NULL )
        {
            printf("-----------\n");
        }
        else
        {
            printf("\"%s\"", self->table[i]->key );
            HashTableEntry *entry = self->table[i]->next;
            while( entry != NULL )
            {
                printf(" -> \"%s\"", entry->key );
                entry = entry->next;
            }
            printf("\n");
        }
    }
    printf("################################\n");
}

void HashTable_resize( HashTable *self, size_t new_size, size_t new_seed )
{
    self->entries.liberator(self->table);
    self->table = self->entries.allocator( new_size * sizeof(HashTableEntry*) );

    self->table_size = new_size;
    if( new_seed != 0 )
        self->seed = new_seed;

    HashTableDarray *darray = &self->entries;
    for( uint32_t i = 0; i < *(darray->index_stack); i++ )
    {
        if( darray->data[i].key != NULL )
        {
            darray->data[i].next = NULL;
            size_t hash = MurmurHash2( darray->data[i].key, strlen(darray->data[i].key), self->seed ) % self->table_size;
            HashTableEntry **entry = &self->table[hash];
            while( *entry != NULL )
            {
                entry = &((*entry)->next);
            }
            *entry = &(darray->data[i]);
        }
    }
}

/***************************************/  
/*************HASH FUNCTION*************/
/***************************************/

#if __WORDSIZE == 32 

uint32_t MurmurHash2 ( const void * key, int len, uint32_t seed )
{
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    uint32_t h = seed ^ len;

    const unsigned char * data = (const unsigned char *)key;

    while(len >= 4)
    {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch(len)
    {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
        h *= m;
    };

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
} 

#elif __WORDSIZE == 64

uint64_t MurmurHash2 ( const void * key, int len, uint64_t seed )
{
    const uint64_t m = 0xc6a4a7935bd1e995LLU;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t * data = (const uint64_t *)key;
    const uint64_t * end = data + (len/8);

    while(data != end)
    {
        uint64_t k = *data++;

        k *= m; 
        k ^= k >> r; 
        k *= m; 

        h ^= k;
        h *= m; 
    }

    const unsigned char * data2 = (const unsigned char*)data;

    switch(len & 7)
    {
        case 7: h ^= ((uint64_t) data2[6]) << 48;
        case 6: h ^= ((uint64_t) data2[5]) << 40;
        case 5: h ^= ((uint64_t) data2[4]) << 32;
        case 4: h ^= ((uint64_t) data2[3]) << 24;
        case 3: h ^= ((uint64_t) data2[2]) << 16;
        case 2: h ^= ((uint64_t) data2[1]) << 8;
        case 1: h ^= ((uint64_t) data2[0]);
        h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
} 

#endif

