#pragma once

#define NOMINMAX
#include "dcamMisc/console4.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <vector>

// Macro to print opengl errors
#define GLCall(x)                           \
do {                                        \
    while(glGetError());                    \
    x;                                      \
    while(GLenum err = glGetError())        \
    {                                       \
        std::cout << "[OpenGL Error] "      \
                    << err << ": "          \
                    << #x << " in "         \
                    << __FILE__ << " at "   \
                    << __LINE__             \
                    << std::endl;           \
        __debugbreak();                     \
    }                                       \
} while (0)

// Global variable to indicate live capture state
extern std::atomic<bool> liveCapOn;

void setCamCapImgSize(double width, double height);
void setCamCapLUT(int min = 0, int max = 65535);
void startCamCap(std::ostream &out, HDCAM hdcam);
void stopCamCap(std::ostream &out);