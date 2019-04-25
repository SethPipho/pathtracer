#include <iostream>
#include <fstream>
#include <cstdlib>
#include <omp.h>
#include <mpi.h>
#include <math.h>
#include <chrono>
#include "vec.h"
#include "ray.h"
#include "canvas.h"
#include "scene.h"
#include "util.h"
#include "trace.h"
#include "CLI11.hpp"

#define GAMMA 2.2

int main(int argc, char** argv){
    MPI_Init(NULL, NULL);
    int my_rank, comm_sz;
    MPI_Comm comm;
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &comm_sz);
    MPI_Comm_rank(comm, &my_rank);

    std::string filename = "render/mpi-v2-test.ppm";
    int samples = 100;
    int img_width = 512;
    int num_threads = 1;

    CLI::App app{"App description"};
    app.add_option("-o,--output", filename, "Path of output file (ppm image)");
    app.add_option("-s,--samples", samples, "Number of samples per pixel");
    app.add_option("-w,--width", img_width, "Width of image in pixels");
    app.add_option("-t,--threads", num_threads, "Number of threads");
    CLI11_PARSE(app, argc, argv);


    if (my_rank == 0){
        std::cout << "Rendering to: '" << filename << "'" << std::endl;
        std::cout << img_width << " x " << img_width << std::endl;
        std::cout << "Samples: " << samples << std::endl;
        std::cout << "Threads: " << num_threads << std::endl;
    }

    Canvas canvas(img_width, img_width);
    
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

    scene.addObject(new Triangle(vec3(-3,-1.5, 11), vec3(-1,-1.5, 9), vec3(-1,.5,10), rgbToVec(150,150,150)));
    scene.addObject(new Triangle(vec3(-1,-1.5, 9), vec3(1,-1.5, 11),  vec3(-1,.5,10), rgbToVec(150,150,150)));
    scene.addObject(new Triangle(vec3(1,-1.5, 11), vec3(-1,-1.5, 9), vec3(-1,.5,10),rgbToVec(150,150,150)));
   
    vec3* pixels = new vec3[canvas.width * canvas.height];

    auto start = std::chrono::high_resolution_clock::now();
    
    for (int y = my_rank; y < canvas.height; y = y + comm_sz){
        //each thread needs seed for call to rand_r() because rand() is locking
        unsigned int seed = y;
      //  #pragma omp parallel for schedule(static,1) 
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
                    color += trace(r, scene, 0, 3, &seed);
                }
                //int index = y * canvas.width + x;
                color = color/samples;
                int index = y * canvas.width + x;
                pixels[index] = color;
            }
        }

    //array of doubles for sending over MPI
    double *row = new double[canvas.width * 3];

    //convert rows to doubles, send rows to process zero
    if (my_rank != 0){
        
          for (int y = my_rank; y < canvas.height; y = y + comm_sz){
              int row_start = y * canvas.width;
              for (int i = 0; i < canvas.width; i++){
                  row[i * 3] = pixels[row_start + i].x;
                  row[i * 3 + 1] = pixels[row_start + i].y;
                  row[i * 3 + 2] = pixels[row_start + i].z;
              }

              MPI_Send(row, canvas.width * 3, MPI_DOUBLE, 0, y, comm);
          }
    } else {

       MPI_Status status; 

        for (int y = 0; y < canvas.height; y++){
            if (y % comm_sz == 0) {continue;}

            MPI_Recv(row, 3 * canvas.width, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
            int row_num = status.MPI_TAG;

             for(int i = 0; i <  canvas.width; i++){
                pixels[row_num * canvas.width + i] = vec3( row[i * 3], row[i * 3 + 1], row[i * 3 + 2]);
            }
        }
    }

      

    if (my_rank == 0){
        //tonemapping and file saving
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
        canvas.savePPM(filename);
    }
    MPI_Barrier(comm);
    return 0;
}


