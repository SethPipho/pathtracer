main: src/main.cpp
	g++ -std=c++11 -Wall -O3 -fopenmp -o main src/main.cpp
clean:
	rm -f ./main

