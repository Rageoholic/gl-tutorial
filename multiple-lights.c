#include "glad.h"
#include "rgl.h"

#include "rutils/debug.h"
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

#define CUBE_VERTEX_FILE "textured-lighting.vert"
#define CUBE_FRAG_FILE "multiple-lights.frag"
#define LIGHT_VERTEX_FILE "lighting-experiment-light.vert"
#define LIGHT_FRAG_FILE "lighting-experiment-light.frag"
#define TEXTURE_PATH "data/container2.png"
#define SPECULAR_PATH "data/container2_specular.png"

#define MOVE_SPEED 8

float lastX = 640;
float lastY = 360;
float yaw = -90;
float pitch = 0;
bool firstMouse = true;
float fov = 70;

typedef struct Material
{
    Vec3f ambient;
    Vec3f diffuse;
    Vec3f specular;
    float shininess;
} Material;

typedef GLuint textureID;

typedef struct TexturedMaterial
{
    textureID diffuse;
    textureID specular;
    float shininess;
} TexturedMaterial;

typedef struct Light
{
    Vec3f pos;

    Vec3f ambient;
    Vec3f diffuse;
    Vec3f specular;
    float constant;
    float linear;
    float quadratic;

} Light;

typedef struct DirectionalLight
{
    Vec3f dir;

    Vec3f ambient;
    Vec3f diffuse;
    Vec3f specular;
} DirectionalLight;

typedef struct SpotLight
{
    Vec3f pos;
    Vec3f dir;
    float cutoff;
    float outerCutoff;
    Vec3f ambient;
    Vec3f diffuse;
    Vec3f specular;
    float constant;
    float linear;
    float quadratic;
} SpotLight;

