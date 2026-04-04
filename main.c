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
        if      (!strcmp(argv[i], "-w") && i+1 < argc) m_ImageWidth     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-h") && i+1 < argc) m_ImageHeight     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-j") && i+1 < argc) m_NumofWorkers = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-t") && i+1 < argc) m_TileSize   = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-s") && i+1 < argc) m_Samples   = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-d") && i+1 < argc) m_Depth     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-o") && i+1 < argc) out   = argv[++i];
        else { usage(argv[0]); return 1; }
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

    printf("Wrote %s (%dx%d, %d workers, tile %d, %d samples per pixel)\n",
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
   
    /* ── TODO: set up camera ─────────────────────────────────
     *
     * Hint — standard pinhole setup:
     *
     *   vec3_t look_from = {3, 2, 5};
     *   vec3_t look_at   = {0, 0, 0};
     *   vec3_t vup       = {0, 1, 0};
     *   float  vfov      = 40.0f;   // vertical FOV in degrees
     *   float  aspect    = (float)width / (float)height;
     *
     *   float theta      = vfov * (float)M_PI / 180.0f;
     *   float half_h     = tanf(theta / 2.0f);
     *   float half_w     = aspect * half_h;
     *
     *   vec3_t w = vec3_norm(vec3_sub(look_from, look_at));
     *   vec3_t u = vec3_norm(vec3_cross(vup, w));
     *   vec3_t v = vec3_cross(w, u);
     *
     *   scene->cam.origin      = look_from;
     *   scene->cam.horizontal  = vec3_scale(u, 2*half_w);
     *   scene->cam.vertical    = vec3_scale(v, 2*half_h);
     *   scene->cam.lower_left  = look_from
     *                            - half_w*u - half_h*v - w;
     */

    /* ── TODO: add spheres ───────────────────────────────────
     *
     * Example:
     *   int n = scene->num_spheres;
     *   scene->spheres[n].origin        = (vec3_t){0, 0, -1};
     *   scene->spheres[n].radius        = 0.5f;
     *   scene->spheres[n].mat.color     = (vec3_t){0.8, 0.3, 0.3};
     *   scene->spheres[n].mat.reflectance = 0.0f;
     *   scene->num_spheres++;
     *
     *   // ground plane as a huge sphere
     *   n = scene->num_spheres;
     *   scene->spheres[n].origin        = (vec3_t){0, -100.5, -1};
     *   scene->spheres[n].radius        = 100.0f;
     *   scene->spheres[n].mat.color     = (vec3_t){0.5, 0.5, 0.5};
     *   scene->spheres[n].mat.reflectance = 0.0f;
     *   scene->num_spheres++;
     */
}

