#pragma once
#include "../header/external/fastNoiseLite.h"
#include "./block.h"
#include "../../CGl/conciseGl.h"
#define CHUNK_SIZE_X 32
#define CHUNK_SIZE_Y 32
#define CHUNK_SIZE_Z 32

#define LOADED_CHUNKS_X 64
#define LOADED_CHUNKS_Y 48
#define LOADED_CHUNKS_Z 64


typedef struct{
    Block_t blocks[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
    int32_t x, y, z;
    uint8_t needsRemesh;
    uint8_t needsSaving;
    uint8_t isGenerated;
    uint8_t used;
} cData;

typedef struct {
    int countVerticies;
    int countIndicies;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int vao;
    unsigned int used;
    unsigned int shaderProgramme;
} cMesh;

typedef struct {
    int countVerticies;
    int countIndicies;
    unsigned int* verticies;
    unsigned int* indicies;
    unsigned int used;
} rawMesh;

typedef struct {
    cData data;
    cMesh mesh;
    rawMesh* rawMesh;
    volatile uint8_t busy;
    uint8_t needed;
    uint8_t needsUpdating;
} fullChunk;

typedef struct{
    // fullChunk**** chunks;
    fullChunk* chunks[LOADED_CHUNKS_X][LOADED_CHUNKS_Y][LOADED_CHUNKS_Z];
    // fullChunk chunks[LOADED_CHUNKS_X][LOADED_CHUNKS_Y][LOADED_CHUNKS_Z];
    fnl_state mainNoise;
    fnl_state caveNoise;

    fnl_state pv;
    fnl_state rain;
    fnl_state temp;
    int32_t maxX, maxY, maxZ;
    int32_t halfMaxX, halfMaxY, halfMaxZ;

    ivec3 offset;
    ivec3 oldOffset;

    uint8_t needsUpdate;
} world;

int generateMesh(fullChunk* chunk, world* loadedWorld, int lx, int ly, int lz, unsigned int shaderProgramme);
int getMesh(cData* data, rawMesh* result, world* loadedWorld, int lx, int ly, int lz);
int initMeshFull(fullChunk* chunk, unsigned int shaderProgramme);
int copyMeshIntoVRAM(int x, int y, int z, unsigned int shaderProgramme, world* loadedWorld, rawMesh* tempMeshes);
fullChunk**** initWorld(int x, int y, int z);