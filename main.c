#include <stdio.h>
#include "CGl/conciseGl.h"
#include "./CGl/other/header/matricies.h"
#include <omp.h>
#include "./game/header/chunk.h"
#include "./game/header/terrainGen.h"
#include "./game/header/world.h"

#define START_WIDTH 1920
#define START_HEIGHT 1080

#define MAIN_THREAD 0
#define AMOUNT_WORKER_THREADS 3 //dont just put this really high, it lowers fps as they "idle" by just waiting in an infinite, memory intensive loop

CGlWindow_t window;
Camera_t camera;

world loadedWorld;

fullChunk* chunksToFree[LOADED_CHUNKS_X][LOADED_CHUNKS_Y][LOADED_CHUNKS_Z];

volatile static int haltThreads = 0; //if i dont define these as volatile compiler optimisations break the programme
volatile int workerThreadsActive[AMOUNT_WORKER_THREADS];

volatile int anyWorkerThreadActive;

//declareations for callback functions
void mousePosCallback(double xPos, double yPos);
void scrollCallback(double xOffset, double yOffset);
void handleKeyboardInput(CGlWindow_t* window);

int main(int argc, char** argv){
    
    if(initCGl(&window, START_WIDTH, START_HEIGHT, "MWGIES TOOLS!!!", 3, 3)){
        printf("Something went wrong initialising CGl");
        return 1;
    }

    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int z = 0;


    loadedWorld.offset[0] = 0;
    loadedWorld.offset[1] = 0;
    loadedWorld.offset[2] = 0;

    //locking mouse
    glfwSetInputMode(window.GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(0); //uncomment this line to turn off vsync

    CGlSetMousePosCallback(&window, mousePosCallback);
    CGlSetScrollCallback(&window, scrollCallback);

    unsigned int shaderProgramme = useLoadShaders("./shaders/newVS.glsl", "./shaders/fragmentShader.glsl");
    unsigned int VAO, VBO, EBO, texture;

    CGlInitGameCamera(&camera, 90.f);

    loadTexturePNG(&texture, "./terrainCopy.png");
    glUniform1i(glGetUniformLocation(shaderProgramme, "texture"), 0);
    printf("Loaded terrain.png\n");
    glBindTexture(GL_TEXTURE_2D, texture);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    //NEW FORMAT (LEFT TO RIGHT):
    //uInt (U CORD AS INT 0-15, 4 BITS) SHIFT 24
    //vInt (V CORD AS INT 0-15, 4 BITS) SHIFT 20
    //lightInt (LIGHT VALUE AS INT 0-3, 2 BITS) SHIFT 18
    //BX (CHUNK LOCAL X CORD AS INT, 0-31, 5 BITS) SHIFT 12
    //BY (CHUNK LOCAL Y CORD AS INT, 0-31, 5 BITS) SHIFT 6
    //BZ (CHUNK LOCAL Z CORD AS INT, 0-31, 5 BITS) NO SHIFT

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned int), (void*)0);
    glEnableVertexAttribArray(0);

    loadedWorld.maxX = LOADED_CHUNKS_X;
    loadedWorld.maxY = LOADED_CHUNKS_Y;
    loadedWorld.maxZ = LOADED_CHUNKS_Z;

    loadedWorld.halfMaxX / loadedWorld.maxX / 2;
    loadedWorld.halfMaxY / loadedWorld.maxY / 2;
    loadedWorld.halfMaxZ / loadedWorld.maxZ / 2;

    printf("maxX %d maxY %d maxZ %d\n", loadedWorld.maxX, loadedWorld.maxY, loadedWorld.maxZ);
    // loadedWorld.chunks = initWorld(loadedWorld.maxX, loadedWorld.maxY, loadedWorld.maxZ); //loading world data

    initTerrainGen(&loadedWorld, 12345);

    // glEnable(GL_CULL_FACE);
    
    #pragma omp parallel for
    for(int x = 0; x < loadedWorld.maxX; ++x){
        #pragma omp parallel for
        for(int y = 0; y < loadedWorld.maxY; ++y){
            #pragma omp parallel for
            for(int z = 0; z < loadedWorld.maxZ; ++z){
                loadedWorld.chunks[x][y][z] = calloc(1, sizeof(fullChunk));
                if(loadedWorld.chunks[x][y][z] == NULL){
                    printf("out of memory!\n");
                    quick_exit(1);
                }
                
                // printf("allocating memory for chunk %d %d %d\n", x, y, z);
                chunksToFree[x][y][z] = NULL;
            }
        }
    }    

    glUseProgram(shaderProgramme);

    //initialising the array to NULL
    for(int x = 0; x < loadedWorld.maxX; ++x){
        for(int y = 0; y < loadedWorld.maxY; ++y){
            for(int z = 0; z < loadedWorld.maxZ; ++z){
                loadedWorld.chunks[x][y][z]->rawMesh = NULL;
                loadedWorld.chunks[x][y][z]->data.needsRemesh = 1;
            }
        }
    }

    GLenum error, oldError;

    #pragma omp parallel num_threads(AMOUNT_WORKER_THREADS + 1)
    {
        _Thread_local static int threadID;
        threadID = omp_get_thread_num();

        if(omp_get_num_threads() != AMOUNT_WORKER_THREADS + 1){
            printf("Coulnt start all therads! Exiting...\n");
            quick_exit(1);
        }

        //MAIN THREAD STUFF
        if(threadID == MAIN_THREAD){
            printf("Hello from Main Thread %d\n", threadID);



            GLint chunkposLocation = glGetUniformLocation(shaderProgramme, "chunkPos");
            unsigned int chunkpos[3];
            mat4 modelMatrix, projectionMatrix, viewMatrix;
            int px, py, pz, rx, ry, rz;
            int amountMeshUpdates = 0;

            while (CGlRefreshWindow(&window, (vec3){0.0f, 0.6f, 0.7})){
                // printf("Threads halted? %d\n", haltThreads);
                handleKeyboardInput(&window);
        
                getTargetedBlock(&camera);

                getViewMatrix(viewMatrix, &camera, shaderProgramme);
                getProjectionMatrix(projectionMatrix, camera.fov, window.width, window.height, shaderProgramme);

                getModelMatrix(modelMatrix, shaderProgramme, (vec3){0.0f, 0.0f, 0.0f});

                amountMeshUpdates = 0;

                if(loadedWorld.needsUpdate){
                    haltThreads = 1;

                    do {
                        anyWorkerThreadActive = 0;
                        for(int i = 0; i < AMOUNT_WORKER_THREADS; ++i){
                            anyWorkerThreadActive |= workerThreadsActive[i];
                        }
                    } while(anyWorkerThreadActive);

                    printf("about to update world\n");
                    ivec3 offset;
                    glm_ivec3_sub(loadedWorld.offset, loadedWorld.oldOffset, offset);
                    updateWorld(&loadedWorld, offset);
                    loadedWorld.needsUpdate = 0;
                    haltThreads = 0;
                }

                for(int x = 0; x < loadedWorld.maxX; ++x){
                    for(int y = 0; y < loadedWorld.maxY; ++y){
                        for(int z = 0; z < loadedWorld.maxZ; ++z){

                            //if there is a chunk to free, free it (has to be done on main thread otherwise there will be a vram leak)
                            if(chunksToFree[x][y][z] != NULL){
                                // printf("\n\nfreeing chunk %d %d %d\n\n\n", x, y, z);
                                freeChunk(chunksToFree[x][y][z]);
                                chunksToFree[x][y][z] = NULL;
                            }

                            //if a new mesh has been generated
                            if(!loadedWorld.chunks[x][y][z]->data.needsRemesh && loadedWorld.chunks[x][y][z]->rawMesh != NULL && amountMeshUpdates <= 20){
                                while(loadedWorld.chunks[x][y][z]->busy){
                                    //wait for chunk to be avalable
                                }
                                loadedWorld.chunks[x][y][z]->busy = 1;
                                // printf("putting l %d %d %d into vram on main thread\n", x, y, z);
                                copyMeshIntoVRAM(x, y, z, shaderProgramme, &loadedWorld, loadedWorld.chunks[x][y][z]->rawMesh);
                                // printf("copyied into vram now freeing\n");
                                free(loadedWorld.chunks[x][y][z]->rawMesh);
                                loadedWorld.chunks[x][y][z]->rawMesh = NULL;
                                // printf("freed the mesh and set it to NULL\n");
                                ++amountMeshUpdates;
                                loadedWorld.chunks[x][y][z]->busy = 0;
                            }

                            //if the chunk contains nothing, then dont render
                            if(!loadedWorld.chunks[x][y][z]->mesh.countVerticies || !loadedWorld.chunks[x][y][z]->data.isGenerated){
                                continue;
                            }
                            glBindVertexArray(loadedWorld.chunks[x][y][z]->mesh.vao);
                            glBindBuffer(GL_ARRAY_BUFFER, loadedWorld.chunks[x][y][z]->mesh.vbo);
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, loadedWorld.chunks[x][y][z]->mesh.ebo);
                            //binding to the correct chunks data

                            //sending chunk pos data to the gpu
                            chunkpos[0] = loadedWorld.chunks[x][y][z]->data.x;
                            chunkpos[1] = loadedWorld.chunks[x][y][z]->data.y;
                            chunkpos[2] = loadedWorld.chunks[x][y][z]->data.z;
                            // glUniform3iv(glGetUniformLocation(shaderProgramme, "chunkPos"), 1, chunkpos);
                            glUniform3iv(chunkposLocation, 1, chunkpos);

                            //drawing
                            glDrawElements(GL_TRIANGLES, loadedWorld.chunks[x][y][z]->mesh.countIndicies, GL_UNSIGNED_INT, 0);
                            oldError = error;
                            error = glGetError();
                            if(oldError != error && error != 0){
                                printf("Error while rendering chunk l %d %d %d VBO: %u VAO: %u EBO: %u ERROR: %u\n", x, y, z, loadedWorld.chunks[x][y][z]->mesh.vbo, loadedWorld.chunks[x][y][z]->mesh.vao, loadedWorld.chunks[x][y][z]->mesh.ebo, error);
                                if(error == GL_OUT_OF_MEMORY){
                                    printf("vcount %d icount %d\n", loadedWorld.chunks[x][y][z]->mesh.countVerticies, loadedWorld.chunks[x][y][z]->mesh.countIndicies);
                                    quick_exit(90);
                                }
                            }
                            // printf("vbo: %u vao: %u ebo: %u\n", loadedWorld.chunks[x][y][z]->mesh.vbo, loadedWorld.chunks[x][y][z]->mesh.vao, loadedWorld.chunks[x][y][z]->mesh.ebo);
                                
                            
                            // printf("vcount %d icount %d\n", loadedWorld.chunks[x][y][z]->mesh.countVerticies, loadedWorld.chunks[x][y][z]->mesh.countIndicies);
                        }
                    }
                }
                px = (int)round(camera.pos[0]);
                py = (int)round(camera.pos[1]);
                pz = (int)round(camera.pos[2]);
                worldToChunkPos(px, py, pz, &rx, &ry, &rz);
                printf("FPS: %f POS: %f %f %f CHUNKPOS: %d %d %d INTPOS: %d %d %d\n", window.FPS, camera.pos[0], camera.pos[1], camera.pos[2], rx, ry, rz, px, py, pz);
            }
            quick_exit(0);
        }
        //END OF MAIN THERAD STUFF


        //WORKER THREAD STUFF
        if(threadID != MAIN_THREAD){
            while(1){
                for(int x = 0; x < loadedWorld.maxX; ++x){
                    for(int y = 0; y < loadedWorld.maxY; ++y){
                        for(int z = 0; z < loadedWorld.maxZ; ++z){
                            
                            workerThreadsActive[threadID] = 0;

                            wait:
                            if(haltThreads){
                                goto wait;
                            }
                            workerThreadsActive[threadID] = 1;

                            if(!loadedWorld.chunks[x][y][z]->data.isGenerated && !loadedWorld.chunks[x][y][z]->busy){
                                tGenTheadFunction(&loadedWorld, x, y, z);
                                // printf("Generating chunk %d %d %d on thread %d\n", x, y, z, threadID);
                            } 
                            if(loadedWorld.chunks[x][y][z]->data.needsRemesh && !loadedWorld.chunks[x][y][z]->busy && loadedWorld.chunks[x][y][z]->data.isGenerated){
                                meshThreadFunction(&loadedWorld, x, y, z);
                                // printf("Meshing chunk %d %d %d on thread %d\n", x, y, z, threadID);
                            }
                        }   
                    }
                }
            }
        }
        printf("Hi from thread %d Total threads: %d\n", threadID, omp_get_num_threads());
        quick_exit(0);
    }
}

