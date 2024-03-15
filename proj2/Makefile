C_FLAGS=-O3 -Wall -Wextra -Werror -ansi -pedantic
DBG_MACROS=-D DEBUG
MEM_MACROS=-D CONTROL_MEMORY_ALLOCS -D LAST_MEM_LIST
VAL_FLAGS=--leak-check=full

.PHONY: clean

run: bin/main
	bin/main

build: bin/main

bin/main: *.c
	gcc $(C_FLAGS) $(DBG_MACROS) $(MEM_MACROS) -o bin/main *.c

debug: bin/dbg
	valgrind $(VAL_FLAGS) bin/dbg

bin/dbg: *.c
	gcc $(C_FLAGS) $(DBG_MACROS) -g -o bin/dbg *.c

test:
	gcc $(C_FLAGS) -o proj2 *.c
	cd ./public-tests && $(MAKE) all
	rm proj2

clean:
	-rm bin/*
	-rm proj2

clean-test:
	cd ./public-tests && $(MAKE) clean
