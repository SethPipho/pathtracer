main: src/main.cpp src/scene.h src/vec.h src/trace.h
	g++ -std=c++11 -Wall -O3 -fopenmp -o main src/main.cpp

clean:
	rm -f ./main
	rm -f ./main-debug
	rm -f ./main-profile
	rm -f ./gmon.out

debug: src/main.cpp src/scene.h src/vec.h
	g++ -pg -g -std=c++11 -Wall -O0 -fopenmp -o main-debug src/main.cpp