Vec3f cameraFront = (Vec3f){0.0f, 0.0f, -1.0f};
static void FramebufferResize(GLFWwindow *win, int width, int height)
{
    ignore win;
    glViewport(0, 0, width, height);
}

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

    Vec3f direction = NormalizeVec3f((Vec3f){cameraFront->x, 0, cameraFront->z});
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
        *cameraPos = AddVec3f(*cameraPos,
                              MultiplyScalarVec3f(direction,
                                                  moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    {
        *cameraPos = SubVec3f(*cameraPos,
                              MultiplyScalarVec3f(NormalizeVec3f(
                                                      CrossProductVec3f(
                                                          direction,
                                                          *cameraUp)),
                                                  moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    {
        *cameraPos = SubVec3f(*cameraPos,
                              MultiplyScalarVec3f(direction,
                                                  moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    {
        *cameraPos = AddVec3f(*cameraPos,
                              MultiplyScalarVec3f(
                                  NormalizeVec3f(CrossProductVec3f(
                                      direction,
                                      *cameraUp)),
                                  moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        *cameraPos = AddVec3f(*cameraPos,
                              MultiplyScalarVec3f(NormalizeVec3f(*cameraUp),
                                                  moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        *cameraPos = AddVec3f(*cameraPos,
                              MultiplyScalarVec3f(NormalizeVec3f(*cameraUp),
                                                  -moveSpeed));
    }

    if (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        *modelPos = SubVec3f(*modelPos,
                             MultiplyScalarVec3f(
                                 NormalizeVec3f(CrossProductVec3f(
                                     *cameraUp,
                                     direction)),
                                 moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS)
    {

        *modelPos = AddVec3f(*modelPos,
                             MultiplyScalarVec3f(
                                 NormalizeVec3f(CrossProductVec3f(
                                     *cameraUp,
                                     direction)),
                                 moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS)
    {
        *modelPos = AddVec3f(*modelPos,
                             MultiplyScalarVec3f(direction,
                                                 moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        *modelPos = SubVec3f(*modelPos,
                             MultiplyScalarVec3f(direction,
                                                 moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
    {
        *modelPos = AddVec3f(*modelPos,
                             MultiplyScalarVec3f(vec3f(0, 1, 0),
                                                 moveSpeed));
    }
    if (glfwGetKey(win, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
    {
        *modelPos = AddVec3f(*modelPos,
                             MultiplyScalarVec3f(vec3f(0, -1, 0),
                                                 moveSpeed));
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
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    Vec3f cubePositions[] = {
        vec3f(0.0f, 0.0f, 0.0f),
        vec3f(2.0f, 5.0f, -15.0f),
        vec3f(-1.5f, -2.2f, -2.5f),
        vec3f(-3.8f, -2.0f, -12.3f),
        vec3f(2.4f, -0.4f, -3.5f),
        vec3f(-1.7f, 3.0f, -7.5f),
        vec3f(1.3f, -2.0f, -2.5f),
        vec3f(1.5f, 2.0f, -2.5f),
        vec3f(1.5f, 0.2f, -1.5f),
        vec3f(-1.3f, 1.0f, -1.5f)};
    unsigned int cubeVAO;
    glGenVertexArrays(1, &cubeVAO);

    glBindVertexArray(cubeVAO);

    /* Set up vertex buffer */
    GLuint VBO;
    glGenBuffers(1, &VBO);

    /* Buffer the data */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vertices),
                 vertices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          8 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          8 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          8 * sizeof(float),
                          (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    textureID diffuseMap;
    glGenTextures(1, &diffuseMap);
    {
        int width, height, nrChannels;
        byte *textureData = stbi_load(TEXTURE_PATH, &width,
                                      &height, &nrChannels, 0);
        if (textureData)
        {
            glBindTexture(GL_TEXTURE_2D, diffuseMap);
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

    textureID specularMap;
    glGenTextures(1, &specularMap);
    {
        int width, height, nrChannels;
        byte *textureData = stbi_load(SPECULAR_PATH, &width,
                                      &height, &nrChannels, 0);
        if (textureData)
        {
            glBindTexture(GL_TEXTURE_2D, specularMap);
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

    /* Set up our shaders */
    ShaderProg cubeProg = CreateShaderProg(CUBE_VERTEX_FILE, CUBE_FRAG_FILE);
    ShaderProg lightProg = CreateShaderProg(LIGHT_VERTEX_FILE, LIGHT_FRAG_FILE);

    glEnable(GL_DEPTH_TEST);
    UseShaderProg(cubeProg);

    Vec3f objectColor = (Vec3f){1, .5, .31};

    SetUniformVec3fShaderProg(cubeProg, "objectColor", objectColor);

    TexturedMaterial cube = {0, 1, 32};

    SetUniformIntShaderProg(cubeProg, "material.diffuse", cube.diffuse);
    SetUniformIntShaderProg(cubeProg, "material.specular", cube.specular);
    SetUniformFloatShaderProg(cubeProg, "material.shininess", cube.shininess);

#ifdef PROFILING
    clock_t totalTime = 0;
    size_t numFrames = 0;
#endif

    Vec3f cameraPos = vec3f(0.0f, 0.0f, 3.0f);
    Vec3f cameraUp = vec3f(0.0f, 1.0f, 0.0f);

    Vec3f modelPos = vec3f(0, 0, 0);

    DirectionalLight dlight = {{-.2, -1, -.3},
                               {.1, .01, .01},
                               {.2, .1, .1},
                               {.4, .0, .0}};

    Light light = {{0},
                   {.4, .2, .1},
                   {.8, .2, .3},
                   {1, .3, .3},
                   1,
                   .9,
                   .032};
    Vec3f lightLocations[4] = {{1.2, 3, 4},
                               {-.3, .4, .8},
                               {-2.1, .5, -.2},
                               {-3.9, .4, .2}};

    SetUniformVec3fShaderProg(cubeProg, "dirLight.ambient", dlight.ambient);
    SetUniformVec3fShaderProg(cubeProg, "dirLight.diffuse", dlight.diffuse);
    SetUniformVec3fShaderProg(cubeProg, "dirLight.specular", dlight.specular);
    SetUniformVec3fShaderProg(cubeProg, "dirLight.dir", dlight.dir);

    /* You *can* do this better with sprintf maybe? Look into that */
    SetUniformVec3fShaderProg(cubeProg, "pointLights[0].ambient", light.ambient);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[0].diffuse", light.diffuse);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[0].specular", light.specular);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[0].pos", AddVec3f(light.pos, lightLocations[0]));
    SetUniformFloatShaderProg(cubeProg, "pointLights[0].constant", light.constant);
    SetUniformFloatShaderProg(cubeProg, "pointLights[0].linear", light.linear);
    SetUniformFloatShaderProg(cubeProg, "pointLights[0].quadratic", light.quadratic);

    SetUniformVec3fShaderProg(cubeProg, "pointLights[1].ambient", light.ambient);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[1].diffuse", light.diffuse);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[1].specular", light.specular);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[1].pos", AddVec3f(light.pos, lightLocations[1]));
    SetUniformFloatShaderProg(cubeProg, "pointLights[1].constant", light.constant);
    SetUniformFloatShaderProg(cubeProg, "pointLights[1].linear", light.linear);
    SetUniformFloatShaderProg(cubeProg, "pointLights[1].quadratic", light.quadratic);

    SetUniformVec3fShaderProg(cubeProg, "pointLights[2].ambient", light.ambient);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[2].diffuse", light.diffuse);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[2].specular", light.specular);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[2].pos", AddVec3f(light.pos, lightLocations[2]));
    SetUniformFloatShaderProg(cubeProg, "pointLights[2].constant", light.constant);
    SetUniformFloatShaderProg(cubeProg, "pointLights[2].linear", light.linear);
    SetUniformFloatShaderProg(cubeProg, "pointLights[2].quadratic", light.quadratic);

    SetUniformVec3fShaderProg(cubeProg, "pointLights[3].ambient", light.ambient);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[3].diffuse", light.diffuse);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[3].specular", light.specular);
    SetUniformVec3fShaderProg(cubeProg, "pointLights[3].pos", AddVec3f(light.pos, lightLocations[3]));
    SetUniformFloatShaderProg(cubeProg, "pointLights[3].constant", light.constant);
    SetUniformFloatShaderProg(cubeProg, "pointLights[3].linear", light.linear);
    SetUniformFloatShaderProg(cubeProg, "pointLights[3].quadratic", light.quadratic);
    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    /* Main loop */
    while (!(glfwWindowShouldClose(win)))
    {
#ifdef PROFILING
        clock_t begin = clock();
#endif
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ProcessInput(deltaTime, win,
                     &cameraUp, &cameraFront,
                     &cameraPos, &modelPos);

        /* Drawing */
        glClearColor(.1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        glBindVertexArray(cubeVAO);

        Vec3f officialCameraFront = AddVec3f(cameraPos, cameraFront);
        Mat4f view = CalcLookAtMat4f(cameraPos,
                                     AddVec3f(cameraPos, cameraFront),
                                     cameraUp);

        Mat4f proj = CreatePerspectiveMat4f(DegToRad(fov),
                                            1280. / 720.,
                                            .1,
                                            1000);

        Mat4f model = IdMat4f;
        UseShaderProg(cubeProg);

        {

            SetUniformMat4fShaderProg(cubeProg, "view", &view);
            SetUniformMat4fShaderProg(cubeProg, "projection", &proj);

            SetUniformVec3fShaderProg(cubeProg, "viewPos", cameraPos);
        }
        for (int i = 0; i < 10; i++)
        {
            Mat4f cubeModel = TranslateMat4f(&model, cubePositions[i]);
            cubeModel = TranslateMat4f(&cubeModel, modelPos);
            float angle = 13 * i;
            cubeModel = RotateMat4f(&cubeModel, DegToRad(angle), (Vec3f){1, .3, .5});

            SetUniformMat4fShaderProg(cubeProg, "model", &cubeModel);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        UseShaderProg(lightProg);
        SetUniformMat4fShaderProg(lightProg, "view", &view);
        SetUniformMat4fShaderProg(lightProg, "projection", &proj);

        glBindVertexArray(lightVAO);
        for (int i = 0; i < 4; i++)
        {
            Mat4f lightModel = TranslateMat4f(&model, AddVec3f(light.pos, lightLocations[i]));
            lightModel = ScaleMat4f(&lightModel, vec3f(.2, .2, .2));

            SetUniformMat4fShaderProg(lightProg, "model", &lightModel);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

#ifdef PROFILING
        /* TODO: profiling info wants to get rendered on screen. This shit where
	   we print stuff is *terrible* and emacs's gud support hates it */
        clock_t end = clock();

        printf("Took %f milliseconds\n", (double)(end - begin) * 1000 / CLOCKS_PER_SEC);

        totalTime += (end - begin);
        numFrames++;
#endif
        /* Final book keeping */
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
#ifdef PROFILING
    printf("took %f milliseconds on average\n",
           (double)totalTime / numFrames * 1000 / CLOCKS_PER_SEC);
#endif
    glfwTerminate();
    return 0;
}
