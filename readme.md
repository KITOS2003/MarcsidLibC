Personal library of general utilities for the C programing language

pick from the src/include directory whatever you want and add it to your include directory

pick the corresponding C file and add it to your source directory


Description:
============

Darray.h:
dynamic array implementation

Hash.h:
Hash table implementation, note that this is not a container, it is better thought as a set of functions that map char* to void*, what those point to is the responsibility of the user.
inserting something with a given key(passing it a char*) and then changing that string WILL result in memory leakage for as long as the hash table instance exists.


