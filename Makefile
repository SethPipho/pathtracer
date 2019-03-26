main: src/main.cpp src/canvas.h src/ray.h src/vec.h
	g++ -o main src/main.cpp
clean:
	rm -f ./main