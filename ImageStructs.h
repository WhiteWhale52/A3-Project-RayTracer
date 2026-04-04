#ifndef IMAGESTRUCTS_H
#define IMAGESTRUCTS_H

#include "MathUtils.h"

#define MAX_SPHERES 32

typedef struct {
    unsigned char r,g,b;
} Pixel;


typedef struct {
    Vector3 origin;
    Vector3 direction;    
} Ray;


typedef struct {
    Vector3 color;      
    Vector3 emissionColor;      
    float  emissionPower;
    float roughness;
} Material;

static inline Vector3 GetEmission(const Material *mat) {
    return Scale(mat->emissionColor, mat->emissionPower);
}


typedef struct {
    Vector3     origin;
    float      radius;
    Material mat;
} Sphere;


 // TODO: set these in build_scene() inside main.c.
typedef struct {
    Vector3 origin;
    Vector3 lower_left;  
    Vector3 horizontal;  
    Vector3 vertical;    
} Camera;


typedef struct {
    Camera camera;
    Sphere spheres[MAX_SPHERES];
    int      num_spheres;
    int      image_width;
    int      image_height;
    int      samples_per_pixel; 
    int      max_depth;         
} Scene;


#endif