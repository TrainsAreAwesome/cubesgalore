#include "../header/world.h"
// #include "../header/chunk.h"
#include "../header/terrainGen.h"
#include "../header/linkklist.h"
#include <string.h>

int worldToChunkPos(int x, int y, int z, int* rx, int* ry, int* rz){
    int i = x;
    if(x >= 0 && x < CHUNK_SIZE_X){
        i = 0;
        goto finishX;
    }
    while(abs(i) % CHUNK_SIZE_X != 0){
        --i;
    }
    finishX: *rx = i / CHUNK_SIZE_X;

    i = y;
    if(y >= 0 && y < CHUNK_SIZE_Y){
        i = 0;
        goto finishY;
    }
    while(abs(i) % CHUNK_SIZE_Y != 0){
        --i;
    }
    finishY: *ry = i / CHUNK_SIZE_Y;

    i = z;
    if(z >= 0 && z < CHUNK_SIZE_Z){
        i = 0;
        goto finishZ;
    }
    while(abs(i) % CHUNK_SIZE_Z != 0){
        --i;
    }
    finishZ: *rz = i / CHUNK_SIZE_Z;
    // int dx = (x % CHUNK_SIZE_X);
    // int dy = (y % CHUNK_SIZE_Y);
    // int dz = (z % CHUNK_SIZE_Z);
    
    // *rx = x / CHUNK_SIZE_X;
    // *ry = y / CHUNK_SIZE_Y;
    // *rz = z / CHUNK_SIZE_Z;
    return 0;
}

//if the l cords given are outside the chunk array returns 0. otherwise returns 1.
int isChunkInBounds(world* loadedWorld, int x, int y, int z){
    if(x < 0){
        return 0;
    }
    if(x >= loadedWorld->maxX){
        return 0;
    }

    if(y < 0){
        return 0;
    }
    if(y >= loadedWorld->maxY){
        return 0;
    }

    if(z < 0){
        return 0;
    }
    if(z >= loadedWorld->maxZ){
        return 0;
    }

    return 1;
}

int isChunkAtEdgeOfWorld(world* loadedWorld, int lx, int ly, int lz){
    if(lx == 0){
        return 1;
    }
    if(lx == loadedWorld->maxX - 1){
        return 1;
    }

    if(ly == 0){
        return 1;
    }
    if(ly == loadedWorld->maxY - 1){
        return 1;
    }

    if(lz == 0){
        return 1;
    }
    if(lz == loadedWorld->maxZ - 1){
        return 1;
    }
}

int isChunkNearEdgeOfWorld(world* loadedWorld, int lx, int ly, int lz){
    if(lx == 1){
        return 1;
    }
    if(lx == loadedWorld->maxX - 2){
        return 1;
    }

    if(ly == 1){
        return 1;
    }
    if(ly == loadedWorld->maxY - 2){
        return 1;
    }

    if(lz == 1){
        return 1;
    }
    if(lz == loadedWorld->maxZ - 2){
        return 1;
    }
}

//frees a chunk in the given loadedWorld with the given l cords
int freeChunk(fullChunk* chunk){
    if(chunk != NULL){
        if(chunk->rawMesh != NULL){
            // if(chunk->rawMesh->verticies != NULL){
                free(chunk->rawMesh->verticies);
            // }
            // if(chunk->rawMesh->indicies != NULL){
                free(chunk->rawMesh->indicies);
            // }
            free(chunk->rawMesh);
            chunk->rawMesh = NULL;
        }
        glDeleteBuffers(1, &chunk->mesh.ebo);
        glDeleteBuffers(1, &chunk->mesh.vbo);
        glDeleteVertexArrays(1, &chunk->mesh.vao);
        free(chunk);
    }
}


