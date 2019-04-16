main: src/main.cpp src/scene.h src/vec.h
	g++ -std=c++11 -Wall -O3 -fopenmp -o main src/main.cpp
clean:
	rm -f ./main

