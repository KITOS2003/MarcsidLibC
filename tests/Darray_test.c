#include<stdio.h>
#include<stdint.h>


#include"../src/include/Darray.h"

int main()
{
    float *arr = Darray_create(float, 3);

    Darray_push(arr, 3.8f);
    Darray_push(arr, 7.76456f);
    Darray_push(arr, 3.8f);
    /* Darray_push(arr, 3.8f); */
    /* Darray_push(arr, 3.8f); */
    /* Darray_push(arr, 3.8f); */
    /* Darray_push(arr, 3.8f); */
    /* Darray_push(arr, 7.76456f); */
    /* Darray_push(arr, 7.76456f); */
    /* Darray_push(arr, 7.76456f); */
    /* Darray_push(arr, 7.76456f); */
    /* Darray_push(arr, 7.76456f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */
    /* Darray_push(arr, 1.1111f); */

    Darray_print(arr, "%f");
}
