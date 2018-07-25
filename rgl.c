#include "rgl.h"
#include "glad.h"

void RGLClearScreen(Color c)
{
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
}
