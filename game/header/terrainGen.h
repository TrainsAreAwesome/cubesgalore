#pragma once
#include "./chunk.h"

int generateChunk(fullChunk* chunk, world* loadedWorld);
void initTerrainGen(world* loadedWorld, int64_t seed);