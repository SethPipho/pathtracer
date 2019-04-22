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
#include "util.h"
#include "trace.h"

#define GAMMA 2.2



int main(){
    Canvas canvas(512, 512);
    int samples = 100;
    int num_threads = 4;

    double wall_r = 100000; //radius for wall spheres

    Scene scene;
    scene.addObject(new Sphere(vec3(1.5, -3, 10), 1, rgbToVec(230, 126, 34), DIFFUSE)); //orange
    scene.addObject(new Sphere(vec3(-1, -2.5, 11), 1.5, rgbToVec(200, 200, 200), DIFFUSE)); //white

    scene.addObject(new Sphere(vec3(0, wall_r + 4, 0), wall_r, rgbToVec(200, 200, 200), DIFFUSE)); //top wall
    scene.addObject(new Sphere(vec3(0, -wall_r - 4, 0), wall_r, rgbToVec(200, 200, 200), DIFFUSE)); //bottom
    scene.addObject(new Sphere(vec3(wall_r + 4, 0, 0), wall_r, rgbToVec(52, 152, 219), DIFFUSE)); //right Wall
    scene.addObject(new Sphere(vec3(-wall_r - 4, 0, 0), wall_r, rgbToVec(231, 76, 60), DIFFUSE)); //left wall
    scene.addObject(new Sphere(vec3(0, 0, wall_r + 20), wall_r, rgbToVec(200, 200, 200), DIFFUSE)); //far wall
    scene.addObject(new Sphere(vec3(0, 0, -wall_r - 20), wall_r, rgbToVec(200, 200, 200), DIFFUSE)); //back wall

    scene.addObject(new Triangle(vec3(-3,-1.5, 11), vec3(-1,-1.5, 9), vec3(-1,.5,10), rgbToVec(106, 176, 76)));
    scene.addObject(new Triangle(vec3(-1,-1.5, 9), vec3(1,-1.5, 11),  vec3(-1,.5,10), rgbToVec(106, 176, 76)));
    scene.addObject(new Triangle(vec3(1,-1.5, 11), vec3(-1,-1.5, 9), vec3(-1,.5,10),rgbToVec(106, 176, 76)));
   
  
    auto start = std::chrono::high_resolution_clock::now();
   
    //array of vectors to hold raw colors
    vec3* pixels = new vec3[canvas.width * canvas.height];

    #pragma omp parallel for schedule(static,1) num_threads(num_threads) 
    for (int y = 0; y < canvas.height; y++){
        unsigned int seed = y;
        for (int x = 0; x < canvas.width; x++){
            vec3 color(0,0,0);
            for (int i = 0; i < samples; i++){
                 //projection plane
                double v_width = 2;
                double v_height = 2;
                double v_depth = 4;

                //compute location on projection plane
                double u = v_width * (double(x) + rand_real(&seed))/double(canvas.width) - (v_width/2);
                double v = v_height * (double(y) + rand_real(&seed))/double(canvas.height) - (v_height/2);
                
                //camera ray
                Ray r(vec3(0,0,-10), vec3(u, -v,v_depth));
                color += trace(r, scene, 0, 4, &seed);
            }
            int index = y * canvas.width + x;
            pixels[index] = color/samples;

            if (y % 24 == 0){
                displayProgressBar(double(y)/double(canvas.width));
            }
        }
    }
    displayProgressBar(1.0);
    std::cout << std::endl;
    auto finish = std::chrono::high_resolution_clock::now();
    auto elapsed =  std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

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


    std::cout << "Threads: " << num_threads << std::endl;
    std::cout << "Render Time: " << double(elapsed.count()) / 1000 << " seconds" << std::endl;
    
    std::cout << "Saving image..." << std::endl;
    canvas.savePPM("render/test-scene.ppm");

    return 0;
}


