#ifndef TRACE_H
#define TRACE_H

#include "vec.h"
#include "ray.h"
#include "canvas.h"
#include "scene.h"
#include "util.h"


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

    //light radius
    double light_radius = 1;
    vec3 light = vec3(0,0, 14) + uniformRandomSampleUnitSphere(seed) * light_radius; //random jitter light to get soft shadows
    vec3 light_color(1,1,1);
    double light_intensity = 3;
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
    hit = r.origin + (r.direction * toi);

    vec3 normal;
    normal = nearest->computeHit(hit);

    vec3 sample = uniformRandomSampleUnitSphere(seed); 
    vec3 target = hit + normal + sample;
    Ray indirect_ray(hit, target - hit);

    vec3 indirect = trace(indirect_ray, scene, depth + 1, max_depth, seed);

    //explicit light sampling
    vec3 to_light = light - hit;
    vec3 light_dir = unit(to_light);
    double light_dist = to_light.length();
    bool is_shadow = 0;

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
}


#endif
