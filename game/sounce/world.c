#include "../header/world.h"
// #include "../header/chunk.h"
#include "../header/terrainGen.h"
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


//updated an array of loaded chunks for a given offset (loading new chunks)
int updateWorld(world* loadedWorld, ivec3 offset){
    
    printf("in update world\n");

    //check offset is valid
    for(int i = 0; i < 3; ++i){
        if(abs(offset[i]) > 1){
            printf("Offset greater that one!: %d %d %d\n", offset[0], offset[1], offset[2]);
            // quick_exit(34567);
        }
    }

    //create the chunk pointer array
    fullChunk* newChunks[LOADED_CHUNKS_X][LOADED_CHUNKS_Y][LOADED_CHUNKS_Z];

    //initialise array to NULL
    for(int x = 0; x < loadedWorld->maxX; ++x){
        for(int y = 0; y < loadedWorld->maxY; ++y){
            for(int z = 0; z < loadedWorld->maxZ; ++z){
                newChunks[x][y][z] = NULL;
                loadedWorld->chunks[x][y][z]->needed = 0;
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
                    newChunks[x][y][z] = loadedWorld->chunks[samplePosX][samplePosY][samplePosZ];
                    loadedWorld->chunks[samplePosX][samplePosY][samplePosZ]->needed = 1;
                } else {
                    // printf("about to allocate chunk %d %d %d", x, y, z);
                    newChunks[x][y][z] = calloc(1, sizeof(fullChunk));
                    // printf("allocated chunk %d %d %d\n", x, y, z);
                    // newChunks[x][y][z]->data.needsRemesh = 1;
                    newChunks[x][y][z]->data.isGenerated = 0;
                    newChunks[x][y][z]->rawMesh = NULL;
                }
            }
        }
    }

    printf("created new chunks\n");
    int ox = loadedWorld->maxX / 2;
    int oy = loadedWorld->maxY / 2;
    int oz = loadedWorld->maxZ / 2;

    //free old unused chunks, then copy the new array into the old one and set chunks that could have their mesh change for remeshing
    for(int x = 0; x < loadedWorld->maxX; ++x){
        for(int y = 0; y < loadedWorld->maxY; ++y){
            for(int z = 0; z < loadedWorld->maxZ; ++z){
                if(!loadedWorld->chunks[x][y][z]->needed){
                    chunksToFree[x][y][z] = loadedWorld->chunks[x][y][z];
                }
                loadedWorld->chunks[x][y][z] = newChunks[x][y][z];

                // loadedWorld->chunks[x][y][z]->data.x = x + loadedWorld->offset[0] - ox;
                // loadedWorld->chunks[x][y][z]->data.y = y + loadedWorld->offset[1] - oy;
                // loadedWorld->chunks[x][y][z]->data.z = z + loadedWorld->offset[2] - oz;
                
                // loadedWorld->chunks[x][y][z]->needsUpdating = 1;
            }
        }
    }
    printf("leaving updateWorld\n");
}


//function that gets called on a worker thread to mesh a chunk
void meshThreadFunction(world* loadedWorld, int x, int y, int z){
    //only remesh when all chunks around are generated

    loadedWorld->chunks[x][y][z]->busy = 1;

    int allChunksGenerated = 1;
    if(x != 0){
        allChunksGenerated &= loadedWorld->chunks[x - 1][y][z]->data.isGenerated;
    }

    if(x != loadedWorld->maxX - 1){
        allChunksGenerated &= loadedWorld->chunks[x + 1][y][z]->data.isGenerated;
    }

    if(y != 0){
        allChunksGenerated &= loadedWorld->chunks[x][y - 1][z]->data.isGenerated;
    }

    if(y != loadedWorld->maxY - 1){
        allChunksGenerated &= loadedWorld->chunks[x][y + 1][z]->data.isGenerated;
    }

    if(z != 0){
        allChunksGenerated &= loadedWorld->chunks[x][y][z - 1]->data.isGenerated;
    }

    if(z != loadedWorld->maxZ - 1){
        allChunksGenerated &= loadedWorld->chunks[x][y][z + 1]->data.isGenerated;
    }

    if(allChunksGenerated){
        // printf("meshing l %d %d %d on meshing thread\n", x, y, z);
        if(loadedWorld->chunks[x][y][z]->rawMesh == NULL){
            // printf("%p\n", loadedWorld->chunks[x][y][z]->rawMesh);
            loadedWorld->chunks[x][y][z]->rawMesh = calloc(1, sizeof(rawMesh));
            // printf("allocated raw mesh %d %d %d\n", x, y, z);
        }  
        if(getMesh(&loadedWorld->chunks[x][y][z]->data, loadedWorld->chunks[x][y][z]->rawMesh, loadedWorld, x, y, z)){
            // free(loadedWorld->chunks[x][y][z]->rawMesh);
            // loadedWorld->chunks[x][y][z]->rawMesh = NULL;
            loadedWorld->chunks[x][y][z]->busy = 0;
            return;
        }
        loadedWorld->chunks[x][y][z]->data.needsRemesh = 0; //we also use this to tell the main thread that we have finished meshing this chunk
    }
    loadedWorld->chunks[x][y][z]->busy = 0;
}

