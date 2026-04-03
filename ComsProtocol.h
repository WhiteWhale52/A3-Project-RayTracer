#ifndef COMSPROTOCOL_H
#define COMSPROTOCOL_H



#include <stdint.h>
#include <unistd.h>

typedef long long LLONG;

/// @brief This is the struct that will hold the message sent from the parent to the worker child. It specifies the rectangular tile of the image that the worker is going to work on. 
typedef struct JobMessage
{
    int m_JobID;
    int m_XStartCoord;
    int m_YStartCoord;
    int m_Width;
    int m_Height;
};

typedef struct JobResultHeader
{
    int m_JobID;
    int PixelCount;
};

typedef Pixel {
    int r,g,b;
}


static inline ssize_t ReadExact(int fd, void *buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t r = read(fd, (char *)buf + total, n - total);
        if (r == 0) return 0;   
        if (r < 0)  return -1;  
        total += (size_t)r;
    }
    return (ssize_t)total;
}

static inline ssize_t WriteExact(int fd, const void *buf, size_t n) {
    size_t total = 0;
    while (total < n) {
        ssize_t w = write(fd, (const char *)buf + total, n - total);
        if (w <= 0) return -1;  
        total += (size_t)w;
    }
    return (ssize_t)total;
}


#endif