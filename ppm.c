#include <stdio.h>
#include <errno.h>
#include "ppm.h"

int WritePPM(const char* filename, const Pixel* pixels, int width, int height){
    FILE* file = fopen(filename, "wb");

    if (!file) {
       fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(errno));
       return -1;
    }

    fprintf(file, "P6\n%d %d\n255\n", width, height);

    size_t numOfPixels = (size_t) width * height;

    if (fwrite(pixels, sizeof(Pixel), numOfPixels, file) != numOfPixels){
        perror("Error in writing to pixels");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}
