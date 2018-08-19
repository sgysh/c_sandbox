.PHONY : all clean

all : binary binary2 binary3

binary : binary.dat
	gcc -c binary.c -Wall
	objcopy --add-section=bindata=binary.dat --set-section-flags=bindata=contents,alloc,load,readonly,data binary.o binary_tmp.o
	gcc binary_tmp.o -Wall -o binary

binary2 : binary.dat binary.o
	gcc -c incbin.s
	gcc binary.o incbin.o -o binary2

binary3 : binary.dat binary.o
	ld --format binary --oformat elf64-x86-64 -r binary.dat -o binary_tmp2.o
	ld -r -T bindata.scr binary_tmp2.o -o binary_tmp3.o
	gcc binary.o binary_tmp3.o -o binary3

binary.dat :
	echo "This is test." > binary.dat

clean :
	rm -f *.o
	rm -f binary binary2 binary3 binary.dat

