#pragma once
#include "../../conciseGl.h"
int getModelMatrix(mat4 dest, unsigned int shaderProgramme, vec3 trans);
int getProjectionMatrix(mat4 dest, float fov, float screenWidth, float screenHeight, unsigned int shaderProgramme);
int getViewMatrix(mat4 dest, Camera_t* camera, unsigned int shaderProgramme);