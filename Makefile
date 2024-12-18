all: do

do: main.o nodeInfo.o helperClass.o init.o port.o functions.o 
	g++ main.o init.o functions.o port.o nodeInfo.o helperClass.o -o prog -lcrypto -lpthread
	
main.o: main.cpp
		g++ -std=c++11 -c main.cpp
		
init.o: init.cpp
		g++ -std=c++11 -c init.cpp

port.o: port.cpp
		g++ -std=c++11 -c port.cpp
		
functions.o: functions.cpp
				 g++ -std=c++11 -c functions.cpp
			 
nodeInfo.o: nodeInfo.cpp
				   g++ -std=c++11 -c nodeInfo.cpp
				   
helperClass.o: helperClass.cpp
			   g++ -std=c++11 -c helperClass.cpp			


	
