CC=gcc
LD=ld
C_FLAGS=-Wall -Wextra -Wno-unused-result -O0 -ggdb -DSUS_TARGET_VERSION=10000
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

CGC=$(DIR_BUILD)/lib/libcgc.so
CGC_PRELOAD=LD_PRELOAD=$(CGC)


.PHONY: all build run dbg val clean test clean-tests


all: build
build: $(OUTBIN)
rebuild: clean .WAIT build


run: $(OUTBIN)
	$(CGC_PRELOAD) ./$(OUTBIN)

$(OUTBIN): $(OBJS) $(CGC)
	@mkdir -p $(@D)
	$(CC) $(OBJS) $(DEPS_EXT) -o $@

$(DIR_BUILD)/obj/%.o: $(DIR_SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) $(INCS_EXT) -c $< -o $@

$(CGC): cgc.c
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) -c $< -o $(DIR_BUILD)/obj/cgc.o
	$(LD) -r $(DIR_BUILD)/obj/cgc.o -L/usr/local/lib -lsus -o $(DIR_BUILD)/obj/cgc.so.o
	$(CC) $(DIR_BUILD)/obj/cgc.so.o -shared -o $@

dbg: $(DBG_BIN)
	gdb $(GDB_FLAGS) ./$(OUTBIN)

val: $(DBG_BIN)
	valgrind $(VAL_FLAGS) ./$(OUTBIN)

clean:
	$(RM) -r $(DIR_BUILD)

loc:
	scc -s lines --no-cocomo --no-gitignore -w --size-unit binary --exclude-ext md,makefiles src include
