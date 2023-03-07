.RECIPEPREFIX=>

CC=gcc

CFLAGS= -g -std=c11 -march=native -pedantic -Wall -Wextra -Wno-implicit-fallthrough -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-pointer-arith
LFLAGS= -lpthread

SOURCE=src
TESTS=tests
BUILD=build
BIN=bin

${BUILD}/%.o: ${SOURCE}/%.c
>	${CC} ${CFLAGS} -c $^ -o $@

${BIN}/Darray_test: ${BUILD}/Darray.o
>	${CC} ${CFLAGS} ${TESTS}/Darray_test.c -o $@ $^ ${LFLAGS}

${BIN}/Hash_test: ${BUILD}/Hash.o
>	${CC} ${CFLAGS} ${TESTS}/Hash_test.c -o $@ $^ ${LFLAGS}

all: ${BIN}/Darray_test ${BIN}/Hash_test

clean:
> rm -r ${BUILD} ${BIN}
> mkdir ${BUILD} ${BIN}

run: 
>	echo -e "RUNNING DYNAMIC ARRAY TESTS\n===========================\n"
>	./${BIN}/Darray_test
>   echo -e "RUNNING HASH TABLE TESTS\n========================\n"
>   ./${BIN}/Hash_test
