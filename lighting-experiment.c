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

#define CUBE_VERTEX_FILE "lighting-experiment.vert"
#define CUBE_FRAG_FILE "lighting-experiment.frag"
#define LIGHT_VERTEX_FILE "lighting-experiment-light.vert"
#define LIGHT_FRAG_FILE "lighting-experiment-light.frag"
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
static void ProcessInput(float deltaTime, GLFWwindow *win,
                         const Vec3f *cameraUp, const Vec3f *cameraFront,
                         Vec3f *cameraPos, Vec3f *modelPos)
{
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
        *cameraPos = AddVec3f(*cameraPos, MultiplyScalarVec3f(*cameraFront, moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    {
        *cameraPos = SubVec3f(*cameraPos, MultiplyScalarVec3f(NormalizeVec3f(
                                                                  CrossProductVec3f(*cameraFront,
                                                                                    *cameraUp)),
                                                              moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    {
        *cameraPos = SubVec3f(*cameraPos, MultiplyScalarVec3f(*cameraFront, moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    {
        *cameraPos = AddVec3f(*cameraPos, MultiplyScalarVec3f(NormalizeVec3f(
                                                                  CrossProductVec3f(*cameraFront,
                                                                                    *cameraUp)),
                                                              moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        *cameraPos = AddVec3f(*cameraPos, MultiplyScalarVec3f(NormalizeVec3f(*cameraUp),
                                                              moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        *cameraPos = AddVec3f(*cameraPos, MultiplyScalarVec3f(NormalizeVec3f(*cameraUp),
                                                              -moveSpeed));
    }

    if (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        *modelPos = AddVec3f(*modelPos, MultiplyScalarVec3f(vec3f(1, 0, 0), moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        *modelPos = AddVec3f(*modelPos, MultiplyScalarVec3f(vec3f(-1, 0, 0), moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS)
    {
        *modelPos = AddVec3f(*modelPos, MultiplyScalarVec3f(vec3f(0, 0, -1), moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        *modelPos = AddVec3f(*modelPos, MultiplyScalarVec3f(vec3f(0, 0, 1), moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
    {
        *modelPos = AddVec3f(*modelPos, MultiplyScalarVec3f(vec3f(0, 1, 0), moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
    {
        *modelPos = AddVec3f(*modelPos, MultiplyScalarVec3f(vec3f(0, -1, 0), moveSpeed));
    }
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
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f};

    unsigned int cubeVAO;
    glGenVertexArrays(1, &cubeVAO);

    glBindVertexArray(cubeVAO);

    /* Set up vertex buffer */
    GLuint VBO;
    glGenBuffers(1, &VBO);

    /* Buffer the data */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    /* Set up our shaders */
    /* TODO: better shader abstraction */
    GLuint cubeVertShader = glCreateShader(GL_VERTEX_SHADER);
    {

        ssize_t cubeVertShaderSourceSize;
        char *cubeVertShaderSource = MapFileToROBuffer(CUBE_VERTEX_FILE, NULL, &cubeVertShaderSourceSize);

        glShaderSource(cubeVertShader, 1, (const char **)&cubeVertShaderSource, NULL);
        glCompileShader(cubeVertShader);

        int success;
        glGetShaderiv(cubeVertShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(cubeVertShader, 512, NULL, infoLog);
            fprintf(stderr, "COMPILE ERROR IN VERT SHADER %s\n", infoLog);
        }
        UnmapMappedBuffer(cubeVertShaderSource, cubeVertShaderSourceSize);
    }

    GLuint lightVertShader = glCreateShader(GL_VERTEX_SHADER);
    {

        ssize_t lightVertShaderSourceSize;
        char *lightVertShaderSource = MapFileToROBuffer(CUBE_VERTEX_FILE, NULL, &lightVertShaderSourceSize);

        glShaderSource(lightVertShader, 1, (const char **)&lightVertShaderSource, NULL);
        glCompileShader(lightVertShader);

        int success;
        glGetShaderiv(lightVertShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(lightVertShader, 512, NULL, infoLog);
            fprintf(stderr, "COMPILE ERROR IN VERT SHADER %s\n", infoLog);
        }
        UnmapMappedBuffer(lightVertShaderSource, lightVertShaderSourceSize);
    }
    GLuint fragShaderCube = glCreateShader(GL_FRAGMENT_SHADER);
    {
        ssize_t fragShaderCubeSourceSize;
        char *fragShaderCubeSource = MapFileToROBuffer(CUBE_FRAG_FILE, NULL, &fragShaderCubeSourceSize);

        glShaderSource(fragShaderCube, 1, (const char **)&fragShaderCubeSource, NULL);
        glCompileShader(fragShaderCube);

        int success;
        glGetShaderiv(fragShaderCube, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(fragShaderCube, 512, NULL, infoLog);
            fprintf(stderr, "COMPILE ERROR IN FRAG SHADER %s\n", infoLog);
        }
        UnmapMappedBuffer(fragShaderCubeSource, fragShaderCubeSourceSize);
    }
    GLuint fragShaderLight = glCreateShader(GL_FRAGMENT_SHADER);
    {
        ssize_t fragShaderLightSourceSize;
        char *fragShaderLightSource = MapFileToROBuffer(LIGHT_FRAG_FILE, NULL, &fragShaderLightSourceSize);

        glShaderSource(fragShaderLight, 1, (const char **)&fragShaderLightSource, NULL);
        glCompileShader(fragShaderLight);

        int success;
        glGetShaderiv(fragShaderLight, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(fragShaderLight, 512, NULL, infoLog);
            fprintf(stderr, "COMPILE ERROR IN FRAG SHADER LIGHT %s\n", infoLog);
        }
        UnmapMappedBuffer(fragShaderLightSource, fragShaderLightSourceSize);
    }
    GLuint cubeProg = glCreateProgram();
    {
        glAttachShader(cubeProg, cubeVertShader);
        glAttachShader(cubeProg, fragShaderCube);
        glLinkProgram(cubeProg);
        int success;
        glGetShaderiv(cubeProg, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(cubeProg, 512, NULL, infoLog);
            fprintf(stderr, "LINK ERROR IN SHADER PROG %s\n", infoLog);
            return -1;
        }
    }

    GLuint lightProg = glCreateProgram();
    {
        glAttachShader(lightProg, lightVertShader);
        glAttachShader(lightProg, fragShaderLight);
        glLinkProgram(lightProg);
        int success;
        glGetShaderiv(lightProg, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(lightProg, 512, NULL, infoLog);
            fprintf(stderr, "LINK ERROR IN SHADER PROG %s\n", infoLog);
            return -1;
        }
    }
    /* Load our texture */

    glEnable(GL_DEPTH_TEST);
    glUseProgram(cubeProg);

    Vec3f objectColor = (Vec3f){1, .5, .31};
    Vec3f lightingColor = (Vec3f){1, 1, 1};
    glUniform3fv(glGetUniformLocation(cubeProg, "objectColor"), 1, (float *)&objectColor);
    glUniform3fv(glGetUniformLocation(cubeProg, "lightColor"), 1, (float *)&lightingColor);

    clock_t totalTime = 0;
    size_t numFrames = 0;

    Vec3f cameraPos = vec3f(0.0f, 0.0f, 3.0f);
    Vec3f cameraUp = vec3f(0.0f, 1.0f, 0.0f);

    Vec3f modelPos = vec3f(0, 0, 0);

    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    /* Main loop */
    while (!(glfwWindowShouldClose(win)))
    {
        clock_t begin = clock();
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ProcessInput(deltaTime, win, &cameraUp, &cameraFront, &cameraPos, &modelPos);

        /* Drawing */
        glClearColor(.1, .1, .1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(cubeProg);

        glBindVertexArray(cubeVAO);

        Mat4f view = CalcLookAtMat4f(cameraPos, AddVec3f(cameraPos, cameraFront), cameraUp);

        Mat4f proj = CreatePerspectiveMat4f(DegToRad(fov), 800. / 600., .1, 1000);

        Mat4f model = IdMat4f;
        Vec3f lightPos = vec3f(1.2, 1.0, 2.0);
        {
            GLuint viewLoc = glGetUniformLocation(cubeProg, "view");
            glUniformMatrix4fv(viewLoc, 1,
                               GL_FALSE, (float *)&view);

            GLuint projLoc = glGetUniformLocation(cubeProg, "projection");
            glUniformMatrix4fv(projLoc, 1,
                               GL_FALSE, (float *)&proj);
            Mat4f cubeModel = TranslateMat4f(&model, modelPos);

            glUniformMatrix4fv(glGetUniformLocation(cubeProg, "model"), 1,
                               GL_FALSE, (float *)&cubeModel);

            GLuint lightLoc = glGetUniformLocation(cubeProg, "lightPos");
            glUniform3fv(lightLoc, 1, (float *)&lightPos);

            GLuint viewPosLoc = glGetUniformLocation(cubeProg, "viewPos");
            glUniform3fv(viewPosLoc, 1, (float *)&cameraPos);
        }
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUseProgram(lightProg);
        {
            GLuint viewLoc = glGetUniformLocation(cubeProg, "view");
            glUniformMatrix4fv(viewLoc, 1,
                               GL_FALSE, (float *)&view);

            GLuint projLoc = glGetUniformLocation(cubeProg, "projection");
            glUniformMatrix4fv(projLoc, 1,
                               GL_FALSE, (float *)&proj);

            Mat4f lightModel = TranslateMat4f(&model, lightPos);
            lightModel = ScaleMat4f(&lightModel, vec3f(.2, .2, .2));

            glUniformMatrix4fv(glGetUniformLocation(cubeProg, "model"), 1,
                               GL_FALSE, (float *)&lightModel);
        }
        glDrawArrays(GL_TRIANGLES, 0, 36);

        clock_t end = clock();

        printf("Took %f milliseconds\n", (double)(end - begin) * 1000 / CLOCKS_PER_SEC);
        totalTime += (end - begin);
        numFrames++;

        /* Final book keeping */
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    printf("took %f milliseconds on average\n", (double)totalTime / numFrames * 1000 / CLOCKS_PER_SEC);
    glfwTerminate();
    return 0;
}
