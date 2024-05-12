#include "../../conciseGl.h"
#include <stdio.h>

int getModelMatrix(mat4 dest, unsigned int shaderProgramme, vec3 trans){
    glm_mat4_identity(dest);
    //glm_rotate(dest, glm_rad((float)glfwGetTime() * 50.f), (vec3){1.0f, 0.5f, 0.0f});
    glm_translate(dest, trans);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramme, "modelMat"), 1, GL_FALSE, (float*)dest);
}

int getProjectionMatrix(mat4 dest, float fov, float screenWidth, float screenHeight, unsigned int shaderProgramme){
    glm_perspective(glm_rad(fov), (float)screenWidth / (float)screenHeight, 0.1f, 10000.f, dest);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramme, "projectionMat"), 1, GL_FALSE, (float*)dest);
}

int getViewMatrix(mat4 dest, Camera_t* camera, unsigned int shaderProgramme){
    vec3 cameraPosPlusCameraFront;
    glm_vec3_add(camera->pos, camera->front, cameraPosPlusCameraFront);
    glm_lookat(
        camera->pos,
        cameraPosPlusCameraFront,
        camera->up,
        dest);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramme, "viewMat"), 1, GL_FALSE, (float*)dest);
}