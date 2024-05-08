all : winsuport.o tennis0 tennis1 tennis2

winsuport.o : winsuport.c winsuport.h
	gcc -Wall -c winsuport.c -o winsuport.o

winsuport2.o : winsuport2.c winsuport2.h
	gcc -Wall -c winsuport2.c -o winsuport2.o

memoria.o: memoria.c memoria.h
	gcc -Wall -c memoria.c -o memoria.o

tennis0 : tennis0.c winsuport.o winsuport.h
	gcc -Wall tennis0.c winsuport.o -o tennis0 -lcurses

tennis1 : tennis1.c winsuport.o winsuport.h
	gcc -Wall tennis1.c winsuport.o -o tennis1 -lcurses -lpthreadS

tennis2 : tennis2.c winsuport.o winsuport.h
	gcc -Wall tennis2.c winsuport.o -o tennis2 -lcurses -lpthread

tennis3: tennis3.c winsuport2.o winsuport2.h pal_ord3.c memoria.o
	gcc -Wall tennis3.c winsuport2.o memoria.o -o tennis3 -lcurses -lpthread

pal_ord3: pal_ord3.c memoria.o winsuport2.o
	gcc -Wall pal_ord3.c memoria.o winsuport2.o -lcurses -o pal_ord3

clean: 
	rm winsuport.o tennis0 tennis1 tennis2 tennis3 pal_ord3 memoria.o winsuport2.o
