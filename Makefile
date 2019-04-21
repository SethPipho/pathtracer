main: src/main.cpp src/scene.h src/vec.h
	g++  -std=c++11 -Wall -O3 -ffast-math -fopenmp -o main src/main.cpp

clean:
	rm -f ./main
	rm -f ./main-debug
	rm -f ./main-profile
	rm -f ./gmon.out

debug: src/main.cpp src/scene.h src/vec.h
	g++ -g -std=c++11 -Wall -O0 -fopenmp -o main-debug src/main.cpp

profile: src/main.cpp src/scene.h src/vec.h
	g++ -pg -g -std=c++11 -Wall -fopenmp -o main-profile src/main.cpp

