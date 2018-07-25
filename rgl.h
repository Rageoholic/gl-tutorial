#ifndef RGL_H
#define RGL_H
#include "rutils/def.h"
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct
    {
        float r;
        float g;
        float b;
        float a;
    } Color;

    void RGLClearScreen(Color c);
#ifdef __cplusplus
}
#endif
#endif
