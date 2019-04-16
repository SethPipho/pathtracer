#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "vec.h"
#include "ray.h"

enum Material {DIFFUSE, MIRROR, EMISSION};

class Intersectable {
    public:
        vec3 color;
        vec3 emmission;
        Material material;
        virtual double intersect(Ray r) = 0;
        virtual void computeHit(Ray r, double t, vec3* hit, vec3* normal) = 0;
};


class Sphere: public Intersectable {
    public:
        vec3 center;
        double radius;
      
        Sphere();
        Sphere(vec3 c, double r, vec3 col, Material m){center = c; radius = r; color = col; material = m;}

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


class Triangle: public Intersectable {
    public:
        vec3 v0;
        vec3 v1;
        vec3 v2;

        vec3 normal;

        Triangle();
        Triangle(vec3 _v0, vec3 _v1, vec3 _v2, vec3 _color){v0 = _v0; v1 = _v1; v2 = _v2; color = _color; normal = -1 * unit(cross(v1 - v0, v2 - v0));} 

        double intersect(Ray r);
        void computeHit(Ray r, double t, vec3* hit, vec3* normal);
};

//Möller–Trumbore intersection algorithm
double Triangle::intersect(Ray r){
    
    const float EPSILON = -0.0000001;

    vec3 edge1, edge2, h, s, q;
    double a,f,u,v;

    edge1 = v1 - v0;
    edge2 = v2 - v0;
    h = cross(r.direction, edge2);
    a = dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return false;    // This ray is parallel to this triangle.
    f = 1.0/a;
    s = r.origin - v0;
    u = f * dot(s,h);
    if (u < 0.0 || u > 1.0)
        return -1;
    q = cross(s, edge1);
    v = f * dot(r.direction, q); 
    if (v < 0.0 || u + v > 1.0)
        return -1;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot(edge2, q);  
    if (t > EPSILON) // ray intersection
    {
        
        return t;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return -1;
}

void Triangle::computeHit(Ray r, double t, vec3* hit, vec3* _normal){
    *hit =  r.origin + (r.direction * t);
    if (dot(r.direction, normal) < 0){
        *_normal = normal;
    } else {
        *_normal = -1 * normal;
    }
    
  
}



class Scene {
    public:
        std::vector<Intersectable*> objects;

        void addObject(Intersectable* o){
            objects.push_back(o);
        }
};


#endif