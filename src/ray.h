
#ifndef RAY_H
#define RAY_H


#include "vec.h"

class Ray {
    public:
        vec3 origin;
        vec3 direction;

        Ray(vec3 o, vec3 dir) {origin = o; direction = dir;}
};

#endif