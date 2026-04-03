#ifndef PPM_H
#define PPM_H

#include "ImageStructs.h"


int WritePPM(const char* filename, const Pixel* pixels, int width, int height);

#endif