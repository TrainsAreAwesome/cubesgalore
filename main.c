#include <stdio.h>
#include "CGl/conciseGl.h"
#include "./CGl/other/header/matricies.h"
#include <omp.h>
#include "./game/header/chunk.h"
#include "./game/header/terrainGen.h"
#include "./game/header/world.h"
#include "./game/header/linkklist.h"

#define START_WIDTH 1920
#define START_HEIGHT 1080

#define MAIN_THREAD 0
#define AMOUNT_WORKER_THREADS 3 //dont just put this really high, it lowers fps as they "idle" by just waiting in an infinite, memory intensive loop

CGlWindow_t window;
Camera_t camera;

uint8_t remeshAllChunks = 0;

world loadedWorld;

volatile static int haltThreads = 0; //if i dont define these as volatile compiler optimisations break the programme
volatile int workerThreadsActive[AMOUNT_WORKER_THREADS];

volatile int anyWorkerThreadActive;

//declareations for callback functions
void mousePosCallback(double xPos, double yPos);
void scrollCallback(double xOffset, double yOffset);
void handleKeyboardInput(CGlWindow_t* window);

int main(int argc, char** argv){

    int64_t seed;

    printf("Please enter your seed: ");
    scanf("%d", &seed);
    printf("%d\n", seed);
    printf("\n\n\n");

    printf("Please enter your render distnace:\nX: ");
    scanf("%d", &loadedWorld.maxX);
    printf("Y: ");
    scanf("%d", &loadedWorld.maxY);
    printf("Z: ");
    scanf("%d", &loadedWorld.maxZ);

    fullChunk* (*loadedChunks)[loadedWorld.maxY][loadedWorld.maxZ] = malloc(sizeof(fullChunk*[loadedWorld.maxX][loadedWorld.maxY][loadedWorld.maxZ]));
    if(loadedChunks == NULL){
        printf("Couldn't get enough memory for the array of pointers to chunks!\n");
    }

    // fullChunk* (*chunksToFree)[loadedWorld.maxY][loadedWorld.maxZ] = malloc(sizeof(fullChunk*[loadedWorld.maxX][loadedWorld.maxY][loadedWorld.maxZ]));
    // if(chunksToFree == NULL){
    //     printf("Couldn't get enough memory for the array of pointers to chunks to free!\n");
    // }

    chunkStack* chunksToFreeQueue = initialiseChunkStack();

    loadedWorld.halfMaxX / loadedWorld.maxX / 2;
    loadedWorld.halfMaxY / loadedWorld.maxY / 2;
    loadedWorld.halfMaxZ / loadedWorld.maxZ / 2;

    printf("render distance: X %d Y %d Z %d\n", loadedWorld.maxX, loadedWorld.maxY, loadedWorld.maxZ);

    
    if(initCGl(&window, START_WIDTH, START_HEIGHT, "MWGIES TOOLS!!!", 3, 3)){
        printf("Something went wrong initialising CGl");
        return 1;
    }


 
    // int (*array)[ty][tz] = malloc(sizeof(int[tx][ty][tz])); //what the fuck
    //3D INT ARRAY EXAMPLE


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

    

    printf("maxX %d maxY %d maxZ %d\n", loadedWorld.maxX, loadedWorld.maxY, loadedWorld.maxZ);
    // loadedChunks = initWorld(loadedWorld.maxX, loadedWorld.maxY, loadedWorld.maxZ); //loading world data

    initTerrainGen(&loadedWorld, seed);

    // glEnable(GL_CULL_FACE);
    
    #pragma omp parallel for
    for(int x = 0; x < loadedWorld.maxX; ++x){
        #pragma omp parallel for
        for(int y = 0; y < loadedWorld.maxY; ++y){
            #pragma omp parallel for
            for(int z = 0; z < loadedWorld.maxZ; ++z){
                // loadedChunks[x][y][z] = calloc(1, sizeof(fullChunk));
                // if(loadedChunks[x][y][z] == NULL){
                //     printf("out of memory!\n");
                //     quick_exit(1);
                // }
                
                // printf("allocating memory for chunk %d %d %d\n", x, y, z);
                // chunksToFree[x][y][z] = NULL;
                loadedChunks[x][y][z] = NULL;
                
            }
        }
    }    

    glUseProgram(shaderProgramme);

    //initialising the array to NULL
    // for(int x = 0; x < loadedWorld.maxX; ++x){
    //     for(int y = 0; y < loadedWorld.maxY; ++y){
    //         for(int z = 0; z < loadedWorld.maxZ; ++z){
    //             loadedChunks[x][y][z]->rawMesh = NULL;
    //         }
    //     }
    // }

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
            int amountChunkFrees = 0;

            while (CGlRefreshWindow(&window, (vec3){0.0f, 0.6f, 0.7})){
                // printf("Threads halted? %d\n", haltThreads);
                handleKeyboardInput(&window);
        
                getTargetedBlock(&camera);

                getViewMatrix(viewMatrix, &camera, shaderProgramme);
                getProjectionMatrix(projectionMatrix, camera.fov, window.width, window.height, shaderProgramme);

                getModelMatrix(modelMatrix, shaderProgramme, (vec3){0.0f, 0.0f, 0.0f});

                amountMeshUpdates = 0;
                amountChunkFrees = 0;

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
                    updateWorld(&loadedWorld, offset, &chunksToFreeQueue, loadedChunks);
                    loadedWorld.needsUpdate = 0;
                    haltThreads = 0;
                }

                for(int x = 0; x < loadedWorld.maxX; ++x){
                    for(int y = 0; y < loadedWorld.maxY; ++y){
                        for(int z = 0; z < loadedWorld.maxZ; ++z){

                            if(loadedChunks[x][y][z] == NULL){
                                continue;
                            }
                            //if there is a chunk to free, free it (has to be done on main thread otherwise there will be a vram leak)
                            if(amountChunkFrees < 100){

                                fullChunk* chunkToFree;
                                chunkToFree = chunkLinkListPop(&chunksToFreeQueue);
                                freeChunk(chunkToFree);
                                if(chunkToFree != NULL){
                                    printf("freed chunk at %p\n", chunkToFree);
                                }
                                ++amountChunkFrees;
                            }

                            if(remeshAllChunks){
                                for(int x = 0; x < loadedWorld.maxX; ++x){
                                    for(int y = 0; y < loadedWorld.maxY; ++y){
                                        for(int z = 0; z < loadedWorld.maxZ; ++z){
                                            if(loadedChunks[x][y][z] == NULL){
                                                continue;
                                            }
                                            while (loadedChunks[x][y][z]->busy){
                                            }
                                            loadedChunks[x][y][z]->data.needsRemesh = 1;
                                        }
                                    }
                                }
                                remeshAllChunks = 0;
                            }


                            //if a new mesh has been generated
                            if(!loadedChunks[x][y][z]->data.needsRemesh && loadedChunks[x][y][z]->rawMesh != NULL/* && amountMeshUpdates <= 20*/){
                                while(loadedChunks[x][y][z]->busy){
                                    //wait for chunk to be avalable
                                }
                                loadedChunks[x][y][z]->busy = 1;
                                // printf("putting l %d %d %d into vram on main thread\n", x, y, z);
                                copyMeshIntoVRAM(x, y, z, shaderProgramme, &loadedWorld, loadedChunks[x][y][z]->rawMesh, loadedChunks);
                                // printf("copyied into vram now freeing\n");
                                free(loadedChunks[x][y][z]->rawMesh);
                                loadedChunks[x][y][z]->rawMesh = NULL;
                                // printf("freed the mesh and set it to NULL\n");
                                ++amountMeshUpdates;
                                loadedChunks[x][y][z]->busy = 0;
                            }

                            //if the chunk contains nothing, then dont render
                            if(!loadedChunks[x][y][z]->mesh.countVerticies || !loadedChunks[x][y][z]->data.isGenerated){
                                continue;
                            }
                            glBindVertexArray(loadedChunks[x][y][z]->mesh.vao);
                            glBindBuffer(GL_ARRAY_BUFFER, loadedChunks[x][y][z]->mesh.vbo);
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, loadedChunks[x][y][z]->mesh.ebo);
                            //binding to the correct chunks data

                            //sending chunk pos data to the gpu
                            chunkpos[0] = loadedChunks[x][y][z]->data.x;
                            chunkpos[1] = loadedChunks[x][y][z]->data.y;
                            chunkpos[2] = loadedChunks[x][y][z]->data.z;
                            // glUniform3iv(glGetUniformLocation(shaderProgramme, "chunkPos"), 1, chunkpos);
                            glUniform3iv(chunkposLocation, 1, chunkpos);

                            //drawing
                            glDrawElements(GL_TRIANGLES, loadedChunks[x][y][z]->mesh.countIndicies, GL_UNSIGNED_INT, 0);
                            oldError = error;
                            error = glGetError();
                            if(oldError != error && error != 0){
                                printf("Error while rendering chunk l %d %d %d VBO: %u VAO: %u EBO: %u ERROR: %u\n", x, y, z, loadedChunks[x][y][z]->mesh.vbo, loadedChunks[x][y][z]->mesh.vao, loadedChunks[x][y][z]->mesh.ebo, error);
                                if(error == GL_OUT_OF_MEMORY){
                                    printf("vcount %d icount %d\n", loadedChunks[x][y][z]->mesh.countVerticies, loadedChunks[x][y][z]->mesh.countIndicies);
                                    quick_exit(90);
                                }
                            }
                            // printf("vbo: %u vao: %u ebo: %u\n", loadedChunks[x][y][z]->mesh.vbo, loadedChunks[x][y][z]->mesh.vao, loadedChunks[x][y][z]->mesh.ebo);
                                
                            
                            // printf("vcount %d icount %d\n", loadedChunks[x][y][z]->mesh.countVerticies, loadedChunks[x][y][z]->mesh.countIndicies);
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

                            if(loadedChunks[x][y][z] == NULL){
                                loadedChunks[x][y][z] = calloc(1, sizeof(fullChunk));
                                if(loadedChunks[x][y][z] == NULL){
                                    printf("Out of memory while trying to allocate chunk %d %d %d on thread %d!\n", x, y, z, threadID);
                                    quick_exit(1);
                                }
                                // loadedChunks[x][y][z]->rawMesh = NULL;

                                #ifdef DEBUG_WORKER_THREADS
                                printf("Allocated chunk %d %d %d\n", x, y, z);
                                #endif
                            } else {
                                if(!loadedChunks[x][y][z]->data.isGenerated && !loadedChunks[x][y][z]->busy){
                                    #ifdef DEBUG_WORKER_THREADS
                                    printf("About to generate chunk %d %d %d on thread %d\n", x, y, z, threadID);
                                    #endif

                                    tGenTheadFunction(&loadedWorld, x, y, z, loadedChunks);

                                    #ifdef DEBUG_WORKER_THREADS
                                    printf("Generated chunk %d %d %d on thread %d\n", x, y, z, threadID);
                                    #endif
                                } 
                                if(loadedChunks[x][y][z]->data.needsRemesh && !loadedChunks[x][y][z]->busy && loadedChunks[x][y][z]->data.isGenerated){
                                    #ifdef DEBUG_WORKER_THREADS
                                    printf("About to mesh chunk %d %d %d on thread %d\n", x, y, z, threadID);
                                    #endif

                                    meshThreadFunction(&loadedWorld, x, y, z, loadedChunks);

                                    #ifdef DEBUG_WORKER_THREADS
                                    printf("Meshed chunk %d %d d on thread %d\n", x, y, z, threadID);
                                    #endif
                                }
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
        printf("Setting all meshes for regeneration...\n");
        remeshAllChunks = 1;
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