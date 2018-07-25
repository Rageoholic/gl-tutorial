#include "glad.h"
#include "rgl.h"

#include "rutils/def.h"
#include "rutils/file.h"

#include <GLFW/glfw3.h>
#include <stdio.h>

#define VERTEX_FILE "basic-render.vert"
#define FRAG_FILE "render-orange.frag"

static void FramebufferResize(GLFWwindow *win, int width, int height)
{
    ignore win;
    glViewport(0, 0, width, height);
}
// TODO: Look into dynamic code loading for the rendering functions because
// *not* having that is a pain
int main(int argc, char **argv)
{
    ignore argc;
    ignore argv;

    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *win = glfwCreateWindow(1280, 720, "LearnOpenGL", NULL, NULL);
    glfwSetWindowAspectRatio(win, 16, 9);
    if (win == NULL)
    {
        fputs("Failed to make window", stderr);
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fputs("Failed to load gl funcs", stderr);
        return -1;
    }
    glEnable(GL_MULTISAMPLE);
    s32 width, height;
    glfwGetWindowSize(win, &width, &height);

    FramebufferResize(win, width, height);
    glfwSetFramebufferSizeCallback(win, FramebufferResize);

    float vertices[] = {.5, -.5, 0,
                        0, .5, 0,
                        -.5, -.5, 0};

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);

    ssize_t vertShaderSourceSize;
    char *vertShaderSource = MapFileToROBuffer(VERTEX_FILE, NULL, &vertShaderSourceSize);

    glShaderSource(vertShader, 1, (const char **)&vertShaderSource, NULL);
    glCompileShader(vertShader);

    int success;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        fprintf(stderr, "COMPILE ERROR IN VERT SHADER %s\n", infoLog);
    }
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    ssize_t fragShaderSourceSize;
    char *fragShaderSource = MapFileToROBuffer(FRAG_FILE, NULL, &fragShaderSourceSize);

    glShaderSource(fragShader, 1, (const char **)&fragShaderSource, NULL);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        fprintf(stderr, "COMPILE ERROR IN FRAG SHADER %s\n", infoLog);
    }

    GLuint shaderProg = glCreateProgram();

    glAttachShader(shaderProg, vertShader);
    glAttachShader(shaderProg, fragShader);
    glLinkProgram(shaderProg);

    glGetShaderiv(shaderProg, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProg, 512, NULL, infoLog);
        fprintf(stderr, "LINK ERROR IN SHADER PROG %s\n", infoLog);
        return -1;
    }

    while (!(glfwWindowShouldClose(win)))
    {
        /* Input handling */
        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(win, true);
        }
        if (glfwGetKey(win, GLFW_KEY_W))
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (glfwGetKey(win, GLFW_KEY_F))
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        /* Drawing */
        RGLClearScreen((Color){.1, .5, .5});

        glUseProgram(shaderProg);
        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Final book keeping */
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
