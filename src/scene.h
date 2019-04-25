#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "vec.h"
#include "ray.h"
#include "util.h"

enum Material {DIFFUSE, MIRROR, EMISSION};

class Intersectable {
    public:
        vec3 color;
        vec3 emmission;
        Material material;
        virtual double intersect(Ray r) = 0;
        virtual vec3 computeHit(vec3 &hit) = 0;
};


class Sphere: public Intersectable {
    public:
        vec3 center;
        double radius;
      
        Sphere();
        Sphere(vec3 c, double r, vec3 col, Material m){center = c; radius = r; color = col; material = m;}

        double intersect(Ray r);
        vec3 computeHit(vec3 &hit);
        
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

vec3 Sphere::computeHit(vec3 &hit){
   return unit(hit - center);
   //return vec3(0,0,0);
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
        vec3 computeHit(vec3 &hit);
};

//Möller–Trumbore intersection algorithm
double Triangle::intersect(Ray r){
    
    const double EPSILON = -0.0000001;

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
    double t = f * dot(edge2, q);  
    if (t > EPSILON) // ray intersection
    {
        
        return t;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return -1;
}

vec3 Triangle::computeHit(vec3& hit){
   return normal;
}




class Scene {
    public:
        std::vector<Intersectable*> objects;

        void addObject(Intersectable* o){
            objects.push_back(o);
        }

        Scene(){}
        Scene(std::string path){
            std::cout << path << std::endl;
            std::ifstream file(path, std::ios::in);

            std::string object;
            
           while(file >> object){ 
                if (object[0] == '#'){continue;}

                if (object == "sphere"){
                
                    double radius;
                    int r,g,b;

                    vec3 pos;

                    file >> pos.x;
                    file >> pos.y;
                    file >> pos.z;
                    file >> radius;
                    file >> r;
                    file >> g;
                    file >> b;

                    addObject(new Sphere(pos, radius, rgbToVec(r,g,b), DIFFUSE));
                }

                if (object == "triangle"){
                    vec3 v0,v1,v2;
                    int r,g,b;

                    file >> v0.x;
                    file >> v0.y;
                    file >> v0.z;

                    file >> v1.x;
                    file >> v1.y;
                    file >> v1.z;

                    file >> v2.x;
                    file >> v2.y;
                    file >> v2.z;

                    file >> r;
                    file >> g;
                    file >> b;

                    addObject(new Triangle(v0,v1,v2, rgbToVec(r,g,b)));
                }
           }

            
        }
};



#endif