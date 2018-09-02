CC:=gcc
CFLAGS:=-Wall

.PHONY : all run clean

all : c11 c99 c90

c11 : c_version.c
	$(CC) $(CFLAGS) -std=c11 $< -o $@

c99 : c_version.c
	$(CC) $(CFLAGS) -std=c99 $< -o $@

c90 : c_version.c
	$(CC) $(CFLAGS) -std=c90 $< -o $@

run : c11 c99 c90
	./c11
	./c99
	./c90

clean :
	-rm c11 c99 c90

