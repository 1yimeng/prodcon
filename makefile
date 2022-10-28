all: prodcon

prodcon: main.o tands.o threads.o
	g++ -o prodcon main.o tands.o threads.o -pthread

main.o: main.cpp
	g++ -c main.cpp -std=c++11

threads.o: threads.cpp
	g++ -c threads.cpp -std=c++11

tands.o: tands.cpp
	g++ -c tands.cpp -std=c++11

clean:
	rm *.o prodcon