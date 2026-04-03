#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "ImageStructs.h"
#include "CommunicationsProtocol.h"
#include "RayTracer.h"


void WorkerRenderLoop(int readFD, int writeFD, const Scene* scene){
    int pid = getpid();

    JobMessage jobMesg = {};

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
            fprintf(stderr, "[Worder %d] write pixels failed: %s\n", pid, strerror(errno));
            free(pixelBuffer);
            exit(1);
        }

        free(pixelBuffer);

        fprintf(stderr, "[Worder %d] finished tile %u ( %ux%u at %u,%u)\n", pid, 
        jobMesg.m_JobID, jobMesg.m_Width, jobMesg.m_Height, jobMesg.m_XStartCoord, jobMesg.m_YStartCoord);


    }
    close(readFD);
    close(writeFD);
    exit(0);
}
