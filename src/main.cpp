#include <iostream>
#include <fstream>
#include <cstdlib>
#include <omp.h>
#include <math.h>
#include <chrono>
#include "vec.h"
#include "ray.h"
#include "canvas.h"
#include "scene.h"

#define GAMMA 2.2


vec3 trace(Ray &r, Scene &scene, int depth, int max_depth, unsigned int *seed);
double rand_real(unsigned int *seed);
double map(double val, double from_min, double from_max, double to_min, double to_max );
double clamp(double val, double min, double max);
vec3 rgbToVec(int r, int g, int b);

int main(){
    Canvas canvas(512, 512);
    int samples = 100;
    int num_threads = 8;

    //projection plane
    double v_width = 2;
    double v_height = 2;
    double v_depth = 4;

    double wall_r = 100000; //radius for wall spheres

   
    Scene scene;
    scene.addObject(new Sphere(vec3(1.5, -3,10), 1, rgbToVec(230, 126, 34), DIFFUSE)); //orange
    scene.addObject(new Sphere(vec3(-1, -2.5, 11), 1.5, rgbToVec(200, 200, 200), DIFFUSE)); //white
    //scene.addObject(new Sphere(vec3(-.5, -1, 11), 1, rgbToVec(46, 204, 113), DIFFUSE)); //green

    //scene.addObject(new Sphere(vec3(0, 4, 10), 2, rgbToVec(255, 255, 255), EMISSION)); //light
   // scene.objects[3]->emmission = vec3(3,3,3);

    scene.addObject(new Sphere(vec3(0, wall_r + 4, 0), wall_r, rgbToVec(200, 200, 200), DIFFUSE)); //top wall/light;
    
   
    scene.addObject(new Sphere(vec3(0, -wall_r - 4, 0), wall_r, rgbToVec(200, 200, 200), DIFFUSE)); //bottom
    scene.addObject(new Sphere(vec3(wall_r + 4, 0, 0), wall_r, rgbToVec(52, 152, 219), DIFFUSE)); //right Wall
    scene.addObject(new Sphere(vec3(-wall_r - 4, 0, 0), wall_r, rgbToVec(231, 76, 60), DIFFUSE)); //left wall
    scene.addObject(new Sphere(vec3(0, 0, wall_r + 20), wall_r, rgbToVec(200, 200, 200), DIFFUSE)); //far wall
    scene.addObject(new Sphere(vec3(0, 0, -wall_r - 20), wall_r, rgbToVec(200, 200, 200), DIFFUSE)); //back wall


    //array of vectors to hold raw colors
    vec3* pixels = new vec3[canvas.width * canvas.height];

    auto start = std::chrono::high_resolution_clock::now();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  

    #pragma omp parallel for num_threads(num_threads) 
    for (int y = 0; y < canvas.height; y++){

        unsigned int seed = y;
        
        int rank = omp_get_thread_num();
        for (int x = 0; x < canvas.width; x++){
            
            vec3 color(0,0,0);
            for (int i = 0; i < samples; i++){

                //compute location on projection plane
                double u = v_width * (double(x) + rand_real(&seed))/double(canvas.width) - (v_width/2);
                double v = v_height * (double(y) + rand_real(&seed))/double(canvas.height) - (v_height/2);

                //camera raay
                Ray r(vec3(0,0,-10), vec3(u, -v,v_depth));
                color += trace(r, scene, 0, 5, &seed);
            }
            int index = y * canvas.width + x;
            pixels[index] = color/samples;
        }
    }


     ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

    //Tonemapping and vec to color conversion
    for (int y = 0; y < canvas.height; y++){
        for (int x = 0; x < canvas.width; x++){

            int index = y * canvas.width + x;
            vec3 color = pixels[index];

            int r = int (pow(clamp(color.x, 0, 1), 1/GAMMA) * 255);
            int g = int (pow(clamp(color.y, 0, 1), 1/GAMMA) * 255);
            int b = int (pow(clamp(color.z, 0, 1), 1/GAMMA) * 255);

            canvas.setPixel(x,y,r,g, b);
        }
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    auto finish = std::chrono::high_resolution_clock::now();
    auto elapsed =  std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

    std::cout << "Render Time: " << double(elapsed.count()) / 1000 << " seconds" << std::endl;
    
    std::cout << "Saving image..." << std::endl;
    canvas.savePPM("render/test-scene.ppm");
   
    return 0;
}


bool nearestIntersection(Scene &scene, Ray &r, Intersectable **nearest, double *toi){
    double t;
    double nearest_toi = 100000;
    bool ray_hit = 0;
    Intersectable *current_nearest;

    for (int j = 0; j < int(scene.objects.size()); j++){
        t = scene.objects[j]->intersect(r);
        if (t > 0 && nearest_toi > t){
            nearest_toi = t;
            current_nearest = scene.objects[j];
            ray_hit = 1;
        }
    }

    if (ray_hit){
        *toi = nearest_toi;
        *nearest = current_nearest;
        return 1;
    }
    return 0;
}

vec3 uniformRandomSampleUnitSphere(unsigned int *seed){
   
    
    double x,y,z;
    do {
        x = rand_real(seed) * 2 - 1;
        y = rand_real(seed) * 2 - 1;
        z = rand_real(seed) * 2 - 1;

    } while(x * x + y * y + z * z >= 1.0);



    return vec3(x,y,z);
}

vec3 trace(Ray &r, Scene &scene, int depth, int max_depth, unsigned int *seed){


    if (depth == max_depth){
        return vec3(0,0,0);
    }

    //random jitter light to get soft shadows
    //light radius
    double light_radius = 1;
    vec3 light = vec3(0,2, 10) + uniformRandomSampleUnitSphere(seed) * light_radius;
    vec3 light_color(1,1,1);
    double light_intensity = 3;
    vec3 ambient_light(.2,.2,.2);
    vec3 world_color(.8,.8,.8);


    Intersectable *nearest = nullptr;
    double toi = 0; //time of impact
    bool ray_hit = nearestIntersection(scene, r, &nearest, &toi);
    
    if (!ray_hit){
        return world_color;
    }

    if (nearest->material == EMISSION){
        return nearest->emmission;
    }
    
    vec3 hit;
    vec3 normal;
    nearest->computeHit(r, toi, &hit, &normal);

    vec3 to_camera = unit(r.direction * -1);

    vec3 sample = uniformRandomSampleUnitSphere(seed); 
    vec3 target = hit + normal + sample;
    Ray indirect_ray(hit, target - hit);

    vec3 indirect = trace(indirect_ray, scene, depth + 1, max_depth, seed);

    //explicit light sampling

    vec3 to_light = light - hit;
    vec3 light_dir = unit(to_light);
    double light_dist = to_light.length();
    bool is_shadow = 0;

    //compute shadow
    Ray shadow_ray(hit, light_dir);
    for (int j = 0; j < int(scene.objects.size()); j++){
        toi = scene.objects[j]->intersect(shadow_ray);
        if (toi > -.001 && toi < light_dist){
            is_shadow = 1;
            break;
        }
    }

    vec3 direct; //direct lighting 
    if (is_shadow){
        direct = vec3(0,0,0);
    } else {
        direct = (light_color * light_intensity * dot(normal, light_dir)/light_dist);
    }
    
     return nearest->color * (indirect + direct);

    

    /*
    //reflection ray
    //vec3 to_camera = unit(r.direction) * -1;
   // vec3 reflection =  normal * (2 * dot(normal, to_camera)) - to_camera; 
    //Ray reflection_ray(hit, reflection);

    //diffuse lighting
    //

    // return vec3(reflection.x + 1,reflection.y + 1, reflection.z + 1) * .5;
    //return vec3(normal.x + 1,normal.y + 1,normal.z + 1) * .5;

    if (depth == max_depth || nearest->material == DIFFUSE){
            return nearest->color * (diffuse + ambient_light) + nearest->emmission;
    }
    return nearest->color * trace(reflection_ray, scene, depth + 1, max_depth);

    */
       
    
}

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