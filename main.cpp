#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <stb/stb_image.h>

#include <iostream>
#include <string>
#include <cmath>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Shader program: vertex -> framgent
// varying variables are common for all program, so it's data is transfered from vertex to fragment shader and interpolated with perspective correction
// attribute variables are variables gotten from buffers and passed into shader program in order of vertex attribute array parameters
// uniform variables are transfered by cpu program, has fixed value for triangle and can be used in vertex and fragment shaders
// unifrom variables are accessed by cpu program by index glGetUniformLocation(GLuint program, String "variable") glUniformType(GLuint uniform, values ...) (Type: 2f, 3f, 4f, 1i)

const char* vertexShaderSource = "\n"
"#version 120                                       \n"
"attribute vec3 position;                           \n"
"attribute vec3 color;                              \n"
"attribute vec2 texcoord;                           \n"
"varying vec3 fragcolor;                            \n"
"varying vec2 texcoords;                            \n"
"uniform mat4 model;                                \n"
"uniform mat4 view;                                 \n"
"uniform mat4 proj;                                 \n"
"void main()                                        \n"
"{                                                  \n"
"   gl_Position = proj * view * model * vec4(position, 1.0f);   \n"
"   // gl_Position = vec4(position, 1.0f);          \n"
"   fragcolor = color;                              \n"
"   texcoords = texcoord;                           \n"
"}                                                  \0";

const char* fragmentShaderSource = "\n"
"#version 120                                       \n"
"uniform vec4 brightness;                           \n"
"uniform sampler2D tex;                             \n"
"varying vec3 fragcolor;                            \n"
"varying vec2 texcoords;                            \n"
"void main()                                        \n"
"{                                                  \n"
"   gl_FragColor = texture2D(tex, texcoords) * vec4(fragcolor, 1.0f);           \n"
"}                                                  \0";

float vertices[] =
{
    0.0f,   0.5f,   0.0f,   1.0f,   1.0f,   0.0f,   0.5f,   1.0f,
    -0.5f,  -0.5f,  0.5f,   1.0f,   0.0f,   1.0f,   0.0f,   0.0f,
    0.5f,   -0.5f,  0.5f,   0.0f,   1.0f,   1.0f,   1.0f,   0.0f,

    0.0f,   0.5f,   0.0f,   1.0f,   1.0f,   0.0f,   0.5f,   1.0f,
    -0.5f,  -0.5f,  -0.5f,   1.0f,   0.0f,   1.0f,   0.0f,   0.0f,
    -0.5f,   -0.5f,  0.5f,   0.0f,   1.0f,   1.0f,   1.0f,   0.0f,

    0.0f,   0.5f,   0.0f,   1.0f,   1.0f,   0.0f,   0.5f,   1.0f,
    0.5f,  -0.5f,  -0.5f,   1.0f,   0.0f,   1.0f,   0.0f,   0.0f,
    -0.5f,   -0.5f,  -0.5f,   0.0f,   1.0f,   1.0f,   1.0f,   0.0f,

    0.0f,   0.5f,   0.0f,   1.0f,   1.0f,   0.0f,   0.5f,   1.0f,
    0.5f,  -0.5f,  0.5f,   1.0f,   0.0f,   1.0f,   0.0f,   0.0f,
    0.5f,   -0.5f,  -0.5f,   0.0f,   1.0f,   1.0f,   1.0f,   0.0f,
};

unsigned int elements[] = {
    0, 1, 2,
    3, 4, 5,
    6, 7, 8,
    9, 10, 11
};

