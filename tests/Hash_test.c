#include<stdio.h>
#include <stdlib.h>

#include"../src/include/Hash.h"

struct data 
{
    int a, b, c;
};

int main()
{
    HashTable ht = HashTable_create(10, 6275141);


    struct data ex = { 1, 5, 6 };

    HashTable_add_entry( &ht, "hello world", &ex );
    HashTable_add_entry( &ht, "hi mom", NULL );
    HashTable_add_entry( &ht, "jklas", NULL );
    HashTable_add_entry( &ht, "juan", NULL );
    HashTable_add_entry( &ht, "nigg", NULL );
    HashTable_add_entry( &ht, "wtd", NULL );
    HashTable_add_entry( &ht, "dd", NULL );
    HashTable_add_entry( &ht, "aas", NULL );
    HashTable_add_entry( &ht, "aa", NULL );
    HashTable_add_entry( &ht, "a", NULL );
    HashTable_add_entry( &ht, "gg", NULL );
    HashTable_add_entry( &ht, "f", NULL );
    HashTable_add_entry( &ht, "dwer", NULL );
    HashTable_add_entry( &ht, "ty", NULL );
    HashTable_add_entry( &ht, "llo", NULL );
    HashTable_add_entry( &ht, "ttui", NULL );


    struct data *dat = HashTable_get_entry( &ht, "hello world" );


    printf("%d, %d, %d\n", dat->a, dat->b, dat->c );

    HashTable_print(&ht);

    return 0;
}
