#include "liveCap.h"

// Global variable to indicate live capture state
bool liveCapOn = false;

// Global variables to store image size
GLsizei liveCapImgWidth = 2048, liveCapImgHeight = 2048;

// Global variables for pan and zoom
double liveCapOffsetX = 0, liveCapOffsetY = 0, liveCapScale = 1, liveCapWinPxPerImPx = 1;

// Global variables to define LUT
GLfloat liveCapLutMin = 0, liveCapLutMax = 65535;

// Global uniform location for LUT values
GLint liveCapLutMinUniformLoc = -1, liveCapLutMaxUniformLoc = -1;

// Global uniform location for MVP matrix
GLint liveCapMVPUniformLoc = -1;

GLuint CompileShader(GLuint type, const std::string& source)
{
    GLuint id = glCreateShader(type);
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)_malloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader: " << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

GLuint CreateShader()
{
    std::string vertexShader =
        "#version 330 core\n"
        "\n"
        "layout(location = 0) in vec2 position;\n"
        "layout(location = 1) in vec2 texCoord;\n"
        "uniform mat4 u_MVP;\n"
        "\n"
        "out vec2 v_TexCoord;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = u_MVP * vec4(position,0.0,1.0);\n"
        "   v_TexCoord = texCoord;\n"
        "}\n";

    std::string fragmentShader =
        "#version 330 core\n"
        "\n"
        "out vec3 FragColor;\n"
        "in vec2 v_TexCoord;\n"
        "uniform sampler2D u_imgTex;\n"
        "uniform float u_lutMin;\n"
        "uniform float u_lutMax;\n"
        "void main()\n"
        "{\n"
        "   float colorVal = texture(u_imgTex,v_TexCoord).r;\n"
        //"   colorVal *= 600;\n"
        "   colorVal = (65535*colorVal - u_lutMin) / (u_lutMax - u_lutMin);\n"
        "   colorVal = colorVal > 1 ? 1 : colorVal < 0 ? 0 : colorVal;\n"
        "   FragColor = vec3(colorVal);\n"
        "}\n";

    GLuint program = glCreateProgram();
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

void applyMVP(float width, float height)
{
    glm::mat4 mvp = glm::ortho(-width / 2, width / 2, height / 2, -height / 2); // Create orthogonal Projection matrix
    glm::mat4 model = glm::mat4(1.0);
    model = glm::scale(model, glm::vec3(liveCapScale * liveCapWinPxPerImPx, liveCapScale * liveCapWinPxPerImPx, 1.0f));
    model = glm::translate(model, glm::vec3(liveCapOffsetX, liveCapOffsetY, 0.0f));
    mvp = mvp * model;
    if (liveCapMVPUniformLoc >= 0)
        GLCall(glUniformMatrix4fv(liveCapMVPUniformLoc, 1, GL_FALSE, &mvp[0][0]));
}

void applyLUT()
{
    static float lutMinLast = 0, lutMaxLast = 0;
    if ((lutMinLast != liveCapLutMin || lutMaxLast != liveCapLutMax) && liveCapLutMinUniformLoc >= 0 && liveCapLutMaxUniformLoc >= 0)
    {
        GLCall(glUniform1f(liveCapLutMinUniformLoc, liveCapLutMin));
        GLCall(glUniform1f(liveCapLutMaxUniformLoc, liveCapLutMax));
        lutMinLast = liveCapLutMin; lutMaxLast = liveCapLutMax;
    }
}

void handleWindowResize(GLFWwindow* window, int width, int height)
{
    double liveCapImgAspRatio = liveCapImgWidth / (float)liveCapImgHeight;
    double liveCapWinAspRatio = width / (float)height;
    if (liveCapImgAspRatio > liveCapWinAspRatio)
        liveCapWinPxPerImPx = width / (float)liveCapImgWidth;
    else
        liveCapWinPxPerImPx = height / (float)liveCapImgHeight;
    liveCapOffsetX = 0; liveCapOffsetY = 0; liveCapScale = 1;

    applyMVP((float)width, (float)height);

    GLCall(glViewport(0, 0, width, height));
}

void handleCursorPosition(GLFWwindow* window, double xpos, double ypos)
{
    static bool buttonPressed = false;
    static double dragX = 0, dragY = 0;

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    //ypos = height - ypos;

    if (!buttonPressed && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        dragX = xpos; dragY = ypos;
        buttonPressed = true;
    }
    else if (buttonPressed && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        liveCapOffsetX += (xpos - dragX) / liveCapScale / liveCapWinPxPerImPx;
        liveCapOffsetY += (ypos - dragY) / liveCapScale / liveCapWinPxPerImPx;
        applyMVP((float)width, (float)height);
        dragX = xpos; dragY = ypos;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        dragX = 0; dragY = 0;
        buttonPressed = false;
    }
}

void handleMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    //ypos = height - ypos;

    glm::mat4 projection = glm::ortho(-(float)width / 2, (float)width / 2, -(float)height / 2, (float)height / 2); // Create orthogonal Projection matrix
    glm::vec3 imgPx = glm::unProject(glm::vec3(xpos, ypos, 0.0f), glm::mat4(1.0), projection, glm::vec4(0, 0, width, height));

    liveCapOffsetX += -imgPx.x / liveCapScale / liveCapWinPxPerImPx;
    liveCapOffsetY += -imgPx.y / liveCapScale / liveCapWinPxPerImPx;
    liveCapScale += 0.1 * liveCapScale * yoffset;
    liveCapScale = liveCapScale < 0.1 ? 0.1 : liveCapScale;
    liveCapScale = liveCapScale > 100 ? 100 : liveCapScale;
    liveCapOffsetX += imgPx.x / liveCapScale / liveCapWinPxPerImPx;
    liveCapOffsetY += imgPx.y / liveCapScale / liveCapWinPxPerImPx;

    applyMVP((float)width, (float)height);
}

