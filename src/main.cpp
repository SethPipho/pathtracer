#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include "vec.h"
#include "ray.h"
#include "canvas.h"
#include "scene.h"


vec3 trace(Ray r, Scene &scene);
double rand_real();
double map(double val, double from_min, double from_max, double to_min, double to_max );
double clamp(double val, double min, double max);
vec3 rgbToVec(int r, int g, int b);

int main(){
    Canvas canvas(512, 512);

    //projection plane
    double v_width = 2;
    double v_height = 2;
    double v_depth = 4;

    double wall_r = 100000; //radius for wall spheres
   
    Scene scene;
    scene.addObject(new Sphere(vec3(-.5, -2,10), 1, rgbToVec(231, 76, 60))); //red
    scene.addObject(new Sphere(vec3(.5, -1, 9.5), 1, rgbToVec(200, 200, 200))); //white
    scene.addObject(new Sphere(vec3(-.5, -1,10.5), 1, rgbToVec(46, 204, 113))); //green


    scene.addObject(new Sphere(vec3(0, wall_r + 4, 0), wall_r, rgbToVec(200, 200, 200))); //top wall;
    scene.addObject(new Sphere(vec3(0, -wall_r - 4, 0), wall_r, rgbToVec(200, 200, 200))); //bottom
    scene.addObject(new Sphere(vec3(wall_r + 4, 0, 0), wall_r, rgbToVec(52, 152, 219))); //right Wall
    scene.addObject(new Sphere(vec3(-wall_r - 4, 0, 0), wall_r, rgbToVec(231, 76, 60))); //left wall
    scene.addObject(new Sphere(vec3(0, 0, wall_r + 20), wall_r, rgbToVec(200, 200, 200))); //far wall

    int samples = 16;


    for (int y = 0; y < canvas.height; y++){
        for (int x = 0; x < canvas.width; x++){
            vec3 color(0,0,0);
            for (int i = 0; i < samples; i++){

                //compute location on projection plane
                double u = v_width * (double(x) + rand_real())/double(canvas.width) - (v_width/2);
                double v = v_height * (double(y) + rand_real())/double(canvas.height) - (v_height/2);

                //camera raay
                Ray r(vec3(0,0,-10), vec3(u, -v,v_depth));

                color += trace(r, scene);
            }


            color /= samples;

            int r = int (sqrt(clamp(color.x, 0, 1)) * 255);
            int g = int (sqrt(clamp(color.y, 0, 1)) * 255);
            int b = int (sqrt(clamp(color.z, 0, 1)) * 255);

            canvas.setPixel(x,y,r,g, b);
        }
    }


    
    std::cout << "Saving image..." << std::endl;
    canvas.savePPM("render/test-scene.ppm");
   
    return 0;
}

vec3 trace(Ray r, Scene &scene){

    vec3 light(2,2, 5);
    vec3 light_color(1,1,1);
    double light_intensity = 4;
    vec3 ambient_light(.2,.2,.2);

    double t;
    double closest_dist = 100000;
    bool ray_hit = 0;
    Sphere *closest;

    for (int j = 0; j < scene.objects.size(); j++){
        t =  scene.objects[j]->intersect(r);
        if (t > 0 && closest_dist > t){
            closest_dist = t;
            closest = (Sphere*) scene.objects[j];
            ray_hit = 1;
        }
    }

    vec3 hit;
    vec3 normal;
    vec3 color;

    if (ray_hit) {
         
        closest->computeHit(r, closest_dist, &hit, &normal);

        vec3 L = light - hit;
        vec3 light_dir = unit(L);
        double light_dist = L.length();

        Ray shadow_ray(hit, light_dir);

        for (int j = 0; j < scene.objects.size(); j++){
            t =  scene.objects[j]->intersect(shadow_ray);
            if (t > -.001 && t < light_dist){
               return closest->color * ambient_light;
            }
        }

    
        color = closest->color * (light_color * light_intensity * dot(normal, light_dir)/light_dist + ambient_light);
        //sum +=  vec3(normal.x + 1,normal.y + 1,normal.z + 1) * .5;
        return  color;
    } else {
        return vec3(.9,.9,.9);
    }
}

//Return double between 0..1
double rand_real(){
    return double(rand()) / double(RAND_MAX);
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