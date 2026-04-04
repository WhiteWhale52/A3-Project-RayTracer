#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#include "CommunicationsProtocol.h"
#include "RayTracer.h"
#include "Worker.h"


void WorkerRenderLoop(int readFD, int writeFD, const Scene* scene){
    int pid = getpid();

    JobMessage jobMesg;

    while(1){
        ssize_t r = ReadExact(readFD, &jobMesg, sizeof(jobMesg));


        // This means that the parent closed this worker's write end
        if (r == 0)         break;

        if (r != sizeof(jobMesg)) {
            fprintf(stderr, "[worker %d] read job failed: %s\n",
             pid, strerror(errno));
            exit(2);
        }

        int numOfPixels = jobMesg.m_Width * jobMesg.m_Height;

        Pixel* pixelBuffer = malloc(numOfPixels * sizeof(Pixel));
        if (!pixelBuffer){
            fprintf(stderr, "[worker %d] malloc failed\n", pid);
            exit(3);
        }

        for (int  row = 0; row < jobMesg.m_Height; row++)
        {
            for (int col = 0; col < jobMesg.m_Width; col++)
            {
                int pixelX = jobMesg.m_XStartCoord + col;
                int pixelY = jobMesg.m_YStartCoord + row;

                pixelBuffer[row * jobMesg.m_Width + col] = TracePixel(scene, pixelX, pixelY);
            }
        }

        JobResultHeader resultHeader = (JobResultHeader){jobMesg.m_JobID, numOfPixels};

        if (WriteExact(writeFD, &resultHeader, sizeof(JobResultHeader)) != sizeof(JobResultHeader)){
            fprintf(stderr, "[Worker %d] write into header failed: %s\n", pid, strerror(errno));

            free(pixelBuffer);
            exit(1);
        }
        

        size_t payload = numOfPixels * sizeof(Pixel);
        if (WriteExact(writeFD, pixelBuffer, payload) != (ssize_t) payload){
            fprintf(stderr, "[Worker %d] write pixels failed: %s\n", pid, strerror(errno));
            free(pixelBuffer);
            exit(1);
        }

        free(pixelBuffer);

        fprintf(stderr, "[Worker %d] finished tile %u ( %ux%u at %u,%u)\n", pid, 
        jobMesg.m_JobID, jobMesg.m_Width, jobMesg.m_Height, jobMesg.m_XStartCoord, jobMesg.m_YStartCoord);


    }
    close(readFD);
    close(writeFD);
    exit(0);
}

void SpawnWorkers(Worker* workers, int n, const Scene* scene){
    for (int i = 0; i < n; i++)
    {
        int jobPipe[2], resultPipe[2];

        if (pipe(jobPipe) == -1){
            perror("Error in creating a pipe for the job between the child and parent");
            exit(1);
        }
        if (pipe(resultPipe) == -1){
            perror("Error in creating a pipe for the result struct between the child and parent");
            exit(1);
        }

        int pid = fork();
        if(pid == -1){
            perror("Error in forking");
            exit(1);
        }

        if (pid == 0){
            close(jobPipe[1]);
            close(resultPipe[0]);

            for (int j = 0; j < i; j++)
            {
                close(workers[j].jobFD);
                close(workers[j].resultFD);
            }

            WorkerRenderLoop(jobPipe[0], resultPipe[1], scene);
        }

        close(jobPipe[0]);
        close(resultPipe[1]);

        workers[i].pid       = pid;
        workers[i].jobFD    = jobPipe[1];
        workers[i].resultFD = resultPipe[0];

        printf("[parent] spawned worker %d  (pid %d)\n", i, pid);
    }
}


