#include "../header/chunk.h" //chunk.h includes block.h
#include <stdint.h>
#include <stdio.h>

unsigned int getUFromBlockID(uint8_t id, uint8_t corner){
    switch(corner){
        case (CORNER_BOTTOM_LEFT):
            switch(id){
                case STONE:
                    return 1;
                case GRASS:
                    return 2;
                case DIRT:
                    return 3;
                case WATER:
                    return 4;
                case SAND:
                    return 5;
                default:
                    printf("Tried to get a U cord for a block ID that doesnt exist :-( %u\n", id);
                    quick_exit(123);
            }
            break;
        case (CORNER_TOP_LEFT):
            switch(id){
                case STONE:
                    return 1;
                case GRASS:
                    return 2;
                case DIRT:
                    return 3;
                case WATER:
                    return 4;
                case SAND:
                    return 5;
                default:
                    printf("Tried to get a U cord for a block ID that doesnt exist :-( %u\n", id);
                    quick_exit(123);
            }
            break;
        case (CORNER_BOTTOM_RIGHT):
            switch(id){
                case STONE:
                    return 2;
                case GRASS:
                    return 3;
                case DIRT:
                    return 4;
                case WATER:
                    return 5;
                case SAND:
                    return 6;
                default:
                    printf("Tried to get a U cord for a block ID that doesnt exist :-( %u\n", id);
                    quick_exit(123);
            }
            break;
        case (CORNER_TOP_RIGHT):
            switch(id){
                case STONE:
                    return 2;
                case GRASS:
                    return 3;
                case DIRT:
                    return 4;
                case WATER:
                    return 5;
                case SAND:
                    return 6;
                default:
                    printf("Tried to get a U cord for a block ID that doesnt exist :-( %u\n", id);
                    quick_exit(123);
            }
            break;
        default:
        printf("Tried to get a U cord from a corner that doesnt exist :-( %u\n", corner);
        quick_exit(123);
    }
}


unsigned int getVFromBlockID(uint8_t id, uint8_t corner){
    switch(corner){
        case (CORNER_BOTTOM_LEFT):
            switch(id){
                case STONE:
                    return 14;
                case GRASS:
                    return 14;
                case DIRT:
                    return 14;
                case WATER:
                    return 14;
                case SAND:
                    return 14;
                default:
                    printf("Tried to get a V cord for a block ID that doesnt exist :-( %u\n", id);
                    quick_exit(123);
            }
            break;
        case (CORNER_BOTTOM_RIGHT):
            switch(id){
                case STONE:
                    return 14;
                case GRASS:
                    return 14;
                case DIRT:
                    return 14;
                case WATER:
                    return 14;
                case SAND:
                    return 14;
                default:
                    printf("Tried to get a V cord for a block ID that doesnt exist :-( %u\n", id);
                    quick_exit(123);
            }
            break;
        case (CORNER_TOP_LEFT):
            switch(id){
                case STONE:
                    return 15;
                case GRASS:
                    return 15;
                case DIRT:
                    return 15;
                case WATER:
                    return 15;
                case SAND:
                    return 15;
                default:
                    printf("Tried to get a V cord for a block ID that doesnt exist :-( %u\n", id);
                    quick_exit(123);
            }
            break;
        case (CORNER_TOP_RIGHT):
            switch(id){
                case STONE:
                    return 15;
                case GRASS:
                    return 15;
                case DIRT:
                    return 15;
                case WATER:
                    return 15;
                case SAND:
                    return 15;
                default:
                    printf("Tried to get a V cord for a block ID that doesnt exist :-( %u\n", id);
                    quick_exit(123);
            }
            break;
        default:
        printf("Tried to get a V cord from a corner that doesnt exist :-( %u\n", corner);
        quick_exit(123);
    }
}

void getTargetedBlock(Camera_t* camera){
    // printf("%f %f %f\n", camera->front[0], camera->front[1], camera->front[2]);
}