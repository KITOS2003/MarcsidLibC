#include <stdio.h>
#define BUILDLESS_INCLUDE_IMPLEMENTATION
#include "src/Buildless.c"

#define SRC "src"
#define BUILD "build"
#define BIN "bin"

void build_objects(char **dependencies, const char *target, void *args)
{
    command_va("%s -c %s -o %s", COMPILER, dependencies[0], target);
}

void build_target(char **dependencies, const char *target, void *args)
{
    command_va("%s -o %s %s", COMPILER, target, str_join(dependencies));
}

int main(const int argc, const char *argv[])
{
    buildless_init(argc, argv);
    int i = 0;

    Rule rules[] = {
        {
            .target = "all",
            .dependencies = STR_ARRAY("bin/Darray_test", "bin/Hash_test"),
            .callback = NULL,
        },
        {
            .target = "bin/@",
            .dependencies = STR_ARRAY("build/@.o"),
            .callback = build_target,
        },
        {
            .target = "build/@.o",
            .dependencies = STR_ARRAY("tests/@.c"),
            .callback = build_objects,
        },
    };

    buildless_go(argc, argv, rules);
}
