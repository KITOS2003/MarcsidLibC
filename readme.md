# Library of utilities for the C programming language

All the C files are meant to be used as STB style headers, that means that in
one translation unit you will need to have:

```c
#define X_INCLUDE_IMPLEMENTATION
#include"X.c"
```

Then include them normally in all other translation units.
