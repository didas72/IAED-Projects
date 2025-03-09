CC=gcc
C_FLAGS=-g -DDEBUG -Wall -Wextra -Wno-unused-result
#C_FLAGS=-O3 -Wall -Wextra -Wno-unused-result
VAL_FLAGS=--leak-check=full --show-leak-kinds=all --track-origins=yes -s

DIR_SRC=src
DIR_INC=include
DIR_BUILD=build
DEPS=sus

OUTBIN=$(DIR_BUILD)/bin/main

SRCS=$(shell find $(DIR_SRC)/ -type f -name '*.c')
OBJS=$(patsubst $(DIR_SRC)/%.c,$(DIR_BUILD)/obj/%.o,$(SRCS))

DEPS_EXT=$(patsubst %,-l%,$(DEPS))
INCS_EXT=$(patsubst %,-I%,$(DIR_INC))


.PHONY: all build run dbg val clean test clean-tests


all: build
build: $(OUTBIN)
rebuild: clean .WAIT build



run: $(OUTBIN)
	./$(OUTBIN)

$(OUTBIN): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(OBJS) $(DEPS_EXT) -o $@

$(DIR_BUILD)/obj/%.o: $(DIR_SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) $(INCS_EXT) -c $< -o $@

dbg: $(DBG_BIN)
	gdb $(GDB_FLAGS) ./$(OUTBIN)

val: $(DBG_BIN)
	valgrind $(VAL_FLAGS) ./$(OUTBIN)

test: $(OUTBIN)
	cd public-tests && $(MAKE)
	cd private-tests && $(MAKE)

clean:
	$(RM) -r $(DIR_BUILD)

clean-tests:
	cd public-tests && $(MAKE) clean
	cd private-tests && $(MAKE) clean

loc:
	scc -s lines --no-cocomo --no-gitignore -w --size-unit binary --exclude-ext md,makefiles src include
