CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

sim04 : sim04.o ConfData.o MetaObj.o PCB.o
	$(CC) $(LFLAGS) -std=c++11 ConfData.o MetaObj.o PCB.o sim04.o -o sim04 -pthread

sim04.o : sim04.cpp
	$(CC) $(CFLAGS) -std=c++11 sim04.cpp

ConfData.o : ConfData.h ConfData.cpp
	$(CC) $(CFLAGS) -std=c++11 ConfData.cpp
	
MetaObj.o : MetaObj.h MetaObj.cpp
	$(CC) $(CFLAGS) -std=c++11 MetaObj.cpp

PCB.o : PCB.h PCB.cpp
	$(CC) $(CFLAGS) -std=c++11 PCB.cpp

clean:
	rm *.o sim04
