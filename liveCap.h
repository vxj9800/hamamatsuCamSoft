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

void startCamCap(std::ostream & out, HDCAM hdcam);

// Global variable to indicate live capture state
extern bool liveCapOn;

// Global variables to store image size
extern GLsizei liveCapImgWidth, liveCapImgHeight;

// Global variables for pan and zoom
extern double liveCapOffsetX, liveCapOffsetY, liveCapScale, liveCapWinPxPerImPx;

// Global variables to define LUT
extern GLfloat liveCapLutMin, liveCapLutMax;

// Global uniform location for LUT values
extern GLint liveCapLutMinUniformLoc, liveCapLutMaxUniformLoc;

// Global uniform location for MVP matrix
extern GLint liveCapMVPUniformLoc;