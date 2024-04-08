CC=gcc
C_FLAGS=-O3 -Wall -Wextra -Wno-unused-result
DBG_FLAGS=-Wall -Wextra -Wno-unused-result -ggdb -DUSE_ALLOC_CHECK
GDB_FLAGS=-q -ex "set print pretty on"

VAL_FLAGS=--leak-check=full --show-leak-kinds=all --track-origins=yes -s

DIR_SRC=src
DIR_INC=include
DIR_LIB=lib
DIR_BUILD=build

LIBS=alloc_check

OUT_BIN=$(DIR_BUILD)/bin/main
DBG_BIN=$(DIR_BUILD)/bin/dbg

SRCS=$(shell find $(DIR_SRC)/ -type f -name '*.c')
LIB_FLAGS=$(patsubst %,-l%,$(LIBS))


.PHONY: all build run run-dbg dbg val clean test clean-tests


all: build
build: $(OUT_BIN)
build-dbg: $(DBG_BIN)



run: $(OUT_BIN)
	./$(OUT_BIN)

run-dbg: $(DBG_BIN)
	./$(DBG_BIN)

$(OUT_BIN): $(SRCS)
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) -I$(DIR_INC) -L$(DIR_LIB) -o $@ $^ $(LIB_FLAGS)

$(DBG_BIN): $(SRCS)
	@mkdir -p $(@D)
	$(CC) $(DBG_FLAGS) -I$(DIR_INC) -L$(DIR_LIB) -o $@ $^ $(LIB_FLAGS)

dbg: $(DBG_BIN)
	gdb $(GDB_FLAGS) ./$(DBG_BIN)

val: $(DBG_BIN)
	valgrind $(VAL_FLAGS) ./$(DBG_BIN)

test: $(OUT_BIN)
	cd public-tests && $(MAKE)
	cd private-tests && $(MAKE)

clean:
	$(RM) -r $(DIR_BUILD)

clean-tests:
	cd public-tests && $(MAKE) clean
	cd private-tests && $(MAKE) clean

loc:
	scc -s lines --no-cocomo --no-gitignore -w --size-unit binary --exclude-ext md,makefiles src include
