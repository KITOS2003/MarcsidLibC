#include<stdio.h>
#include<stdint.h>


#include"../src/include/Darray.h"

void f( void *arg )
{
    float *value = arg;
    printf("%f", *value );
}

int main()
{
    float *arr = Darray_create(float, 3);

    float eee[] = { 1.0f, 23.5f, 56.9f, 45.7f, 44.5f, 3.33f, 67.0f };

    float a = 0.0f;

    Darray_push_multiple( &arr, &eee, 7 );
    Darray_pop( &arr, &a );

    printf("%f\n\n\n", a);

    Darray_print( arr, NULL, f );
}
