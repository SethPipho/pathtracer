#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "vec.h"
#include "ray.h"

class Intersectable {
    public:
        vec3 color;
        virtual double intersect(Ray r) = 0;
        virtual void computeHit(Ray r, double t, vec3* hit, vec3* normal) = 0;
};


class Sphere: public Intersectable {
    public:
        vec3 center;
        double radius;
      
        Sphere();
        Sphere(vec3 c, double r, vec3 col){center = c; radius = r; color = col;}

        double intersect(Ray r);
        void computeHit(Ray r, double t, vec3* hit, vec3* normal);
        
};

double Sphere::intersect(Ray r){
    vec3 oc = r.origin - center;
    double a = dot(r.direction, r.direction);
    double b = 2.0 * dot(oc, r.direction);
    double c = dot(oc, oc) - radius * radius;
    double discriminant = b * b - 4 * a * c;
    if (discriminant <= 0){
        return -1;
    }
    return (-b - sqrt(discriminant)) / (2.0 * a);
}

void Sphere::computeHit(Ray r, double t, vec3* hit, vec3* normal){
    *hit =  r.origin + (r.direction * t);
    *normal = unit(*hit - center);
}




class Scene {
    public:
        std::vector<Intersectable*> objects;

        void addObject(Intersectable* o){
            objects.push_back(o);
        }
};


#endif