//function that gets called on a worker thread to generate a chunk
void tGenTheadFunction(world* loadedWorld, int x, int y, int z){
    
    loadedWorld->chunks[x][y][z]->busy = 1;

    //generate it and "queue" it for meshing
    loadedWorld->chunks[x][y][z]->data.x = x + loadedWorld->offset[0] - (loadedWorld->maxX / 2);
    loadedWorld->chunks[x][y][z]->data.y = y + loadedWorld->offset[1] - (loadedWorld->maxY / 2);
    loadedWorld->chunks[x][y][z]->data.z = z + loadedWorld->offset[2] - (loadedWorld->maxZ / 2);

    generateChunk(loadedWorld->chunks[x][y][z], loadedWorld);
    loadedWorld->chunks[x][y][z]->data.needsRemesh = 1;

    //set surrounding chunks to update their mesh
    if(x != 0){
        loadedWorld->chunks[x - 1][y][z]->data.needsRemesh = 1;
    }

    if(x != loadedWorld->maxX - 1){
        loadedWorld->chunks[x + 1][y][z]->data.needsRemesh = 1;
    }

    if(y != 0){
        loadedWorld->chunks[x][y - 1][z]->data.needsRemesh = 1;
    }

    if(y != loadedWorld->maxY - 1){
        loadedWorld->chunks[x][y + 1][z]->data.needsRemesh = 1;
    }

    if(z != 0){
        loadedWorld->chunks[x][y][z - 1]->data.needsRemesh = 1;
    }

    if(z != loadedWorld->maxZ - 1){
        loadedWorld->chunks[x][y][z + 1]->data.needsRemesh = 1;
    }

    loadedWorld->chunks[x][y][z]->data.isGenerated = 1;
    // printf("generated chunk %d %d %d in tgen thread\n", x, y, z);

    loadedWorld->chunks[x][y][z]->busy = 0;
}

//sets current chunk and surrounding chunks for remeshing
void updateChunk(world* loadedWorld, int x, int y, int z){
    
    loadedWorld->chunks[x][y][z]->busy = 1;

    //generate it and "queue" it for meshing
    loadedWorld->chunks[x][y][z]->data.x = x + loadedWorld->offset[0] - loadedWorld->halfMaxX;
    loadedWorld->chunks[x][y][z]->data.y = y + loadedWorld->offset[1] - loadedWorld->halfMaxY;
    loadedWorld->chunks[x][y][z]->data.z = z + loadedWorld->offset[2] - loadedWorld->halfMaxZ;

    loadedWorld->chunks[x][y][z]->data.needsRemesh = 1;

    //set surrounding chunks to update their mesh
    if(x != 0){
        loadedWorld->chunks[x - 1][y][z]->data.needsRemesh = 1;
    }

    if(x != loadedWorld->maxX - 1){
        loadedWorld->chunks[x + 1][y][z]->data.needsRemesh = 1;
    }

    if(y != 0){
        loadedWorld->chunks[x][y - 1][z]->data.needsRemesh = 1;
    }

    if(y != loadedWorld->maxY - 1){
        loadedWorld->chunks[x][y + 1][z]->data.needsRemesh = 1;
    }

    if(z != 0){
        loadedWorld->chunks[x][y][z - 1]->data.needsRemesh = 1;
    }

    if(z != loadedWorld->maxZ - 1){
        loadedWorld->chunks[x][y][z + 1]->data.needsRemesh = 1;
    }

    // loadedWorld->chunks[x][y][z]->data.isGenerated = 1;
    // printf("generated chunk %d %d %d in tgen thread\n", x, y, z);

    loadedWorld->chunks[x][y][z]->busy = 0;
}