all : g512sum g256sum

g512sum : g512sum.o transforms.o
	g++  -Wall -Ofast g512sum.o transforms.o -o g512sum


g256sum : g256sum.o transforms.o
	g++  -Wall -O2 g256sum.o transforms.o -o g256sum

g512sum.o : g512sum.cpp transforms.h
	g++ -c -Wall -Ofast g512sum.cpp

g256sum.o : g256sum.cpp transforms.h
	g++ -c -Wall -O2 g256sum.cpp

transforms.o : transforms.cpp transforms.h
	g++ -c -Wall -Ofast  transforms.cpp

clean : 
	rm *.o 
