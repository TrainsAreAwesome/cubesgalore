#define FNL_IMPL
#include "../header/terrainGen.h"
#include <omp.h>
#define MAX_TERRAIN_HEIGHT 1000

void initTerrainGen(world* loadedWorld, int seed){
    loadedWorld->mainNoise = fnlCreateState(seed);
    loadedWorld->mainNoise.fractal_type = FNL_FRACTAL_FBM;
    loadedWorld->mainNoise.octaves = 10;
    loadedWorld->mainNoise.frequency = 0.0005;
    loadedWorld->mainNoise.noise_type = FNL_NOISE_PERLIN;
    loadedWorld->mainNoise.lacunarity = 1.5f;
    loadedWorld->mainNoise.weighted_strength = -1.f;
    loadedWorld->mainNoise.gain = -0.420f;


    loadedWorld->caveNoise = fnlCreateState(seed);
    loadedWorld->caveNoise.noise_type = FNL_NOISE_PERLIN;
    loadedWorld->caveNoise.frequency = 0.01;
    loadedWorld->caveNoise.octaves = 1;
    loadedWorld->caveNoise.gain = 1.0f;
    loadedWorld->caveNoise.lacunarity = 1.0f;
    loadedWorld->caveNoise.weighted_strength = 5.0f;

    loadedWorld->pv = fnlCreateState(seed);
    loadedWorld->pv.noise_type = FNL_NOISE_PERLIN;
    loadedWorld->pv.frequency = 0.001;

}

int generateChunk(fullChunk* chunk, world* loadedWorld){
    // #pragma omp parallel for
    for(int x = 0; x < CHUNK_SIZE_X; ++x){
        // #pragma omp parallel for
        for(int z = 0; z < CHUNK_SIZE_Z; ++z){
            float heightf = fnlGetNoise2D(&loadedWorld->mainNoise, (float)(x + (chunk->data.x * CHUNK_SIZE_X)), (float)(z + (chunk->data.z * CHUNK_SIZE_Z)));
            float pv = fnlGetNoise2D(&loadedWorld->pv, (float)(x + (chunk->data.x * CHUNK_SIZE_X)), (float)(z + (chunk->data.z * CHUNK_SIZE_Z)));
            // float rain = fnlGetNoise2D(&loadedWorld->rain, (float)(x + (chunk->data.x * CHUNK_SIZE_X)), (float)(z + (chunk->data.z * CHUNK_SIZE_Z)));
            // float temp = fnlGetNoise2D(&loadedWorld->temp, (float)(x + (chunk->data.x * CHUNK_SIZE_X)), (float)(z + (chunk->data.z * CHUNK_SIZE_Z)));

            float tpv = (pv + 1) / 2;
            tpv *= 5;
            pv *= tpv;
            


            heightf *= MAX_TERRAIN_HEIGHT;
            heightf *= pv;
            for(int y = 0; y < CHUNK_SIZE_Y; ++y){

                //surface

                if((int)heightf == (y + (chunk->data.y * CHUNK_SIZE_Y))){
                    if(heightf > 150){
                        chunk->data.blocks[x][y][z].id = STONE;
                    } else {
                        chunk->data.blocks[x][y][z].id = GRASS;
                    }
                } else if((int)heightf > (y + (chunk->data.y * CHUNK_SIZE_Y))){
                    if(((int)heightf - 5) <= (y + (chunk->data.y * CHUNK_SIZE_Y))){
                        if (heightf > 150){
                            chunk->data.blocks[x][y][z].id = STONE;
                        } else {
                            chunk->data.blocks[x][y][z].id = DIRT;
                        }            
                    } else {
                        chunk->data.blocks[x][y][z].id = STONE;
                    }
                } else {
                    chunk->data.blocks[x][y][z].id = AIR;
                }
                

                //caves

                // if(chunk->data.blocks[x][y][z].id != AIR){
                //     float cn = fnlGetNoise3D(&loadedWorld->caveNoise, (float)(x + (chunk->data.x * CHUNK_SIZE_X)), (float)(y + (chunk->data.y * CHUNK_SIZE_Y)), (float)(z + (chunk->data.z * CHUNK_SIZE_Z)));
                //     if(cn < 0.025 && cn > -0.025){
                //         chunk->data.blocks[x][y][z].id = AIR;
                //     }
                // }



            }
        }
    }
}