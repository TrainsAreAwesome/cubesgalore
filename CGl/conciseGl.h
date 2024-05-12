#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "sounce/cglm/include/cglm/cglm.h"
#include "sounce/loading/include/loadShaders.h"
#include "sounce/loading/include/loadTexture.h"
#include "sounce/loading/include/stb_image.h"

//struct that represents a window
typedef struct {
    //width of window
    union {
        int width;
        int w;
    };
    //height of window
    union {
        int height;
        int h;
    };
    float FPS;
    float lastFrame;
    float deltaTime;
    GLFWwindow* GLFWWindow;
    int wireframeMode;
    int firstMouse;
    float lastX;
    float lastY;
} CGlWindow_t;

typedef struct {
    vec3 pos;
    vec3 front;
    vec3 up;
    float yaw;
    float pitch;
    float fov;
    ivec3 playerChunkPos;
    ivec3 oldPlayerChunkPos;
    ivec3 posInt;
} Camera_t;

//Used to initialise your window. First make a CGlWindow_t and pass a pointer to it here.
//Then add the starting width and height, the window title, and the OpenGL version you want to use.
//This function returns 0 on sucsess and 1 on failure.
int initCGl(CGlWindow_t* window, int startW, int startH, char* title, int glMajor, int glMinor);

//Used to check weather your window should still be open.
//Returns 1 if window should stay open
//Returns 0 if window should close
int CGlWindowOpen(CGlWindow_t* window);

//Refresh a CGl window. Pass in a pointer to the window and a colour
//to clear the window to in the format of a CGLM vec3 type with float values between 0 and 1.
//If the window should close this function will return 0. Otherwise it returns 1.
int CGlRefreshWindow(CGlWindow_t* window, vec3 colour);

//If you want to take scroll wheel input, make a function that accepts two doubles.
//Then call this function, pass in the window, and a pointer to that function.
//Whenever next the scroll wheel is used the function you set will be called
int CGlSetScrollCallback(CGlWindow_t* window, void(*function)(double, double));

//If you want to take mouse pos input, make a function that accepts two doubles.
//Then call this function, pass in the window, and a pointer to that function.
//Whenever next the mouse pos is updated the function you set will be called
int CGlSetMousePosCallback(CGlWindow_t* window, void(*function)(double, double));

//sets some values in a given camera pointer to set up a camera
int CGlInitGameCamera(Camera_t* camera, float fov);