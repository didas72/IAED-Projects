# Copyright (C) 2021, Pedro Reis dos Santos
.SUFFIXES: .in .out .diff
MAKEFLAGS += --no-print-directory # No entering and leaving messages
SHELL := /bin/bash # Execute command with bash
OK="\e[1;32mtest $< PASSED\e[0m"
KO="\e[1;31mtest $< FAILED\e[0m"
EXE=../proj
LOG=tests.log
#TIMEFORMAT=%R (real) %U (user) %S (system)
CFLAGS=-g

all:: clean # run regression tests
	@rm -f $(LOG)
	@for i in `ls test*.in | sed -e "s/in/diff/"`; do $(MAKE) $(MFLAGS) $$i; done
	@echo "`wc -l < $(LOG)` tests passed"

timed:
	TIMEFORMAT=%R $(MAKE) $(MFLAGS) EXE="time $(EXE)"

timeout:
	TIMEFORMAT=%R $(MAKE) $(MFLAGS) EXE="timeout 3 $(EXE)"

.in.diff:
	@-if [ -f $*.arg ]; then $(EXE) `cat $*.arg` < $< > $*.myout; else $(EXE) < $< > $*.myout; fi
	@-diff $*.myout $*.out > $@
	@if [ `wc -l < $@` -eq 0 ]; then echo -e $(OK); echo $* >> $(LOG); else echo -e $(KO); fi;

.in.out:
	@-if [ -f $*.arg ]; then $(EXE) `cat $*.arg` < $< > $@; else $(EXE) < $< > $@; fi
	@echo $@

clean::
	rm -rf *.diff *.myout $(LOG) __pycache__

