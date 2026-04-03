#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "ImageStructs.h"


Pixel TracePixel(const Scene *scene, int pixelX, int pixelY);

/* ----------------------------------------------------------
 * Scene builder — called once in main.c before forking.
 *
 * TODO: implement in main.c (or a separate scene.c if you
 * prefer).  Populate scene->spheres, scene->cam, etc.
 * ---------------------------------------------------------- */
void BuildScene(Scene *scene, int width, int height,
                 int samples, int max_depth);

#endif