void handleWindowClose(GLFWwindow* window)
{
    glfwSetWindowShouldClose(window, GLFW_FALSE);
}

void handleKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE)
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        liveCapOffsetX = 0; liveCapOffsetY = 0; liveCapScale = 1;
        applyMVP((float)width, (float)height);
        GLCall(glViewport(0, 0, width, height));
    }

}

void liveCapShow(std::ostream& out, HDCAM hdcam)
{
    DCAMERR camErr;

    // prepare frame param
    DCAMBUF_FRAME bufframe;
    memset(&bufframe, 0, sizeof(bufframe));
    bufframe.size = sizeof(bufframe);
    bufframe.iFrame = 0;

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        out << "GLFW initialization failed." << std::endl << "otSoft> ";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Get Monitor information to set correct window size
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    int liveCapWindowWidth = (int)(mode->width / 1.5);
    int liveCapWindowHeight = (int)(mode->height / 1.5);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(liveCapWindowWidth, liveCapWindowHeight, "liveCam", NULL, NULL);
    if (!window)
    {
        out << "GLFW window creation failed." << std::endl;
        glfwTerminate();
        return;
    }

    // Set necessary callbacks
    glfwSetWindowSizeCallback(window, handleWindowResize);
    glfwSetCursorPosCallback(window, handleCursorPosition);
    glfwSetScrollCallback(window, handleMouseScroll);
    glfwSetWindowCloseCallback(window, handleWindowClose);
    glfwSetKeyCallback(window, handleKeyPress);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);


    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        out << "GLEW initialization failed." << std::endl;
        out << "Error: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return;
    }

    // Dark blue background
    glClearColor(0.251f, 0.259f, 0.345f, 1.0f);

    float vertices[16] = {
       -(liveCapImgWidth / 2.0f), -(liveCapImgHeight / 2.0f), 0.0f, 0.0f,
        (liveCapImgWidth / 2.0f), -(liveCapImgHeight / 2.0f), 1.0f, 0.0f,
        (liveCapImgWidth / 2.0f),  (liveCapImgHeight / 2.0f), 1.0f, 1.0f,
       -(liveCapImgWidth / 2.0f),  (liveCapImgHeight / 2.0f), 0.0f, 1.0f
    };

    GLuint indices[16] = {
       0, 1, 2,
       2, 3, 0
    };

    GLuint vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    GLuint vbo;
    GLCall(glGenBuffers(1, &vbo));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, vertices, GL_STATIC_DRAW));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0));
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));

    GLuint ibo;
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, indices, GL_STATIC_DRAW));

    GLuint shader = CreateShader();
    GLCall(glUseProgram(shader));

    /* OpenGL texture binding of the image from the camera  */
    GLuint imgTexID;
    GLCall(glGenTextures(1, &imgTexID)); /* Texture name generation */
    GLCall(glActiveTexture(GL_TEXTURE0)); // Activate texture unit 0 to store the image
    GLCall(glBindTexture(GL_TEXTURE_2D, imgTexID)); /* Binding of texture name */
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)); /* We will use linear interpolation for magnification filter */
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)); /* We will use linear interpolation for minifying filter */
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, liveCapImgWidth, liveCapImgHeight, 0, GL_RED, GL_UNSIGNED_SHORT, bufframe.buf));

    // Get uniform location to update MVP matrix
    liveCapMVPUniformLoc = glGetUniformLocation(shader, "u_MVP");

    // Get uniform location to update lutMin and lutMax
    liveCapLutMinUniformLoc = glGetUniformLocation(shader, "u_lutMin");
    liveCapLutMaxUniformLoc = glGetUniformLocation(shader, "u_lutMax");
    GLCall(glUniform1f(liveCapLutMinUniformLoc, liveCapLutMin));
    GLCall(glUniform1f(liveCapLutMaxUniformLoc, liveCapLutMax));

    // Call window resize function to correctly initialize MVP
    handleWindowResize(window, liveCapWindowWidth, liveCapWindowHeight);

    /* Loop until the user closes the window */
    while (liveCapOn)
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        // Update the texture
        // access image
        camErr = dcambuf_lockframe(hdcam, &bufframe);
        if (!failed(camErr) && bufframe.type == DCAM_PIXELTYPE_MONO16)
            GLCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, liveCapImgWidth, liveCapImgHeight, GL_RED, GL_UNSIGNED_SHORT, bufframe.buf));
        else
            out << std::endl << "Error locking the camera frame." << std::endl;

        // Update LUT
        applyLUT();

        // Draw the triangles
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // Cleanup VBO
    GLCall(glDeleteTextures(1, &imgTexID));
    GLCall(glDeleteBuffers(1, &vbo));
    GLCall(glDeleteBuffers(1, &ibo));
    GLCall(glDeleteVertexArrays(1, &vao));
    GLCall(glDeleteProgram(shader));

    glfwTerminate();
    return;
}

