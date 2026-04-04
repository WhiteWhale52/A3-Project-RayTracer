#ifndef WORKER_H
#define WORKER_H

#include "ImageStructs.h"

typedef struct {
    int pid;
    int   jobFD;      /* parent writes jobs  here → worker reads  */
    int   resultFD;   /* parent reads results here ← worker writes */
} Worker;

void WorkerRenderLoop(int read_fd, int write_fd, const Scene *scene);

static void Usage(const char *prog);

static void SpawnWorkers(Worker* workers, int n, const Scene* scene);

static void DispatchCollect(Worker* workers, int numOfWorkers, Pixel* frameBuffer, int imgWidth, int imgHeight, int tileSize);

static void RIPWorkers(Worker* workers, int n);

#pragma region Consts 
#define DEFAULT_WIDTH    800
#define DEFAULT_HEIGHT   600
#define DEFAULT_WORKERS  4
#define DEFAULT_TILE_SIZE  64  
#define DEFAULT_SAMPLES  4
#define DEFAULT_DEPTH    8
#pragma endregion Consts

#endif