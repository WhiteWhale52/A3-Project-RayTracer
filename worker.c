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
            fprintf
        }
    }
}
