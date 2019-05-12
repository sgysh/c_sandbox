all:
	gcc -c gval.c
	objdump -D -M intel gval.o
	gcc -static -Wl,--omagic -o gval gval.o

clean:
	rm -f gval gval.o
