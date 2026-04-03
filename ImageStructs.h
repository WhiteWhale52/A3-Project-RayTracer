#ifndef IMAGESTRUCTS_H
#define IMAGESTRUCTS_H

#include "MathUtils.h"

typedef struct Pixel {
    int r,g,b;
};



/* ----------------------------------------------------------
 * ray_t — origin + unit direction
 * ---------------------------------------------------------- */
typedef struct {
    Vector3 origin;
    Vector3 dir;     /* should be normalised before use */
} ray_t;

/* ----------------------------------------------------------
 * material_t
 *
 * TODO: expand this however you like — add reflectance,
 * emission, roughness, etc.
 * ---------------------------------------------------------- */
typedef struct {
    Vector3 color;       /* base diffuse colour [0,1]^3 */
    float  reflectance; /* 0 = matte, 1 = mirror       */
} material_t;

/* ----------------------------------------------------------
 * sphere_t
 * ---------------------------------------------------------- */
typedef struct {
    Vector3     center;
    float      radius;
    material_t mat;
} sphere_t;

/* ----------------------------------------------------------
 * camera_t — simple pinhole camera
 *
 * TODO: set these in build_scene() inside main.c.
 * ---------------------------------------------------------- */
typedef struct {
    Vector3 origin;
    Vector3 lower_left;  /* world-space corner of the image plane */
    Vector3 horizontal;  /* full width  vector across image plane */
    Vector3 vertical;    /* full height vector up   image plane   */
} camera_t;

/* ----------------------------------------------------------
 * scene_t — everything a worker needs to render
 * ---------------------------------------------------------- */
#define MAX_SPHERES 32

typedef struct {
    camera_t cam;
    sphere_t spheres[MAX_SPHERES];
    int      num_spheres;
    int      image_width;
    int      image_height;
    int      samples_per_pixel;  /* anti-aliasing sample count */
    int      max_depth;          /* recursion limit            */
} scene_t;


#endif