void startCamCap(std::ostream& out, HDCAM hdcam)
{
    DCAMERR err;

    // open wait handle
    DCAMWAIT_OPEN	waitopen;
    memset(&waitopen, 0, sizeof(waitopen));
    waitopen.size = sizeof(waitopen);
    waitopen.hdcam = hdcam;

    err = dcamwait_open(&waitopen);
    if (failed(err))
    {
        out << "Could not create DCAMWAIT_OPEN object." << std::endl;
        return;
    }
    else
    {
        HDCAMWAIT hwait = waitopen.hwait;
        
        // allocate buffer
        int32 number_of_buffer = 1;
        err = dcambuf_alloc(hdcam, number_of_buffer);
        if (failed(err))
        {
            out << "Could not allocate frame buffer in the DCAM module." << std::endl;
            return;
        }
        else
        {
            // start capture
            err = dcamcap_start(hdcam, DCAMCAP_START_SEQUENCE);
            if (failed(err))
            {
                out << "Could not start camera capture." << std::endl;
                return;
            }
            else
            {
                // set wait param
                DCAMWAIT_START waitstart;
                memset(&waitstart, 0, sizeof(waitstart));
                waitstart.size = sizeof(waitstart);
                waitstart.eventmask = DCAMWAIT_CAPEVENT_FRAMEREADY;
                waitstart.timeout = 11000;

                // wait image
                err = dcamwait_start(hwait, &waitstart);
                if (failed(err))
                {
                    out << "Could not detect frame ready event." << std::endl;
                    return;
                }

                // Start the live feed
                liveCapShow(out, hdcam);
                liveCapOn = false;

                // stop capture
                dcamcap_stop(hdcam);
            }

            // release buffer
            dcambuf_release(hdcam);
        }

        // close wait handle
        dcamwait_close(hwait);
    }
}