//updated an array of loaded chunks for a given offset (loading new chunks)
int updateWorld(world* loadedWorld, ivec3 offset, chunkStack** freeQueue, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]){
    
    printf("in update world\n");

    //check offset is valid
    for(int i = 0; i < 3; ++i){
        if(abs(offset[i]) > 1){
            printf("Offset greater that one!: %d %d %d\n", offset[0], offset[1], offset[2]);
            // quick_exit(34567);
        }
    }

    //create the chunk pointer array
    fullChunk* (*newChunks)[loadedWorld->maxY][loadedWorld->maxZ] = malloc(sizeof(fullChunk*[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]));

    //initialise array to NULL
    for(int x = 0; x < loadedWorld->maxX; ++x){
        for(int y = 0; y < loadedWorld->maxY; ++y){
            for(int z = 0; z < loadedWorld->maxZ; ++z){
                newChunks[x][y][z] = NULL;
                if(loadedChunks[x][y][z] != NULL){
                    loadedChunks[x][y][z]->needed = 0;
                }
            }
        }
    }

    printf("made temp pointer array and init to NULL\n");

    //copy old chunks in, create new ones if the old doesnt exist
    int samplePosX, samplePosY, samplePosZ;
    #pragma omp parallel for
    for(int x = 0; x < loadedWorld->maxX; ++x){
        #pragma omp parallel for
        for(int y = 0; y < loadedWorld->maxY; ++y){
            #pragma omp parallel for
            for(int z = 0; z < loadedWorld->maxZ; ++z){
                samplePosX = x + offset[0];
                samplePosY = y + offset[1];
                samplePosZ = z + offset[2];
                
                if(isChunkInBounds(loadedWorld, samplePosX, samplePosY, samplePosZ)){
                    newChunks[x][y][z] = loadedChunks[samplePosX][samplePosY][samplePosZ];

                    if(loadedChunks[samplePosX][samplePosY][samplePosZ] != NULL){
                        loadedChunks[samplePosX][samplePosY][samplePosZ]->needed = 1;
                    }
                } else {
                    // // printf("about to allocate chunk %d %d %d", x, y, z);
                    // newChunks[x][y][z] = calloc(1, sizeof(fullChunk));
                    // // printf("allocated chunk %d %d %d\n", x, y, z);
                    // // newChunks[x][y][z]->data.needsRemesh = 1;
                    // newChunks[x][y][z]->data.isGenerated = 0;
                    // newChunks[x][y][z]->rawMesh = NULL;

                    // newChunks[x][y][z] = NULL;

                    //this is commented out as i now allocate chunks on the worker threads
                }
            }
        }
    }

    printf("got needed chunks\n");
    int ox = loadedWorld->maxX / 2;
    int oy = loadedWorld->maxY / 2;
    int oz = loadedWorld->maxZ / 2;

    //free old unused chunks, then copy the new array into the old one and set chunks that could have their mesh change for remeshing
    for(int x = 0; x < loadedWorld->maxX; ++x){
        for(int y = 0; y < loadedWorld->maxY; ++y){
            for(int z = 0; z < loadedWorld->maxZ; ++z){

                if(loadedChunks[x][y][z] != NULL){
                    if(!loadedChunks[x][y][z]->needed){
                        // chunksToFree[x][y][z] = loadedChunks[x][y][z];
                        // freeChunk(loadedChunks[x][y][z]);
                        chunkLinkListPush(freeQueue, loadedChunks[x][y][z]);
                        loadedChunks[x][y][z] = NULL;
                    }
                }
            
                loadedChunks[x][y][z] = newChunks[x][y][z];

                // loadedChunks[x][y][z]->data.x = x + loadedWorld->offset[0] - ox;
                // loadedChunks[x][y][z]->data.y = y + loadedWorld->offset[1] - oy;
                // loadedChunks[x][y][z]->data.z = z + loadedWorld->offset[2] - oz;
                
                // loadedChunks[x][y][z]->needsUpdating = 1;
            }
        }
    }
    free(newChunks);
    printf("leaving updateWorld\n");
}


//function that gets called on a worker thread to mesh a chunk
void meshThreadFunction(world* loadedWorld, int x, int y, int z, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]){
    //only remesh when all chunks around are generated

    loadedChunks[x][y][z]->busy = 1;

    int allChunksGenerated = 1;
    if(x != 0){
        if(loadedChunks[x - 1][y][z] != NULL){
            allChunksGenerated &= loadedChunks[x - 1][y][z]->data.isGenerated;
        } else {
            allChunksGenerated = 0;
        }
    }

    if(x != loadedWorld->maxX - 1){
        if(loadedChunks[x + 1][y][z] != NULL){
            allChunksGenerated &= loadedChunks[x + 1][y][z]->data.isGenerated;
        } else {
            allChunksGenerated = 0;
        }
    }

    if(y != 0){
        if(loadedChunks[x][y - 1][z] != NULL){
            allChunksGenerated &= loadedChunks[x][y - 1][z]->data.isGenerated;
        } else {
            allChunksGenerated = 0;
        }
    }

    if(y != loadedWorld->maxY - 1){
        if(loadedChunks[x][y + 1][z] != NULL){
            allChunksGenerated &= loadedChunks[x][y + 1][z]->data.isGenerated;
        } else {
            allChunksGenerated = 0;
        }
    }

    if(z != 0){
        if(loadedChunks[x][y][z - 1] != NULL){
            allChunksGenerated &= loadedChunks[x][y][z - 1]->data.isGenerated;
        } else {
            allChunksGenerated = 0;
        }
    }

    if(z != loadedWorld->maxZ - 1){
        if(loadedChunks[x][y][z + 1] != NULL){
            allChunksGenerated &= loadedChunks[x][y][z + 1]->data.isGenerated;
        } else {
            allChunksGenerated = 0;
        }
    }

    if(allChunksGenerated){
        // printf("meshing l %d %d %d on meshing thread\n", x, y, z);
        if(loadedChunks[x][y][z]->rawMesh == NULL){
            // printf("%p\n", loadedChunks[x][y][z]->rawMesh);
            loadedChunks[x][y][z]->rawMesh = calloc(1, sizeof(rawMesh));
            // printf("allocated raw mesh %d %d %d\n", x, y, z);
        } else {
            loadedChunks[x][y][z]->busy = 0; //wait for current mesh to be sent to gpu before overwriting it
            return;
        }
        if(getMesh(&loadedChunks[x][y][z]->data, loadedChunks[x][y][z]->rawMesh, loadedWorld, x, y, z, loadedChunks)){
            // free(loadedChunks[x][y][z]->rawMesh);
            // loadedChunks[x][y][z]->rawMesh = NULL;
            loadedChunks[x][y][z]->busy = 0;
            return;
        }
        loadedChunks[x][y][z]->data.needsRemesh = 0; //we also use this to tell the main thread that we have finished meshing this chunk
    }
    loadedChunks[x][y][z]->busy = 0;
}

