C_FLAGS=-O3 -Wall -Wextra -Werror -ansi -pedantic

.PHONY: clean

run: build
	bin/main

build:
	gcc $(C_FLAGS) -o bin/main *.c

clean:
	-rm bin/*
	-rm proj1

test:
	gcc $(C_FLAGS) -o proj1 *.c
	cd ./public-tests && $(MAKE) all
	rm proj1

clean-test:
	cd ./public-tests && $(MAKE) clean
