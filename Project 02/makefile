CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

sim02 : sim02.o ConfData.o MetaObj.o PCB.o
	$(CC) $(LFLAGS) -std=c++11 ConfData.o MetaObj.o PCB.o sim02.o -o sim02 -pthread

sim02.o : sim02.cpp
	$(CC) $(CFLAGS) -std=c++11 sim02.cpp

ConfData.o : ConfData.h ConfData.cpp
	$(CC) $(CFLAGS) -std=c++11 ConfData.cpp
	
MetaObj.o : MetaObj.h MetaObj.cpp
	$(CC) $(CFLAGS) -std=c++11 MetaObj.cpp

PCB.o : PCB.h PCB.cpp
	$(CC) $(CFLAGS) -std=c++11 PCB.cpp

clean:
	rm *.o sim02
