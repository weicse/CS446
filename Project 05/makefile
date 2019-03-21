CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

sim05 : sim05.o ConfData.o MetaObj.o PCB.o
	$(CC) $(LFLAGS) -std=c++11 ConfData.o MetaObj.o PCB.o sim05.o -o sim05 -pthread

sim05.o : sim05.cpp
	$(CC) $(CFLAGS) -std=c++11 sim05.cpp

ConfData.o : ConfData.h ConfData.cpp
	$(CC) $(CFLAGS) -std=c++11 ConfData.cpp
	
MetaObj.o : MetaObj.h MetaObj.cpp
	$(CC) $(CFLAGS) -std=c++11 MetaObj.cpp

PCB.o : PCB.h PCB.cpp
	$(CC) $(CFLAGS) -std=c++11 PCB.cpp

clean:
	rm *.o sim05
