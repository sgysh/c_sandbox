CC:=gcc
CFLAGS:=-Wall

.PHONY : all run clean

all : hello

hello : hello.c
	gcc -Wall $< `pkg-config fuse --cflags --libs` -o $@

run : hello
	@mkdir fusedir
	@./hello fusedir/
	cat fusedir/hello

clean :
	rm -f hello
	-fusermount -u fusedir
	-rm -r fusedir

