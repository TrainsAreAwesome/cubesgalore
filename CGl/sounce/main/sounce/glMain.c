// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
#include "../../../conciseGl.h"
#include <stdio.h>
#include <stdlib.h>

//callback definitions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

//default callback for scroll and mouse pos events.
//if this isnt set, the programme just segfaults
void doNothing(double, double){
    return;
}

//user defined callbacks
void (*mouseCallbackFun)(double, double) = doNothing;
void (*scrollCallbackFun)(double, double) = doNothing;

//callbacks that call the user defined callbacks (yes really)
void mouseCall(GLFWwindow* window, double xPos, double yPos);
void scrollCall(GLFWwindow* window, double xOffset, double yOffset);


int updatedH, updatedW; //these are used to get info from the glfw callback funcs to the CGl window

int initCGl(CGlWindow_t* window, int startW, int startH, char* title, int glMajor, int glMinor){
    //set width and height
    window->h = startH;
    window->w = startW;
    //the starting w and h are also the latest so set these
    updatedH = startH;
    updatedW = startW;
    window->deltaTime = 0.0f;
    window->lastFrame = 0.0f;
    window->wireframeMode = 0;
    window->firstMouse = true;
    window->lastX = startW / 2;
    window->lastY = startH / 2;

    //init glfw
    if(!glfwInit()){
        printf("GLFW Initilisation failed :-(\n");
        return 1;
    }

    //set opengl version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //create the glfw window
    window->GLFWWindow = glfwCreateWindow(startW, startH, title, NULL, NULL);
    if(!window->GLFWWindow){
        printf("GLFW Window is false :-(\n");
        glfwTerminate();
        return 1;
    }


    glfwMakeContextCurrent(window->GLFWWindow);
    
    //set callbacks
    glfwSetFramebufferSizeCallback(window->GLFWWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(window->GLFWWindow, mouseCall);
    glfwSetScrollCallback(window->GLFWWindow, scrollCall);


    //load glad
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        printf("GLAD failed to load :-(\n");
        return 1;
    }
    glEnable(GL_DEPTH_TEST);
    printf("Initialised CGl\n");
}

//use this to know when to close your window
int CGlWindowOpen(CGlWindow_t* window){
    return !glfwWindowShouldClose(window->GLFWWindow);
}

int CGlRefreshWindow(CGlWindow_t* window, vec3 colour){
    float currentFrame = glfwGetTime();
    glClearColor(colour[0], colour[1], colour[2], 1.0f);
    if(!CGlWindowOpen(window)){
        return 0;
    }
    window->height = updatedH;
    window->width = updatedW;

    window->deltaTime = currentFrame - window->lastFrame;
    window->lastFrame = currentFrame;
    window->FPS = 1 / window->deltaTime;

    glfwSwapBuffers(window->GLFWWindow);
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(window->wireframeMode){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    return 1;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
    updatedH = height;
    updatedW = width;
}

void mouseCall(GLFWwindow* window, double xPos, double yPos){
    mouseCallbackFun(xPos, yPos);
}

void scrollCall(GLFWwindow* window, double xOffset, double yOffset){
    scrollCallbackFun(xOffset, yOffset);
}

int CGlSetScrollCallback(CGlWindow_t* window, void(*function)(double, double)){
    scrollCallbackFun = function;
}

int CGlSetMousePosCallback(CGlWindow_t* window, void(*function)(double, double)){
    mouseCallbackFun = function;
}

//sets some values in a given camera pointer to set up a camera
int CGlInitGameCamera(Camera_t* camera, float fov){
    camera->yaw = -90.f;
    camera->pitch = 0.f;
    camera->fov = fov;

    camera->pos[0] = 0.0f;
    camera->pos[1] = 0.0f;
    camera->pos[2] = 0.0f;

    camera->front[0] = 0.0f;
    camera->front[1] = 0.0f;
    camera->front[2] = -1.0f;

    camera->up[0] = 0.0f;
    camera->up[1] = 1.0f;
    camera->up[2] = 0.0f;
}