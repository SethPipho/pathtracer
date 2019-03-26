#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include "vec.h"
#include "ray.h"
#include "canvas.h"


double rand_real(){
    return double(rand()) / double(RAND_MAX);
}

class Sphere {
    public:
        vec3 center;
        double radius;
        vec3 color;
        Sphere();
        Sphere(vec3 c, double r, vec3 col){center = c; radius = r; color = col;}
};

double raySphereIntersection(Ray r, Sphere s){
    vec3 oc = r.origin - s.center;
    double a = dot(r.direction, r.direction);
    double b = 2.0 * dot(oc, r.direction);
    double c = dot(oc, oc) - s.radius * s.radius;
    double discriminant = b * b - 4 * a * c;
    if (discriminant <= 0){
        return -1;
    }
    return (-b - sqrt(discriminant)) / (2.0 * a);
}

double map(double val, double from_min, double from_max, double to_min, double to_max ){
    return to_min + (to_max - to_min) * ((val - from_min) / (from_max - from_min));
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


vec3 rgbToVec(int r, int g, int b){
    return vec3(double(r)/255, double(g)/255, double(b)/255);
}

int main(){
    Canvas canvas(512, 512);

    double v_width = 4;
    double v_height = 4;
   
    int num_spheres = 2;
    Sphere *spheres[2];
    spheres[0] = new Sphere(vec3(-1,-1,-5), 2, rgbToVec(231, 76, 60));
    spheres[1] = new Sphere(vec3(1,1,-5), 2, rgbToVec(52, 152, 219));



    int samples = 16;

    vec3 light(3,-3, 0);
    vec3 light_color(1,1,1);
    double light_intensity = 6;
    vec3 ambient_light(.5,.5,.5);


    for (int y = 0; y < canvas.height; y++){
        for (int x = 0; x < canvas.width; x++){
            vec3 sum(0,0,0);
            for (int i = 0; i < samples; i++){

                
                double u = v_width * (double(x) + rand_real())/double(canvas.width) - (v_width/2);
                double v = v_height * (double(y) + rand_real())/double(canvas.height) - (v_height/2);

                Ray r(vec3(0,0,0), vec3(u,v,-1));
                double t;
                double closest_dist = 100000;
                bool ray_hit = 0;
                Sphere *closest;

                for (int j = 0; j < num_spheres; j++){
                    t = raySphereIntersection(r, *spheres[j]);
                    if (t > 0 && closest_dist > t){
                        closest_dist = t;
                        closest = spheres[j];
                        ray_hit = 1;
                    }
                }
                
                vec3 hit =  r.origin + (r.direction * closest_dist);
                if (ray_hit) {
                    vec3 normal = unit(hit - closest->center);
                    vec3 L = light - hit;
                    vec3 light_dir = unit(L);
                    double light_dist = L.length();
                    vec3 color = closest->color * (light_color * light_intensity * dot(normal, light_dir)/light_dist + ambient_light);
                    //sum +=  vec3(normal.x + 1,normal.y + 1,normal.z + 1) * .5;
                    sum += color;
                } else {
                    sum += vec3(.9,.9,.9);
                }

            }

            int r = int (clamp(sum.x/samples, 0, 1) * 255);
            int g = int (clamp(sum.y/samples, 0, 1) * 255);
            int b = int (clamp(sum.z/samples, 0, 1) * 255);

            canvas.setPixel(x,y,r,g, b);

            
        }
    }


    canvas.savePPM("render/test.ppm");
   
    return 0;
}