void DispatchCollect(Worker* workers, int numOfWorkers, Pixel* frameBuffer, int imgWidth, int imgHeight, int tileSize){
    int tileX = (imgWidth + tileSize - 1) / tileSize;
    int tileY = (imgHeight + tileSize - 1) / tileSize;

    int totalTiles = tileX * tileY;

    // Here we computed total number of tiles we need.
    printf("[parent] dispatching %d tiles to %d workers\n", totalTiles, numOfWorkers);

    int jobID = 0;

    for (int dy = 0; dy < tileY; dy++)
    {
        for (int dx = 0; dx < tileX; dx++)
        {
            int currentWorkerID = jobID % numOfWorkers;

            JobMessage jobMessage;
            jobMessage.m_JobID = jobID;
            jobMessage.m_XStartCoord = (dx * tileSize);
            jobMessage.m_YStartCoord = (dy * tileSize);
            jobMessage.m_Width = (dx + 1) * tileSize < imgWidth ? 
                tileSize :
                imgWidth - dx * tileSize;
            jobMessage.m_Height = (dy + 1) * tileSize < imgHeight ?
                tileSize :
                imgHeight - dy * tileSize;
            
            if (WriteExact(workers[currentWorkerID].jobFD, &jobMessage, sizeof(JobMessage)) != sizeof(JobMessage)){
                fprintf(stderr, "[parent] write job failed for worker with id %d: %s\n", currentWorkerID, strerror(errno));
                exit(1);
            }

            jobID++;
        }
    }

    for (int i = 0; i < numOfWorkers; i++)        close(workers[i].jobFD);

    for (int i = 0; i < numOfWorkers; i++)
    {
        workers[i].jobFD = -1;
    }

    int workersDone = 0;

    int* openWorkers = calloc((size_t) numOfWorkers, sizeof(int));
    for (int i = 0; i < numOfWorkers; i++) openWorkers[i] = 1;

    while (workersDone < numOfWorkers)
    {
        for (int i = 0; i < numOfWorkers; i++)
        {
            if (!openWorkers[i]){
                continue;
            }

            JobResultHeader resultHeader;
            ssize_t returnVal = ReadExact(workers[i].resultFD, &resultHeader, sizeof(JobResultHeader));
            if (returnVal == 0) {
                close(workers[i].resultFD);
                openWorkers[i]=0;
                workersDone++;
                continue;
            }
            
            if (returnVal < 0){
                fprintf(stderr, "[parent] reading the Job result header failed" " worker %d: %s\n", i, strerror(errno));
                exit(1);
            }

            Pixel* tileBuffer = malloc(resultHeader.PixelCount*sizeof(Pixel));
            if (!tileBuffer) {
                perror("Malloc of tile buffer failed");
                exit(1);
            }

            size_t bytes = resultHeader.PixelCount * sizeof(Pixel);
            if (ReadExact(workers[i].resultFD, tileBuffer, bytes) != (ssize_t)bytes) {
                fprintf(stderr, "[parent] reading pixels from tile buffer failed for worker %d\n", i);
                exit(1);
            }
            
            int tileXLocation = (imgWidth + tileSize - 1) / tileSize;
            int jobID = resultHeader.m_JobID;
            int c_TileX = jobID % tileXLocation;
            int c_TileY = jobID / tileXLocation;
            int xStartCoord = c_TileX * tileSize;
            int yStartCoord = c_TileY * tileSize;
            int c_TileWidth = (xStartCoord + tileSize < imgWidth) ? tileSize : imgWidth - xStartCoord;
            int c_TileHeight = (yStartCoord + tileSize < imgHeight) ? tileSize : imgHeight - yStartCoord;

            for (int row = 0; row < c_TileHeight; row++) {
                int dst = (yStartCoord + row) * imgWidth + xStartCoord;
                int src = row * c_TileWidth;
                memcpy(&frameBuffer[dst], &tileBuffer[src], (size_t)c_TileWidth * sizeof(Pixel));
            }

            free(tileBuffer);
            printf("[parent] received tile %u  (%d×%d @ %d,%d)\n",
                  resultHeader.m_JobID, c_TileWidth, c_TileHeight, xStartCoord, yStartCoord);
        }
    }
    free(openWorkers);
}

void RIPWorkers(Worker* workers, int n){
    for (int i = 0; i < n; i++)
    {
        int status;
        if (waitpid(workers[i].pid, &status, 0) == -1){
            perror("WaitPID error");
        } else if (WIFEXITED(status) && WEXITSTATUS(status) !=0){
            fprintf(stderr, "[parent] worker %d exited with code %d\n", i, WEXITSTATUS(status));
        }
    }  
}


void Usage(const char *prog){
      fprintf(stderr,
        "Usage: %s [options]\n"
        "  -w <width>    image width  (default %d)\n"
        "  -h <height>   image height (default %d)\n"
        "  -j <workers>  worker count (default %d)\n"
        "  -t <tile>     tile size    (default %d)\n"
        "  -s <samples>  samples/px   (default %d)\n"
        "  -d <depth>    max depth    (default %d)\n"
        "  -o <file>     output PPM   (default output.ppm)\n",
        prog,
        DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_WORKERS,
        DEFAULT_TILE_SIZE, DEFAULT_SAMPLES, DEFAULT_DEPTH);
}


