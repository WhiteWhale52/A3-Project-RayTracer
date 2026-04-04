#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "CommunicationsProtocol.h"
#include "RayTracer.h"
#include "Worker.h"
#include "PPM.h"


int main(int argc, char* argv[]){
    int m_ImageWidth = DEFAULT_WIDTH;
    int m_ImageHeight = DEFAULT_HEIGHT;
    int m_NumofWorkers = DEFAULT_WORKERS;
    int m_TileSize = DEFAULT_TILE_SIZE;
    int m_Samples = DEFAULT_SAMPLES;
    int m_Depth = DEFAULT_DEPTH;

    const char* out = "Output.ppm";

     for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-help")) {
        Usage(argv[0]);
        return 0;
        }
        else if      (!strcmp(argv[i], "-w") && i+1 < argc) m_ImageWidth     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-h") && i+1 < argc) m_ImageHeight     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-j") && i+1 < argc) m_NumofWorkers = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-t") && i+1 < argc) m_TileSize   = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-s") && i+1 < argc) m_Samples   = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-d") && i+1 < argc) m_Depth     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-o") && i+1 < argc) out   = argv[++i];
        else { Usage(argv[0]); return 1; }
    }

    if (m_NumofWorkers < 1) {
        fprintf(stderr, "Need at least 1 worker child.\n"); 
        return 1;
    }

    Scene m_Scene;
    memset(&m_Scene,0, sizeof(m_Scene));

    /* TODO 4 — call build_scene() with your sphere layout.
     * Implement build_scene() at the bottom of this file.
     * Set scene.cam, populate scene.spheres[], set
     * scene.num_spheres.  Everything else is filled below.    */
    BuildScene(&m_Scene, m_ImageWidth, m_ImageHeight, m_Samples, m_Depth);

    Pixel* m_frameBuffer = calloc((size_t) m_ImageHeight * m_ImageWidth, sizeof(Pixel));
    if (!m_frameBuffer){
        perror("Error in calloc of the frameBuffer");
        return 1;
    }

    Worker* m_Workers = calloc((size_t) m_NumofWorkers, sizeof(Worker));
    if (!m_Workers){
        perror("Error in calloc of the workers");
        return 1;
    }

    SpawnWorkers(m_Workers, m_NumofWorkers, &m_Scene);

    DispatchCollect(m_Workers, m_NumofWorkers, m_frameBuffer, m_ImageWidth, m_ImageHeight, m_TileSize);

    for (int i = 0; i < m_NumofWorkers; i++)
    {
        close(m_Workers[i].jobFD);
    }
    
    RIPWorkers(m_Workers, m_NumofWorkers);

    if (WritePPM(out, m_frameBuffer, m_ImageWidth, m_ImageHeight) != 0){
        fprintf(stderr, "Failed to write %s\n", out);
        free(m_frameBuffer); 
        free(m_Workers);
        return 1;
    }

    printf("Wrote %s (%dx%d, %d workers, tile size %d, %d samples per pixel)\n",
                  out, m_ImageWidth, m_ImageHeight, m_NumofWorkers, m_TileSize, m_Samples);

    free(m_frameBuffer);
    free(m_Workers);

    return 0;

}

void BuildScene(Scene* scene, int width, int height, int samples, int max_depth){
    scene->image_width = width;
    scene->image_height = height;
    scene->max_depth = max_depth;
    scene->num_spheres = 0;
    scene->samples_per_pixel = samples;
   

    Vector3 look_from = {0.0f,  0.0f, 6.0f};  /* m_Position     */
    Vector3 look_at   = {0.0f,  0.0f, 0.0f};  /* pos + forward  */
    Vector3 viewUp       = {0.0f,  1.0f, 0.0f};
    float  viewFOV      = 45.0f;                 /* m_VerticalFOV  */
    float  aspect    = (float)width / (float)height;

    /* halfHeight = tan(vFOV/2) — same value that glm::perspectiveFov
     * encodes into the projection matrix's [1][1] entry        */
    float halfHeight = tanf(viewFOV * (float)M_PI / 180.0f / 2.0f);
    float halfWidth = aspect * halfHeight;

    /* orthonormal camera basis — equivalent to glm::lookAt:
     *   w  = normalize(eye - target)    right-handed -Z forward
     *   u  = normalize(cross(viewUp, w))   points right
     *   v  = cross(w, u)                points up               */
    Vector3 w = Norm(Subtract(look_from, look_at));
    Vector3 u = Norm(Cross(viewUp, w));
    Vector3 v = Cross(w, u);

    scene->camera.origin     = look_from;
    scene->camera.horizontal = Scale(u, 2.0f * halfWidth);
    scene->camera.vertical   = Scale(v, 2.0f * halfHeight);
    scene->camera.lower_left = Subtract(Subtract(Subtract(look_from, Scale(u, halfWidth)), Scale(v, halfHeight)),
                 w);

    /* ── Spheres — matching the default scene from your C++ app */
    int n;

    /* Sphere 1 — pink diffuse (centre stage)                   */
    n = scene->num_spheres;
    scene->spheres[n].origin             = (Vector3){ 0.0f,  0.0f, 0.0f};
    scene->spheres[n].radius             = 1.0f;
    scene->spheres[n].material.color          = (Vector3){1.0f, 0.0f, 1.0f};
    scene->spheres[n].material.roughness      = 0.5f;
    scene->spheres[n].material.emissionColor = (Vector3){0.0f,0.0f, 0.0f};
    scene->spheres[n].material.emissionPower = 0.0f;
    scene->num_spheres++;

    /* Sphere 2 — blue diffuse (right)                          */
    n = scene->num_spheres;
    scene->spheres[n].origin             = (Vector3){ 2.0f,  0.0f, 0.0f};
    scene->spheres[n].radius             = 1.0f;
    scene->spheres[n].material.color          = (Vector3){0.2f, 0.3f, 1.0f};
    scene->spheres[n].material.roughness      = 0.1f;
    scene->spheres[n].material.emissionColor = (Vector3){0.0f, 0.0f, 0.0f};
    scene->spheres[n].material.emissionPower = 0.0f;
    scene->num_spheres++;

    /* Sphere 3 — emissive orange light (upper left)            */
    n = scene->num_spheres;
    scene->spheres[n].origin             = (Vector3){-2.0f,  2.0f, -3.0f};
    scene->spheres[n].radius             = 1.5f;
    scene->spheres[n].material.color          = (Vector3){0.8f, 0.5f, 0.2f};
    scene->spheres[n].material.roughness      = 0.0f;
    scene->spheres[n].material.emissionColor = (Vector3){0.9f, 0.6f, 0.2f};
    scene->spheres[n].material.emissionPower = 200.0f; 
    scene->num_spheres++;

    /* Sphere 4 — grey ground plane (huge sphere trick)         */
    n = scene->num_spheres;
    scene->spheres[n].origin             = (Vector3){ 0.0f, -101.0f, 0.0f};
    scene->spheres[n].radius             = 100.0f;
    scene->spheres[n].material.color          = (Vector3){0.5f, 0.5f, 0.5f};
    scene->spheres[n].material.roughness      = 0.8f;
    scene->spheres[n].material.emissionColor = (Vector3){0.0f, 0.0f, 0.0f};
    scene->spheres[n].material.emissionPower = 0.0f;
    scene->num_spheres++;
}

