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


vec3 trace(Ray r, Scene &scene);
double rand_real(unsigned int *seed);
double map(double val, double from_min, double from_max, double to_min, double to_max );
double clamp(double val, double min, double max);
vec3 rgbToVec(int r, int g, int b);

int main(){
    MPI_Init(NULL, NULL);
    int i;
    double my_num;
    int my_rank, comm_sz;
    MPI_Comm comm;
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &comm_sz);
    MPI_Comm_rank(comm, &my_rank);

    Canvas canvas(512, 512);
    int samples = 16;
    int num_threads = 4;

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



    if(my_rank == 0){

      double* pixel_row = (double *) malloc(3 * sizeof(double) * canvas.width);
      int row_start; 
      MPI_Status status; 
      //array of vectors to hold raw colors
      vec3* pixels = new vec3[canvas.width * canvas.height];
    
      auto start = std::chrono::high_resolution_clock::now();
    
      

      for(int k = 0; k < canvas.height; k++){
	
        MPI_Recv(pixel_row, 3 * canvas.width, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
	row_start = status.MPI_TAG;
        for(int l = row_start * canvas.width; l < canvas.width; l++){
          pixels[l] = new vec3(*pixel_row(l * 3), *pixel_row(l * 3 + 1), *pixel_row(l * 3 + 2));
	}
      }

      #pragma omp parallel for num_threads(num_threads)
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

      auto finish = std::chrono::high_resolution_clock::now();
      auto elapsed =  std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

      std::cout << "Render Time: " << double(elapsed.count()) / 1000 << " seconds" << std::endl;

      std::cout << "Saving image..." << std::endl;
      canvas.savePPM("render/test-scene.ppm");
    }
 
    else{   
      for (int y = my_rank - 1; y < canvas.height; y = y + comm_sz - 1){
          //each thread needs seed for call to rand_r() because rand() is locking
	  double color_array[canvas.width * 3];
          unsigned int seed = y;
          #pragma omp parallel for num_threads(num_threads)
          for (int x = 0; x < canvas.width; x++){
              vec3 color(0,0,0);

              for (int i = 0; i < samples; i++){

                  //compute location on projection plane
                  double u = v_width * (double(x) + rand_real(&seed))/double(canvas.width) - (v_width/2);
                  double v = v_height * (double(y) + rand_real(&seed))/double(canvas.height) - (v_height/2);

                  //camera ray
                  Ray r(vec3(0,0,-10), vec3(u, -v,v_depth));
                  color += trace(r, scene);
              }
              //int index = y * canvas.width + x;
              color = color/samples;
	      color_array[x * 3] = color.x;
	      color_array[x * 3 + 1] = color.y;
	      color_array[x * 3 + 2] = color.z;
          }
	  MPI_Send(&color_array[0], canvas.width * 3, MPI_DOUBLE, 0, y, comm);
        }
    }
    MPI_Barrier(comm);
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
    Sphere *closest = nullptr;

    for (int j = 0; j < int(scene.objects.size()); j++){
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

        //compute shadow
        Ray shadow_ray(hit, light_dir);

        for (int j = 0; j < int(scene.objects.size()); j++){
            t =  scene.objects[j]->intersect(shadow_ray);
            if (t > -.001 && t < light_dist){
               return closest->color * ambient_light;
            }
        }

        //diffuse lighting
        color = closest->color * (light_color * light_intensity * dot(normal, light_dir)/light_dist + ambient_light);

        //sum +=  vec3(normal.x + 1,normal.y + 1,normal.z + 1) * .5;
        return color;
    } else {
        return vec3(.9,.9,.9);
    }
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
