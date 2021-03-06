#include "glad.h"
#include "rgl.h"

#include "rutils/def.h"
#include "rutils/file.h"
#include "rutils/math.h"
#include "stb_image.h"

#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

#define VERTEX_FILE "transform-render.vert"
#define FRAG_FILE "render-with-2-textures.frag"
#define TEXTURE_PATH "data/container.jpg"
#define TEXTURE_PATH_2 "data/awesomeface.png"

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

    /* Window setup */
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    stbi_set_flip_vertically_on_load(true);

    GLFWwindow *win = glfwCreateWindow(1280, 720, "LearnOpenGL", NULL, NULL);
    glfwSetWindowAspectRatio(win, 800, 600);
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

    /* Enable multisampling */
    glEnable(GL_MULTISAMPLE);

    /* Set up our viewport */
    s32 width, height;
    glfwGetWindowSize(win, &width, &height);

    FramebufferResize(win, width, height);
    glfwSetFramebufferSizeCallback(win, FramebufferResize);

    /* Prep vertex array */

    float vertices[] = {

        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };

    u32 indices[] = {0, 1, 3,
                     1, 2, 3};

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    /* Set up vertex buffer */
    GLuint VBO, EBO;
    glGenBuffers(1, &VBO);

    /* Buffer the data */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    /* Set up our shaders */
    /* TODO: better shader abstraction */
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    {

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
        UnmapMappedBuffer(vertShaderSource, vertShaderSourceSize);
    }
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    {
        ssize_t fragShaderSourceSize;
        char *fragShaderSource = MapFileToROBuffer(FRAG_FILE, NULL, &fragShaderSourceSize);

        glShaderSource(fragShader, 1, (const char **)&fragShaderSource, NULL);
        glCompileShader(fragShader);
        int success;

        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
            fprintf(stderr, "COMPILE ERROR IN FRAG SHADER %s\n", infoLog);
        }
        UnmapMappedBuffer(fragShaderSource, fragShaderSourceSize);
    }
    GLuint shaderProg = glCreateProgram();
    {
        glAttachShader(shaderProg, vertShader);
        glAttachShader(shaderProg, fragShader);
        glLinkProgram(shaderProg);
        int success;
        glGetShaderiv(shaderProg, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(shaderProg, 512, NULL, infoLog);
            fprintf(stderr, "LINK ERROR IN SHADER PROG %s\n", infoLog);
            return -1;
        }
    }
    /* Load our texture */

    GLuint texture[2];
    glGenTextures(2, texture);
    {
        int width, height, nrChannels;
        byte *textureData = stbi_load(TEXTURE_PATH, &width, &height, &nrChannels, 0);
        if (textureData)
        {
            glBindTexture(GL_TEXTURE_2D, texture[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(textureData);
        }
        else
        {
            fputs("TEXTURE MISSING", stderr);
        }
    }
    {
        int width, height, nrChannels;
        byte *textureData = stbi_load(TEXTURE_PATH_2, &width, &height, &nrChannels, 0);
        if (textureData)
        {
            glBindTexture(GL_TEXTURE_2D, texture[1]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(textureData);
        }
        else
        {
            fputs("TEXTURE MISSING", stderr);
        }
    }

    glUseProgram(shaderProg);
    glUniform1i(glGetUniformLocation(shaderProg, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shaderProg, "texture2"), 1);

    /* Calculate our transform */
    Mat4f rotMat = RotateMat4f(&IdMat4f, DegToRad(-90), vec3f(0, 0, 1));

    Mat4f rotAndScaleMat = ScaleMat4f(&rotMat, vec3f(.5, .5, .5));

    glUniformMatrix4fv(glGetUniformLocation(shaderProg, "transform"), 1,
                       GL_FALSE, (float *)&rotAndScaleMat);
    /* Main loop */
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
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture[1]);

        Mat4f transMat = IdMat4f;
        transMat = TranslateMat4f(&IdMat4f,
                                  vec3f(sin(glfwGetTime() * 3 / 10),
                                        cos(glfwGetTime() * 5 / 10),
                                        0));
        transMat = RotateMat4f(&transMat, glfwGetTime() * 2, vec3f(0, 1, 1));
        transMat = ScaleMat4f(&transMat, vec3f(.5, .5, .5));

        glUniformMatrix4fv(glGetUniformLocation(shaderProg, "transform"), 1,
                           GL_FALSE, (float *)&transMat);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        /* Final book keeping */
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
