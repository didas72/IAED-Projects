build: test.elf
test.elf: cgc.c test.c
	gcc -g -o $@ -DSUS_TARGET_VERSION=10000 $^ -lsus

run: test.elf
	./$<
