#ifndef RGL_H
#define RGL_H
#include "glad.h"
#include "rutils/def.h"
#include "rutils/math.h"
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

    typedef struct
    {
        GLuint _id;
    } ShaderProg;

    void RGLClearScreen(Color c);

    void UseShaderProg(ShaderProg s);
    void SetUniformVec3fShaderProg(ShaderProg s, char *uniformName, Vec3f v);
    void SetUniformMat4fShaderProg(ShaderProg s, char *uniformName, Mat4f *m);
    void SetUniformFloatShaderProg(ShaderProg s, char *uniformName, float f);
    void SetUniformIntShaderProg(ShaderProg s, char *uniformName, int i);

    ShaderProg CreateShaderProg(char *vertShaderPath, char *fragShaderPath);
#ifdef __cplusplus
}
#endif
#endif
