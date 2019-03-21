CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

sim01 : sim01.o ConfData.o MetaObj.o
	$(CC) $(LFLAGS) -std=c++11 ConfData.o MetaObj.o sim01.o -o sim01

sim01.o : sim01.cpp
	$(CC) $(CFLAGS) -std=c++11 sim01.cpp

ConfData.o : ConfData.h ConfData.cpp
	$(CC) $(CFLAGS) -std=c++11 ConfData.cpp
	
MetaObj.o : MetaObj.h MetaObj.cpp
	$(CC) $(CFLAGS) -std=c++11 MetaObj.cpp

clean:
	rm *.o sim01 *.lgf
