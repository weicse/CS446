CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

sim03 : sim03.o ConfData.o MetaObj.o PCB.o
	$(CC) $(LFLAGS) -std=c++11 ConfData.o MetaObj.o PCB.o sim03.o -o sim03 -pthread

sim03.o : sim03.cpp
	$(CC) $(CFLAGS) -std=c++11 sim03.cpp

ConfData.o : ConfData.h ConfData.cpp
	$(CC) $(CFLAGS) -std=c++11 ConfData.cpp
	
MetaObj.o : MetaObj.h MetaObj.cpp
	$(CC) $(CFLAGS) -std=c++11 MetaObj.cpp

PCB.o : PCB.h PCB.cpp
	$(CC) $(CFLAGS) -std=c++11 PCB.cpp

clean:
	rm *.o sim03
