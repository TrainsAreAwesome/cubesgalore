#include "../header/chunk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include "../../CGl/conciseGl.h"

fullChunk**** initWorld(int x, int y, int z){
    printf("creating chunk pointer array stuff\n");
    fullChunk**** final = calloc(x, sizeof(fullChunk***));
    for(int cy = 0; cy < y; ++cy){
        final[cy] = calloc(y, sizeof(fullChunk**));
        for(int cz = 0; cz < z; ++cz){
            final[cy][cz] = calloc(z, sizeof(fullChunk*));
        }
    }
    printf("done\n");
    return final;
}

const unsigned int upi[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};

const unsigned int nothing[] = {
    
};

// const unsigned int upi[] = {  // note that we start from 0!
//     0, 3, 1,   // first triangle
//     1, 3, 2    // second triangle
// };

//shitty mesh generator
int generateMesh(fullChunk* chunk, world* loadedWorld, int lx, int ly, int lz, unsigned int shaderProgramme, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]){
    //defining data
    cMesh* mesh = &chunk->mesh;
    cData* data = &chunk->data;
    
    //deleting old mesh data
    if(!mesh->used){
        glGenBuffers(1, &mesh->ebo);
        glGenBuffers(1, &mesh->vbo);
        glGenVertexArrays(1, &mesh->vao);

        glBindVertexArray(mesh->vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
        glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
        glEnableVertexAttribArray(0);
        mesh->used = 1;
    } else {
        printf("\n\n\nupdating used mesh!\n\n\n");
        quick_exit(0xccccc);
        glBindVertexArray(mesh->vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    }
    glUseProgram(shaderProgramme);

    //creating an array to store if each block is solid
    uint8_t isSolid[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];

    int vertexIndex = 0; //stores the amount of verticies as we will use malloc later on
    int indieIndex = 0; //^

    int drawPXFace = 0; //we use these to check weather we should draw a face in each direction
    int drawNXFace = 0;
    int drawPYFace = 0;
    int drawNYFace = 0;
    int drawPZFace = 0;
    int drawNZFace = 0;

    // int drawAnything = 0; //if there are no faces to be rendered (all blockids sum together to 0) then we dont need to loop through lots

    // float* tempVerticies = malloc(sizeof(float) * (CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 144)); //that is the max amount of data one chunk vertex table could hold. we put verticies in there and then malloc the correct amount later
    unsigned int* tempVerticies = malloc(sizeof(unsigned int) * (CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 24));
    unsigned int* tempIndicies = malloc(sizeof(unsigned int) * (CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 36)); //^ but for indicies, we put these both on the heap as otherwise we would have a stack overflow by merely creating them

    uint32_t isChunkSolid;

    //generating the solid / not solid table
    for(int x = 0; x < CHUNK_SIZE_X; ++x){
        for(int y = 0; y < CHUNK_SIZE_Y; ++y){
            for(int z = 0; z < CHUNK_SIZE_Z; ++z){
                isSolid[x][y][z] = chunk->data.blocks[x][y][z].id;
                isChunkSolid += isSolid[x][y][z];
            }
        }
    }
    
    if(!isChunkSolid){
        data->needsRemesh = 0;
        //to erase previous buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, (void*)0, GL_STATIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, 0, (void*)0, GL_STATIC_DRAW);
        return 0;
    }

    for(unsigned int x = 0; x < CHUNK_SIZE_X; ++x){
        for(unsigned int y = 0; y < CHUNK_SIZE_Y; ++y){
            for(unsigned int z = 0; z < CHUNK_SIZE_Z; ++z){
                // no point rendering air
                if(isSolid[x][y][z] == 0){
                    continue;
                }
                drawPXFace = 0;
                drawNXFace = 0;
                drawPYFace = 0;
                drawNYFace = 0;
                drawPZFace = 0;
                drawNZFace = 0;
                
                //posotive x
                if(x == CHUNK_SIZE_X - 1){ //if we are at a chunk border
                    if(lx != loadedWorld->maxX - 1){ //check if we are at the border of loaded world
                        if(!loadedChunks[lx + 1][ly][lz]->data.blocks[0][y][z].id){ //if we arnt check the block next in the next chunk
                            drawPXFace = 1;
                        }
                    }
                } else if(isSolid[x + 1][y][z] == 0){
                    drawPXFace = 1;
                }

                //negative x
                if(x == 0){
                    if(lx != 0){
                        if(!loadedChunks[lx - 1][ly][lz]->data.blocks[CHUNK_SIZE_X - 1][y][z].id){
                            drawNXFace = 1;
                        }
                    }
                } else if(isSolid[x - 1][y][z] == 0){
                    drawNXFace = 1;
                }

                //posotive y
                if(y == CHUNK_SIZE_Y - 1){
                    if(ly != loadedWorld->maxY - 1){
                        if(!loadedChunks[lx][ly + 1][lz]->data.blocks[x][0][z].id){
                            drawPYFace = 1;
                        }
                    }
                } else if(isSolid[x][y + 1][z] == 0){
                    drawPYFace = 1;
                }

                //negative y
                if(y == 0){
                    if(ly != 0){
                        if(!loadedChunks[lx][ly - 1][lz]->data.blocks[x][CHUNK_SIZE_Y - 1][z].id){
                            drawNYFace = 1;
                        }
                    }
                } else if(isSolid[x][y- 1][z] == 0){
                    drawNYFace = 1;
                }

                //posative z
                if(z == CHUNK_SIZE_Z - 1){
                    if(lz != loadedWorld->maxZ - 1){
                        if(!loadedChunks[lx][ly][lz + 1]->data.blocks[x][y][0].id){
                            drawPZFace = 1;
                        }
                    }
                } else if(isSolid[x][y][z + 1] == 0){
                    drawPZFace = 1;
                }

                //negative z
                if(z == 0){
                    if(lz != 0){
                        if(!loadedChunks[lx][ly][lz - 1]->data.blocks[x][y][CHUNK_SIZE_Z - 1].id){
                            drawNZFace = 1;
                        }
                    }
                } else if(isSolid[x][y][z - 1] == 0){
                    drawNZFace = 1;
                }

                if(drawPXFace){
                    // float upv[] = {
                    //     0.5f + (float)(x + (data->x * CHUNK_SIZE_X)),  0.5f + (float)(y + (data->y * CHUNK_SIZE_Y)), -0.5f + (float)(z + (data->z * CHUNK_SIZE_Z)), 1.0f, 1.0f, 1.0f, // top right
                    //     0.5f + (float)(x + (data->x * CHUNK_SIZE_X)), -0.5f + (float)(y + (data->y * CHUNK_SIZE_Y)), -0.5f + (float)(z + (data->z * CHUNK_SIZE_Z)), 1.0f, 1.0f, 0.0f, // bottom right
                    //     0.5f + (float)(x + (data->x * CHUNK_SIZE_X)), -0.5f + (float)(y + (data->y * CHUNK_SIZE_Y)), 0.5f + (float)(z + (data->z * CHUNK_SIZE_Z)), 1.0f, 0.0f, 0.0f, // bottom left
                    //     0.5f + (float)(x + (data->x * CHUNK_SIZE_X)),  0.5f + (float)(y + (data->y * CHUNK_SIZE_Y)), 0.5f + (float)(z + (data->z * CHUNK_SIZE_Z)), 1.0f, 0.0f, 1.0f // top left
                    // };
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (2 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (2 << 18)  | ((x + 1) << 12) | (y << 6)       | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (2 << 18)  | ((x + 1) << 12) | (y << 6)       | (z + 1), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (2 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. TOP LEFT
                    };
                        
                    
                    //adding stuff
                    // for(int c = 0; c < sizeof(upv) / sizeof(float); ++c){
                    //     tempVerticies[vertexIndex] = upv[c];
                    //     //adding incecies
                    //     if(sizeof(upi) / sizeof(unsigned int) > c){ //if c is a valid index into the indicies array
                    //         tempIndicies[indieIndex] = ((upi[c]) + (vertexIndex / 6));
                    //         ++indieIndex;
                    //     }
                    //     ++vertexIndex;
                    // }
                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                if(drawNXFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (2 << 18) | (x << 12) | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (2 << 18) | (x << 12) | (y << 6)       | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (2 << 18) | (x << 12) | (y << 6)       | (z + 1), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (2 << 18) | (x << 12) | ((y + 1) << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };
                        
                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                if(drawPYFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (3 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z + 1), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (3 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (3 << 18) | (x << 12)       | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (3 << 18) | (x << 12)       | ((y + 1) << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };

                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                if(drawNYFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (0 << 18) | ((x + 1) << 12) | (y << 6) | (z + 1), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (0 << 18) | ((x + 1) << 12) | (y << 6) | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (0 << 18) | (x << 12)       | (y << 6) | (z), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (0 << 18) | (x << 12)       | (y << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };

                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                if(drawPZFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (1 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z + 1), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (1 << 18) | ((x + 1) << 12) | (y << 6)       | (z + 1), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (1 << 18) | (x << 12)       | (y << 6)       | (z + 1), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (1 << 18) | (x << 12)       | ((y + 1) << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };

                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                
                if(drawNZFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (1 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (1 << 18) | ((x + 1) << 12) | (y << 6)       | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (1 << 18) | (x << 12)       | (y << 6)       | (z), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (1 << 18) | (x << 12)       | ((y + 1) << 6) | (z) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };

                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }
            }
        }
    }
    
    int countVerticies = vertexIndex /*+ 1*/;
    int tempVI = 0;

    for(int c = 0; c < ((vertexIndex) + (vertexIndex / 2)); ++c){
        if(!(c % 6) && c != 0){
            tempVI += 4;
        }
        tempIndicies[c] = upi[c % 6] + tempVI;
        ++indieIndex;
    }


    int countIndicies = indieIndex /*+ 1*/;

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * countIndicies, tempIndicies, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * countVerticies, tempVerticies, GL_STATIC_DRAW);
    mesh->countVerticies = countVerticies; //update vertex count
    mesh->countIndicies = countIndicies; //update indicie count
    free(tempIndicies);
    free(tempVerticies);
    data->needsRemesh = 0;

}


int initMeshFull(fullChunk* chunk, unsigned int shaderProgramme){
    glGenBuffers(1, &chunk->mesh.ebo);
    glGenBuffers(1, &chunk->mesh.vbo);
    glGenVertexArrays(1, &chunk->mesh.vao);

    glBindVertexArray(chunk->mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->mesh.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->mesh.ebo);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgramme);
}


//CORDS IN ARE L CORDS, RUN ON MAIN THREAD ONLY
int copyMeshIntoVRAM(int x, int y, int z, unsigned int shaderProgramme, world* loadedWorld, rawMesh* tmesh, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]){
    // printf("\ncopying mesh into vram start\n");
    if(loadedChunks[x][y][z] != NULL){
        if(loadedChunks[x][y][z]->mesh.used == 0){
            initMeshFull(loadedChunks[x][y][z], shaderProgramme);
            // printf("\ninit mesh in vramf\n");
        } else {
            glBindVertexArray(loadedChunks[x][y][z]->mesh.vao);
            glBindBuffer(GL_ARRAY_BUFFER, loadedChunks[x][y][z]->mesh.vbo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, loadedChunks[x][y][z]->mesh.ebo);
        }
        if(tmesh->countVerticies){
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * tmesh->countIndicies, tmesh->indicies, GL_STATIC_DRAW);
            glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * tmesh->countVerticies, tmesh->verticies, GL_STATIC_DRAW);
        } else if(loadedChunks[x][y][z]->mesh.used) { //if there was previous data but all blocks got destroyed then set the buffer to zero
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * tmesh->countIndicies, (void*)0, GL_STATIC_DRAW);
            glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * tmesh->countVerticies, (void*)0, GL_STATIC_DRAW);
        }
        // printf("\njust done glbufferdata\n");
        loadedChunks[x][y][z]->mesh.countIndicies = tmesh->countIndicies;
        loadedChunks[x][y][z]->mesh.countVerticies = tmesh->countVerticies;
        // printf("\nset vi count");
        //deleting the mesh

        free(tmesh->verticies);

        free(tmesh->indicies);

        loadedChunks[x][y][z]->mesh.used = 1;
    }
    
        
    // printf("\nfreed data and about to exit copyMeshIntoVRAM\n");
}


//shitty mesh generator, returns mesh data instead of copying it into gpu, better for multithreading
int getMesh(cData* data, rawMesh* result, world* loadedWorld, int lx, int ly, int lz, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]){
    //defining data

    if(result == NULL){
        quick_exit(23456);
        // result = calloc(1, sizeof(rawMesh));
    }

    // printf("about to do result->used\n");
    if(result->used){
        if(result->verticies != NULL){
            // free(result->verticies); // MEMORY ISSUES HERE
            // printf("MEMORY LEAK VERTICIES\n");
            return 1;
            // result->verticies = NULL;
        }
        if(result->indicies != NULL){
            // free(result->indicies); // MEMORY ISSUES HERE
            // printf("MEMORY LEAK INDICIES\n");
            return 1;
            // result->indicies = NULL;
        }
    }

    // printf("here end in getMesh\n");

    //creating an array to store if each block is solid
    uint8_t isSolid[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];

    int vertexIndex = 0; //stores the amount of verticies as we will use malloc later on
    int indieIndex = 0; //^

    int drawPXFace = 0; //we use these to check weather we should draw a face in each direction
    int drawNXFace = 0;
    int drawPYFace = 0;
    int drawNYFace = 0;
    int drawPZFace = 0;
    int drawNZFace = 0;

    // int drawAnything = 0; //if there are no faces to be rendered (all blockids sum together to 0) then we dont need to loop through lots

    // float* tempVerticies = malloc(sizeof(float) * (CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 144)); //that is the max amount of data one chunk vertex table could hold. we put verticies in there and then malloc the correct amount later
    unsigned int* tempVerticies = malloc(sizeof(unsigned int) * (CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 24));
    unsigned int* tempIndicies = malloc(sizeof(unsigned int) * (CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 36)); //^ but for indicies, we put these both on the heap as otherwise we would have a stack overflow by merely creating them

    uint32_t isChunkSolid;

    //generating the solid / not solid table
    for(int x = 0; x < CHUNK_SIZE_X; ++x){
        for(int y = 0; y < CHUNK_SIZE_Y; ++y){
            for(int z = 0; z < CHUNK_SIZE_Z; ++z){
                isSolid[x][y][z] = data->blocks[x][y][z].id;
                isChunkSolid += isSolid[x][y][z];
            }
        }
    }
    
    if(!isChunkSolid){
        return 0;
    }

    // printf("passed the solidity check in getMesh\n");

    for(unsigned int x = 0; x < CHUNK_SIZE_X; ++x){
        for(unsigned int y = 0; y < CHUNK_SIZE_Y; ++y){
            for(unsigned int z = 0; z < CHUNK_SIZE_Z; ++z){
                // no point rendering air
                if(isSolid[x][y][z] == 0){
                    continue;
                }
                drawPXFace = 0;
                drawNXFace = 0;
                drawPYFace = 0;
                drawNYFace = 0;
                drawPZFace = 0;
                drawNZFace = 0;
                
                //posotive x
                if(x == CHUNK_SIZE_X - 1){ //if we are at a chunk border
                    if(lx != loadedWorld->maxX - 1){ //check if we are at the border of loaded world
                        if(loadedChunks[lx + 1][ly][lz] != NULL){
                            if(!loadedChunks[lx + 1][ly][lz]->data.blocks[0][y][z].id){ //if we arnt check the block next in the next chunk
                                drawPXFace = 1;
                            }
                        }
                    }
                } else if(isSolid[x + 1][y][z] == 0){
                    drawPXFace = 1;
                }

                //negative x
                if(x == 0){
                    if(lx != 0){
                        if(loadedChunks[lx - 1][ly][lz] != NULL){
                            if(!loadedChunks[lx - 1][ly][lz]->data.blocks[CHUNK_SIZE_X - 1][y][z].id){
                                drawNXFace = 1;
                            }
                        }
                    }
                } else if(isSolid[x - 1][y][z] == 0){
                    drawNXFace = 1;
                }

                //posotive y
                if(y == CHUNK_SIZE_Y - 1){
                    if(ly != loadedWorld->maxY - 1){
                        if(loadedChunks[lx][ly + 1][lz] != NULL){
                            if(!loadedChunks[lx][ly + 1][lz]->data.blocks[x][0][z].id){
                                drawPYFace = 1;
                            }
                        }
                    }
                } else if(isSolid[x][y + 1][z] == 0){
                    drawPYFace = 1;
                }

                //negative y
                if(y == 0){
                    if(ly != 0){
                        if(loadedChunks[lx][ly - 1][lz] != NULL){
                            if(!loadedChunks[lx][ly - 1][lz]->data.blocks[x][CHUNK_SIZE_Y - 1][z].id){
                                drawNYFace = 1;
                            }
                        }
                    }
                } else if(isSolid[x][y- 1][z] == 0){
                    drawNYFace = 1;
                }

                //posative z
                if(z == CHUNK_SIZE_Z - 1){
                    if(lz != loadedWorld->maxZ - 1){
                        if(loadedChunks[lx][ly][lz + 1] != NULL){
                            if(!loadedChunks[lx][ly][lz + 1]->data.blocks[x][y][0].id){
                                drawPZFace = 1;
                            }
                        }
                    }
                } else if(isSolid[x][y][z + 1] == 0){
                    drawPZFace = 1;
                }

                //negative z
                if(z == 0){
                    if(lz != 0){
                        if(loadedChunks[lx][ly][lz - 1] != NULL){
                            if(!loadedChunks[lx][ly][lz - 1]->data.blocks[x][y][CHUNK_SIZE_Z - 1].id){
                                drawNZFace = 1;
                            }
                        }
                    }
                } else if(isSolid[x][y][z - 1] == 0){
                    drawNZFace = 1;
                }

                if(drawPXFace){
                    // float upv[] = {
                    //     0.5f + (float)(x + (data->x * CHUNK_SIZE_X)),  0.5f + (float)(y + (data->y * CHUNK_SIZE_Y)), -0.5f + (float)(z + (data->z * CHUNK_SIZE_Z)), 1.0f, 1.0f, 1.0f, // top right
                    //     0.5f + (float)(x + (data->x * CHUNK_SIZE_X)), -0.5f + (float)(y + (data->y * CHUNK_SIZE_Y)), -0.5f + (float)(z + (data->z * CHUNK_SIZE_Z)), 1.0f, 1.0f, 0.0f, // bottom right
                    //     0.5f + (float)(x + (data->x * CHUNK_SIZE_X)), -0.5f + (float)(y + (data->y * CHUNK_SIZE_Y)), 0.5f + (float)(z + (data->z * CHUNK_SIZE_Z)), 1.0f, 0.0f, 0.0f, // bottom left
                    //     0.5f + (float)(x + (data->x * CHUNK_SIZE_X)),  0.5f + (float)(y + (data->y * CHUNK_SIZE_Y)), 0.5f + (float)(z + (data->z * CHUNK_SIZE_Z)), 1.0f, 0.0f, 1.0f // top left
                    // };
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (2 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (2 << 18)  | ((x + 1) << 12) | (y << 6)       | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (2 << 18)  | ((x + 1) << 12) | (y << 6)       | (z + 1), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (2 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. TOP LEFT
                    };
                        
                    
                    //adding stuff
                    // for(int c = 0; c < sizeof(upv) / sizeof(float); ++c){
                    //     tempVerticies[vertexIndex] = upv[c];
                    //     //adding incecies
                    //     if(sizeof(upi) / sizeof(unsigned int) > c){ //if c is a valid index into the indicies array
                    //         tempIndicies[indieIndex] = ((upi[c]) + (vertexIndex / 6));
                    //         ++indieIndex;
                    //     }
                    //     ++vertexIndex;
                    // }
                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                if(drawNXFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (2 << 18) | (x << 12) | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (2 << 18) | (x << 12) | (y << 6)       | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (2 << 18) | (x << 12) | (y << 6)       | (z + 1), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (2 << 18) | (x << 12) | ((y + 1) << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };
                        
                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                if(drawPYFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (3 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z + 1), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (3 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (3 << 18) | (x << 12)       | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (3 << 18) | (x << 12)       | ((y + 1) << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };

                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                if(drawNYFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (0 << 18) | ((x + 1) << 12) | (y << 6) | (z + 1), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (0 << 18) | ((x + 1) << 12) | (y << 6) | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (0 << 18) | (x << 12)       | (y << 6) | (z), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (0 << 18) | (x << 12)       | (y << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };

                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                if(drawPZFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (1 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z + 1), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (1 << 18) | ((x + 1) << 12) | (y << 6)       | (z + 1), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (1 << 18) | (x << 12)       | (y << 6)       | (z + 1), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (1 << 18) | (x << 12)       | ((y + 1) << 6) | (z + 1) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };

                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }

                
                if(drawNZFace){
                    unsigned int upv[] = {
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 24)    | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_RIGHT) << 20)    | (1 << 18) | ((x + 1) << 12) | ((y + 1) << 6) | (z), //U, V, LIGHT, X, Y, Z. TOP RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 24) | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_RIGHT) << 20) | (1 << 18) | ((x + 1) << 12) | (y << 6)       | (z), //U, V, LIGHT, X, Y, Z. BOTTOM RIGHT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 24)  | (getVFromBlockID(isSolid[x][y][z], CORNER_BOTTOM_LEFT) << 20)  | (1 << 18) | (x << 12)       | (y << 6)       | (z), //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                        (getUFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 24)     | (getVFromBlockID(isSolid[x][y][z], CORNER_TOP_LEFT) << 20)     | (1 << 18) | (x << 12)       | ((y + 1) << 6) | (z) //U, V, LIGHT, X, Y, Z. BOTTOM LEFT
                    };

                    for(int c = 0; c < sizeof(upv) / sizeof(unsigned int); ++c){
                        tempVerticies[vertexIndex] = upv[c];
                        ++vertexIndex;
                    }
                }
            }
        }
    }

    // printf("added faces in getMesh\n");
    
    int countVerticies = vertexIndex /*+ 1*/;
    int tempVI = 0;

    for(int c = 0; c < ((vertexIndex) + (vertexIndex / 2)); ++c){
        if(!(c % 6) && c != 0){
            tempVI += 4;
        }
        tempIndicies[c] = upi[c % 6] + tempVI;
        ++indieIndex;
    }

    // printf("calculated indicies in getMesh\n");


    int countIndicies = indieIndex /*+ 1*/;


    result->countVerticies = countVerticies; //update vertex count
    result->countIndicies = countIndicies; //update indicie count

    // printf("stored things in result mesh in getMesh\n");

    result->verticies = calloc(countVerticies, sizeof(unsigned int));
    result->indicies = calloc(countIndicies, sizeof(unsigned int));

    // printf("allocated memory and about to memcpy\n");
    memcpy(result->verticies, tempVerticies, countVerticies * sizeof(unsigned int));
    memcpy(result->indicies, tempIndicies, countIndicies * sizeof(unsigned int));
    // printf("memcpyd in getMesh done\n");

    // printf("about to free in getMesh\n");
    free(tempIndicies);
    free(tempVerticies);
    // printf("freed in getMesh\n");


    data->needsRemesh = 0;
    result->used = 1;

    return 0;
    // printf("finished getMesh...\n");
}