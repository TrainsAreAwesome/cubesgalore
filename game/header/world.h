#pragma once
#include "chunk.h"

extern fullChunk* chunksToFree[LOADED_CHUNKS_X][LOADED_CHUNKS_Y][LOADED_CHUNKS_Z];

int worldToChunkPos(int x, int y, int z, int* rx, int* ry, int* rz);
int updateWorld(world* loadedWorld, ivec3 offset);
int isChunkInBounds(world* loadedWorld, int x, int y, int z);
int isChunkAtEdgeOfWorld(world* loadedWorld, int lx, int ly, int lz);
int isChunkNearEdgeOfWorld(world* loadedWorld, int lx, int ly, int lz);
int freeChunk(fullChunk* chunk);


void meshThreadFunction(world* loadedWorld, int x, int y, int z);
void tGenTheadFunction(world* loadedWorld, int x, int y, int z);
void updateChunk(world* loadedWorld, int x, int y, int z);