#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <math.h>
#include <stdlib.h>
#include <float.h>

typedef struct {
    float x, y, z;
} Vector3;

static inline Vector3 Add(Vector3 a, Vector3 b) {
    return (Vector3){
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}
static inline Vector3 Subtract(Vector3 a, Vector3 b){
       return (Vector3){
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

static inline Vector3 Scale(Vector3 v, float s){
       return (Vector3){
        v.x * s,
        v.y * s,
        v.z * s
    };
}

static inline Vector3 Mul(Vector3 a, Vector3 b){
       return (Vector3){
        a.x * b.x,
        a.y * b.y,
        a.z * b.z
    };
} 

static inline float  Dot(Vector3 a, Vector3 b){
       return 
        a.x * b.x + 
        a.y * b.y +
        a.z * b.z
    ;
}

static inline Vector3 Cross(Vector3 a, Vector3 b){
     return (Vector3){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

static inline float Length(Vector3 v){
     return sqrtf(Dot(v, v));
}

static inline Vector3 Norm(Vector3 v) {
     Scale(v, 1.0f / Length(v));
} 

static inline Vector3 Lerp(Vector3 a, Vector3 b, float t){
    Add(Scale(a, 1.0f-t), Scale(b, t));
}


static int PCGHash(int input) {
    int state = input * 747796405u + 2891336453u;
    int word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

/* Advances *seed and returns a float in [0, 1). */
static float RandFloat(int *seed) {
    *seed = PCGHash(*seed);
    return (float)*seed / (float)_I32_MAX;
}

/* Random unit vector — port of Utilities::InUnitSphere(seed).
 * Generates a point in [-1,1]^3 then normalises it.          */
static Vector3 RandomUnitVector(int *seed) {
    return Norm((Vector3){
        RandFloat(seed) * 2.0f - 1.0f,
        RandFloat(seed) * 2.0f - 1.0f,
        RandFloat(seed) * 2.0f - 1.0f
    });
}


static float randf(void) {
    return (float)rand() / ((float)RAND_MAX + 1.0f);
}



#endif