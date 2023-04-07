#include "../src/Buildless.c"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void test_end_beg(const int argc, const char *argv[])
{
    if (argc > 2)
    {
        int beg = str_begins_with(argv[1], argv[2]);
        int end = str_ends_with(argv[1], argv[3]);
        printf("%d\n", beg);
        printf("%d\n", end);
    }
}

void test_generic_cmp(const int argc, const char *argv[])
{
    if (argc >= 2)
    {
        char **matches = NULL;
        bool result = generic_cmp(argv[1], argv[2], &matches);
        printf("%d\n", result);
        if (result)
        {
            for (size_t i = 0; i < Darray_length(matches); i += 1)
            {
                if (*matches[i] == 0)
                {
                    printf("-- EMPTY --\n");
                }
                else
                {
                    printf("%s\n", matches[i]);
                }
            }
            const char *str = generic_to_literal(argv[3], matches);
            if (str != NULL)
                printf("%s\n", str);
        }
    }
}

void test_str_find(int argc, char *argv[])
{
    if (argc >= 2)
    {
        size_t end;
        size_t start = str_find(argv[1], argv[2], &end);
        printf("%ld %ld\n", (int64_t)start, (int64_t)end);
    }
}

void test_mkdirs(const int argc, const char **argv)
{
    if (argc > 0)
        mkdir_r(argv[1]);
}

void test_listdir(const int argc, const char **argv)
{
    if (argc > 0)
    {
        char **arr = listdir_r(argv[1]);
        if (arr == NULL)
            return;

        for (size_t i = 0; i < Darray_length(arr); i += 1)
        {
            printf("%s\n", arr[i]);
        }
        Darray_destroy(arr);
    }
}

void test_concat(const int argc, const char *argv[])
{
    if (argc > 2)
    {
        printf("%s\n", str_concat(argv[0], argv[1], argv[2], argv[3]));
    }
}

void test_str_split(const int argc, const char *argv[])
{
    if (argc > 1)
    {
        char **split = str_split(argv[1], '@');
        for (size_t i = 0; i < Darray_length(split); i += 1)
        {
            if (*(split[i]) == 0)
            {
                printf("-- EMPTY --\n");
            }
            else
            {
                printf("%s\n", split[i]);
            }
        }
    }
}

int main(const int argc, const char *argv[])
{
    const char *arr[] = {NULL, "aaa@bbb", "aaa_hello_bbb"};
    /* test_generic_cmp(6, arr); */
    /* test_generic_cmp(argc, argv); */
    /* test_str_find(argc, argv); */
    /* test_mkdirs(argc, argv); */
    /* test_listdir(argc, argv); */
    /* printf("%d\n", is_dir(argv[1])); */
    /* test_concat(argc, argv); */
    /* test_str_split(argc, argv); */
}
