#ifndef UTIL_H_
#define UTIL_H_

#include <cstdlib>
#include "vec.h"

//Return double between 0..1
double rand_real(unsigned int *seed){
    return double(rand_r(seed)) / double(RAND_MAX);
}

vec3 rgbToVec(int r, int g, int b){
    return vec3(double(r)/255, double(g)/255, double(b)/255);
}

double clamp(double val, double min, double max){
    if (val > max) {
        return max;
    }
    if (val < min){
        return min;
    }
    return val;
}


double map(double val, double from_min, double from_max, double to_min, double to_max ){
    return to_min + (to_max - to_min) * ((val - from_min) / (from_max - from_min));
}



void displayProgressBar(double progress){
    double width = 20;
    int num_chars = int(width * progress);
    
    std::cout << "  " << int(progress * 100) << "% [";

    for (int i = 0; i < num_chars; i++){
        std::cout << "/";
    }
    for (int i = num_chars; i < width; i++){
        std::cout << "-";
    }
    std::cout << "]\r" ;
    std::cout.flush();
}

#endif