void handleKeyboardInput(CGlWindow_t* window){

    // waitKeyboardInput:
    // if(haltThreads){
    //     goto waitKeyboardInput;
    //     printf("waiting...\n");
    // }

    //closing the window
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window->GLFWWindow, true);
    }

    //TOGGLING WIREFRAME MODE
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_F5) == GLFW_PRESS){
        window->wireframeMode = !window->wireframeMode;
    }

    float cameraSpeed;

    //CAMERA CONTROLS
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        cameraSpeed = 100.f * window->deltaTime;
    } else {
        cameraSpeed = 50.f * window->deltaTime;
    }

    //GETTING MOVEMENT STUFF I THINK
    vec3 cameraSpeedTimesCameraFront, strafeValue, cSpeedXcUp;
    glm_vec3_scale(camera.front, cameraSpeed, cameraSpeedTimesCameraFront);

    glm_vec3_scale(camera.up, cameraSpeed, cSpeedXcUp);

    glm_cross(camera.front, camera.up, strafeValue);
    glm_normalize(strafeValue);
    glm_vec3_scale(strafeValue, cameraSpeed, strafeValue);

    //FORWARD MOVEMENT
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_W) == GLFW_PRESS){
        glm_vec3_add(camera.pos, cameraSpeedTimesCameraFront, camera.pos);
    }
    //BACKWARD MOVEMENT
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_S) == GLFW_PRESS){
        glm_vec3_sub(camera.pos, cameraSpeedTimesCameraFront, camera.pos);
    }
    //LEFTHAND MOVEMENT
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_A) == GLFW_PRESS){
        glm_vec3_sub(camera.pos, strafeValue, camera.pos);
    }
    //RIGHTHAND MOVEMENT
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_D) == GLFW_PRESS){
        glm_vec3_add(camera.pos, strafeValue, camera.pos);
    }
    //UPWARD MOVEMENT
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_SPACE) == GLFW_PRESS){
        glm_vec3_add(camera.pos, cSpeedXcUp, camera.pos);
    }
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        glm_vec3_sub(camera.pos, cSpeedXcUp, camera.pos);
    }
    
    //testing mesh regeneration
    if(glfwGetKey(window->GLFWWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS){
        // printf("Regenerating mesh!\n");
        float startMesh = glfwGetTime();
        float endMesh = glfwGetTime();
        float dif = endMesh - startMesh;
        // printf("Done! Time took: %f", dif);
    }

    for(int i = 0; i < 3; ++i){
        int cx, cy, cz;
        camera.posInt[i] = (int)roundf(camera.pos[i]);
        loadedWorld.oldOffset[i] = loadedWorld.offset[i];
    }
    worldToChunkPos(camera.posInt[0], camera.posInt[1], camera.posInt[2], &loadedWorld.offset[0], &loadedWorld.offset[1], &loadedWorld.offset[2]);
    for(int i = 0; i < 3; ++i){
        if(loadedWorld.offset[i] != loadedWorld.oldOffset[i]){
            loadedWorld.needsUpdate = 1;
            break;
        }
    }
}

