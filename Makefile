cfiles := $(wildcard *.c)

compile: $(cfiles) $(hfiles)
	gcc -O2 -o nondeterministic_tm $(cfiles) -lm