void framebufferSizeCallBack(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {} 

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {}

int main(void)
{

    if (!glfwInit()) return -1;

    // Create GLFW Window and make context

    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        "OpenGL Floppa pyramid",
        NULL,
        NULL);

    if (!window)
    {
        std::cout << "Creating GLFWWindow Failed!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallBack);

    // Init GLEW to get OpenGL functions

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW Initialization Failed!" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL Version:\t\t" << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version:\t\t" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "GPU Vendor:\t\t" << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GPU Renderer:\t\t" << glGetString(GL_RENDERER) << std::endl;

    // Here check for GL_ARB_vertex_array_object support by glewIsSupported("GL_ARB_vertex_array_object")
    if (!glewIsSupported("GL_ARB_vertex_array_object"))
    {
        std::cout << "GL_ARB_vertex_array_object UNSUPPORTED. CHECK THE VIDEO DRIVER!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Create framebuffer with size of window, enable SRGB pixel format and set clear color

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glClearColor(0.2, 0.2, 0.2, 1.0);

    // Loading a single triangle with vertices and colors attributes into VRAM
    // Drawing without using any shaders
    // All attributes are interpolated and drawn by default shader

    // BUFFER OBJECTS

    int vertexCount = sizeof(vertices) / sizeof(float);
    int elementCount = sizeof(elements) / sizeof(float);

    std::cout << vertexCount << std::endl;
    std::cout << elementCount << std::endl;

    GLuint vLocation = 0;
    GLuint cLocation = 1;
    GLuint tLocation = 2;
    GLuint nLocation = 3;

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * elementCount, elements, GL_STATIC_DRAW);

    GLuint vbo;

    if (vertices != nullptr) {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Compiling shaders

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint compileStatus;
    GLchar message[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        glGetShaderInfoLog(vertexShader, 512, NULL, message);
        std::cout << message << std::endl;
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, message);
        std::cout << message << std::endl;
    }

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &compileStatus);
    if (!compileStatus) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, message);
        std::cout << message << std::endl;
    }

    // Applying textures into the mesh

    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char* texturedata = stbi_load("./assets/floppa.png", &width, &height, &channels, 0);

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (texturedata != nullptr) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texturedata);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        // Avtor mudak
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(texturedata);

    // Main loop while no GLFWWindowShouldClose event sent 

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    int brightnessLocation = glGetUniformLocation(shaderProgram, "brightness");
    int texLocation = glGetUniformLocation(shaderProgram, "tex");
    glUseProgram(shaderProgram);
    glUniform1i(texLocation, 0);

    int modelLocation = glGetUniformLocation(shaderProgram, "model");
    int viewLocation = glGetUniformLocation(shaderProgram, "view");
    int projLocation = glGetUniformLocation(shaderProgram, "proj");

    float rotation = 0.0f;
    float prevtime = glfwGetTime();

    // Position of camera is shifted on x axis by -2
    glm::vec3 position = glm::vec3(-2.0f, 0.0f, 0.0f);
    // Orientation of camera
    glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 1.0f);
    // Up vector for camera logic
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float prevx = 0;
    float prevy = 0;
    float yaw = 0.0f;
    float pitch = 0.0f;

    float cameraSpeed = 1.0f;
    float mouseSensitivity = 15.0f;
    bool firstMouseFrame = true;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        // FPS on titlebar text

        float crnttime = glfwGetTime();
        float deltaTime = crnttime - prevtime;
        prevtime = crnttime;

        std::string fps = std::to_string(1.0f / deltaTime);
        std::string dt = std::to_string(deltaTime);
        std::string title = "FPS: " + fps + " | dt: " + dt;

        glfwSetWindowTitle(window, title.c_str());

        // Events polling
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // Transform matrices    

        rotation += 30.0 * deltaTime;

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);

        // Camera input handling

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position += cameraSpeed * orientation * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position += cameraSpeed * -orientation * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position += cameraSpeed * -glm::normalize(glm::cross(orientation, up)) * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position += cameraSpeed * glm::normalize(glm::cross(orientation, up)) * deltaTime;

        // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

        double x;
        double y;
        glfwGetCursorPos(window, &x, &y);

        if (firstMouseFrame) {
            prevx = x;
            prevy = y;
            firstMouseFrame = false;
        }

        float xoffset = x - prevx;
        float yoffset = prevy - y;
        prevx = x;
        prevy = y;

        xoffset *= mouseSensitivity * deltaTime;
        yoffset *= mouseSensitivity * deltaTime;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.9f) pitch = 89.9f;
        if (pitch < -89.9f) pitch = -89.9f;

        glm::vec3 direction = glm::vec3(
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
        orientation = glm::normalize(direction);

        // } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        //     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);        
        // }
        
        // Little changes into mdoel matrix for demo
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        // Temporary code line. Note that Z+ is toward us
        view = glm::lookAt(position, position + orientation, up);
        // Make flexible for aspect ratios with different screen sizes
        proj = glm::perspective(glm::radians(75.0f), (float)(SCREEN_WIDTH/SCREEN_HEIGHT), 0.1f, 100.0f);

        // DRAW CALL USING VAO

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, texture);

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(proj));
        // glUniformMatrix4fw(matrixLocation, 1, GL_FALSE, glm::value_ptr(proj * view));

        // TEST UNIFORM VARIABLE IN SHADER
        // float brightness = (sin(glfwGetTime() * 3.1415927) / 2.0) + 0.5;
        // glUniform4f(brightnessLocation, brightness, brightness, brightness, 1.0);

        glDrawElements(GL_TRIANGLES, sizeof(elements) / sizeof(unsigned int), GL_UNSIGNED_INT, NULL);
        
        glBindVertexArray(0);
        glUseProgram(0);
    
        glfwSwapBuffers(window);
    }

    // DELETE ALL BUFFERS BEFORE TERMINATION

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
