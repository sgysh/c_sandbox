.PHONY : all clean

all : combine

combine :
	gcc -c app1.c -Wall
	gcc -c app2.c -Wall
	objcopy --redefine-sym main=app1_main --redefine-sym exit=combine_exit app1.o combine_app1.o
	objcopy --redefine-sym main=app2_main --redefine-sym exit=combine_exit app2.o combine_app2.o
	gcc combine.c combine_app1.o combine_app2.o -Wall -o combine
	ln -s combine app1
	ln -s combine app2

clean :
	rm -f *.o
	rm -f combine app1 app2