void mousePosCallback(double xPos, double yPos){
    if(window.firstMouse){
        window.lastX = xPos;
        window.lastY = yPos;
        window.firstMouse = false;
    }

    float xOffset = xPos - window.lastX;
    float yOffset = window.lastY - yPos;

    window.lastX = xPos;
    window.lastY = yPos;

    const float sensitivity = 0.1f;
    yOffset *= sensitivity;
    xOffset *= sensitivity;

    camera.yaw += xOffset;
    camera.pitch += yOffset;

    if(camera.pitch > 89.f){
        camera.pitch = 89.f;
    }
    if(camera.pitch < -89.f){
        camera.pitch = -89.f;
    }


    vec3 cameraDirection;
    cameraDirection[0] = cos(glm_rad(camera.yaw)) * cos(glm_rad(camera.pitch)); //X
    cameraDirection[1] = sin(glm_rad(camera.pitch)); //Y
    cameraDirection[2] = sin(glm_rad(camera.yaw)) * cos(glm_rad(camera.pitch)); //Z
    glm_normalize(cameraDirection);

    camera.front[0] = cameraDirection[0]; //X
    camera.front[1] = cameraDirection[1]; //Y
    camera.front[2] = cameraDirection[2]; //Z
    return;
}

void scrollCallback(double xOffset, double yOffset){
    camera.fov -= (float)yOffset;
    if (camera.fov < 1.0f){camera.fov = 1.0f;}
    if (camera.fov > 359.0f){camera.fov = 359.0f;}
    return;
}