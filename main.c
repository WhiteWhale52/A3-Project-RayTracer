#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "CommunicationsProtocol.h"
#include "RayTracer.h"
#include "PPM.h"

#pragma region Consts 
#define DEFAULT_WIDTH    800
#define DEFAULT_HEIGHT   600
#define DEFAULT_WORKERS  4
#define DEFAULT_TILE_SIZE  64  
#define DEFAULT_SAMPLES  4
#define DEFAULT_DEPTH    8
#pragma endregion Consts

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
    
    ReapWorkers(m_Workers, m_NumofWorkers);

}