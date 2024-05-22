#pragma once
#include "chunk.h"
#include "linkklist.h"

int worldToChunkPos(int x, int y, int z, int* rx, int* ry, int* rz);
int updateWorld(world* loadedWorld, ivec3 offset, chunkStack** freeQueue, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]);
int isChunkInBounds(world* loadedWorld, int x, int y, int z);
int isChunkAtEdgeOfWorld(world* loadedWorld, int lx, int ly, int lz);
int isChunkNearEdgeOfWorld(world* loadedWorld, int lx, int ly, int lz);
int freeChunk(fullChunk* chunk);


void meshThreadFunction(world* loadedWorld, int x, int y, int z, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]);
void tGenTheadFunction(world* loadedWorld, int x, int y, int z, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]);
void updateChunk(world* loadedWorld, int x, int y, int z, fullChunk* loadedChunks[loadedWorld->maxX][loadedWorld->maxY][loadedWorld->maxZ]);