//function that gets called on a worker thread to generate a chunk
void tGenTheadFunction(world* loadedWorld, int x, int y, int z, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]){
    
    loadedChunks[x][y][z]->busy = 1;

    //generate it and "queue" it for meshing
    loadedChunks[x][y][z]->data.x = x + loadedWorld->offset[0] - (loadedWorld->maxX / 2);
    loadedChunks[x][y][z]->data.y = y + loadedWorld->offset[1] - (loadedWorld->maxY / 2);
    loadedChunks[x][y][z]->data.z = z + loadedWorld->offset[2] - (loadedWorld->maxZ / 2);

    #ifdef DEBUG_WORKER_THREADS
    printf("Set cords for chunk %d %d %d\n", x, y, z);
    #endif

    generateChunk(loadedChunks[x][y][z], loadedWorld);

    #ifdef DEBUG_WORKER_THREADS
    printf("Generated block data for chunk %d %d %d\n", x, y, z);
    #endif

    loadedChunks[x][y][z]->data.needsRemesh = 1;

    //set surrounding chunks to update their mesh
    if(x != 0){
        if(loadedChunks[x - 1][y][z] != NULL){
            loadedChunks[x - 1][y][z]->data.needsRemesh = 1;
        }
    }

    if(x != loadedWorld->maxX - 1){
        if(loadedChunks[x + 1][y][z] != NULL){
            loadedChunks[x + 1][y][z]->data.needsRemesh = 1;
        }
    }

    if(y != 0){
        if(loadedChunks[x][y - 1][z] != NULL){
            loadedChunks[x][y - 1][z]->data.needsRemesh = 1;
        }
    }

    if(y != loadedWorld->maxY - 1){
        if(loadedChunks[x][y + 1][z] != NULL){
            loadedChunks[x][y + 1][z]->data.needsRemesh = 1;
        }
    }

    if(z != 0){
        if(loadedChunks[x][y][z - 1] != NULL){
            loadedChunks[x][y][z - 1]->data.needsRemesh = 1;
        }
    }

    if(z != loadedWorld->maxZ - 1){
        if(loadedChunks[x][y][z + 1] != NULL){
            loadedChunks[x][y][z + 1]->data.needsRemesh = 1;
        }
    }

    loadedChunks[x][y][z]->data.isGenerated = 1;

    #ifdef DEBUG_WORKER_THREADS
    printf("generated chunk %d %d %d\n", x, y, z);
    #endif

    loadedChunks[x][y][z]->busy = 0;
}

//sets current chunk and surrounding chunks for remeshing
void updateChunk(world* loadedWorld, int x, int y, int z, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]){
    
    loadedChunks[x][y][z]->busy = 1;

    //generate it and "queue" it for meshing
    loadedChunks[x][y][z]->data.x = x + loadedWorld->offset[0] - loadedWorld->halfMaxX;
    loadedChunks[x][y][z]->data.y = y + loadedWorld->offset[1] - loadedWorld->halfMaxY;
    loadedChunks[x][y][z]->data.z = z + loadedWorld->offset[2] - loadedWorld->halfMaxZ;

    loadedChunks[x][y][z]->data.needsRemesh = 1;

    //set surrounding chunks to update their mesh
    if(x != 0){
        loadedChunks[x - 1][y][z]->data.needsRemesh = 1;
    }

    if(x != loadedWorld->maxX - 1){
        loadedChunks[x + 1][y][z]->data.needsRemesh = 1;
    }

    if(y != 0){
        loadedChunks[x][y - 1][z]->data.needsRemesh = 1;
    }

    if(y != loadedWorld->maxY - 1){
        loadedChunks[x][y + 1][z]->data.needsRemesh = 1;
    }

    if(z != 0){
        loadedChunks[x][y][z - 1]->data.needsRemesh = 1;
    }

    if(z != loadedWorld->maxZ - 1){
        loadedChunks[x][y][z + 1]->data.needsRemesh = 1;
    }

    // loadedChunks[x][y][z]->data.isGenerated = 1;
    // printf("generated chunk %d %d %d in tgen thread\n", x, y, z);

    loadedChunks[x][y][z]->busy = 0;
}