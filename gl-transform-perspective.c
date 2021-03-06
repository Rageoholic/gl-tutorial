#include "glad.h"
#include "rgl.h"

#include "rutils/def.h"
#include "rutils/file.h"
#include "rutils/math.h"
#include "stb_image.h"

#define countof(x) (sizeof(x) / sizeof(x[0]))

#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define VERTEX_FILE "transform-projection.vert"
#define FRAG_FILE "render-with-2-textures.frag"
#define TEXTURE_PATH "data/container.jpg"
#define TEXTURE_PATH_2 "data/awesomeface.png"
#define MOVE_SPEED 4

Vec3f cameraFront = (Vec3f){0.0f, 0.0f, -1.0f};
static void FramebufferResize(GLFWwindow *win, int width, int height)
{
    ignore win;
    glViewport(0, 0, width, height);
}

float lastX = 640;
float lastY = 360;
float yaw = -90;
float pitch = 0;
bool firstMouse = true;
float fov = 70;

void ScrollCallback(GLFWwindow *win, double xoffset, double yoffset)
{
    ignore win;
    ignore xoffset;
    if (fov >= 1 && fov <= 70)
    {
        fov -= yoffset * 2;
    }
    if (fov <= 1)
    {
        fov = 1;
    }
    if (fov >= 70)
    {
        fov = 70;
    }
}

void MouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    ignore window;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    Vec3f front;
    front.x = cos(DegToRad(yaw)) * cos(DegToRad(pitch));
    front.y = sin(DegToRad(pitch));
    front.z = sin(DegToRad(yaw)) * cos(DegToRad(pitch));
    cameraFront = NormalizeVec3f(front);
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
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowAspectRatio(win, 1280, 720);
    if (win == NULL)
    {
        fputs("Failed to make window", stderr);
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(win);
    glfwSetScrollCallback(win, ScrollCallback);
    glfwSetCursorPosCallback(win, MouseCallback);

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
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

    Vec3f cubePositions[] = {
        {0.0f, 0.0f, 0.0f},
        {2.0f, 5.0f, -15.0f},
        {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f},
        {2.4f, -0.4f, -3.5f},
        {-1.7f, 3.0f, -7.5f},
        {1.3f, -2.0f, -2.5f},
        {1.5f, 2.0f, -2.5f},
        {1.5f, 0.2f, -1.5f},
        {-1.3f, 1.0f, -1.5f}};

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    /* Set up vertex buffer */
    GLuint VBO;
    glGenBuffers(1, &VBO);

    /* Buffer the data */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderProg);
    glUniform1i(glGetUniformLocation(shaderProg, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shaderProg, "texture2"), 1);

    glClearColor(.3, .4, .5, 1);
    clock_t totalTime = 0;
    size_t numSamples = 0;

    Vec3f cameraPos = vec3f(0.0f, 0.0f, 3.0f);

    Vec3f cameraUp = vec3f(0.0f, 1.0f, 0.0f);

    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    /* Main loop */
    while (!(glfwWindowShouldClose(win)))
    {

        clock_t begin = clock();
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float moveSpeed = MOVE_SPEED * deltaTime;

        /* Input handling */
        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(win, true);
        }
        if (glfwGetKey(win, GLFW_KEY_F1) == GLFW_PRESS)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (glfwGetKey(win, GLFW_KEY_F2) == GLFW_PRESS)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPos = AddVec3f(cameraPos, MultiplyScalarVec3f(cameraFront, moveSpeed));
        }
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPos = SubVec3f(cameraPos, MultiplyScalarVec3f(NormalizeVec3f(
                                                                    CrossProductVec3f(cameraFront,
                                                                                      cameraUp)

                                                                        ),
                                                                moveSpeed));
        }
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPos = SubVec3f(cameraPos, MultiplyScalarVec3f(cameraFront, moveSpeed));
        }
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPos = AddVec3f(cameraPos, MultiplyScalarVec3f(NormalizeVec3f(
                                                                    CrossProductVec3f(cameraFront,
                                                                                      cameraUp)

                                                                        ),
                                                                moveSpeed));
        }

        /* Drawing */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glBindVertexArray(VAO);

        Mat4f view = CalcLookAtMat4f(cameraPos, AddVec3f(cameraPos, cameraFront), cameraUp);

        GLuint viewLoc = glGetUniformLocation(shaderProg, "view");
        glUniformMatrix4fv(viewLoc, 1,
                           GL_FALSE, (float *)&view);

        Mat4f proj = CreatePerspectiveMat4f(DegToRad(fov), 800. / 600., .1, 1000);

        GLuint projLoc = glGetUniformLocation(shaderProg, "projection");
        glUniformMatrix4fv(projLoc, 1,
                           GL_FALSE, (float *)&proj);

        for (size_t i = 0; i < countof(cubePositions); i++)
        {
            Mat4f model = TranslateMat4f(&IdMat4f, cubePositions[i]);

            model = RotateMat4f(&model, (float)glfwGetTime(), vec3f(deltaTime, 1, 0));
            float angle = 13 * i;

            model = RotateMat4f(&model,
                                DegToRad(angle) * (float)glfwGetTime() / 3,
                                vec3f(1, .3, .5));

            glUniformMatrix4fv(glGetUniformLocation(shaderProg, "model"), 1,
                               GL_FALSE, (float *)&model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        clock_t end = clock();

        printf("Took %f milliseconds\n", (double)(end - begin) / CLOCKS_PER_SEC * 1000);

        totalTime += end - begin;
        numSamples++;

        /* Final book keeping */
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    printf("Took %f milliseconds  on average\n",
           (double)(totalTime / numSamples) / CLOCKS_PER_SEC * 1000);
    glfwTerminate();
    return 0;
}
