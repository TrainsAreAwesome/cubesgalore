#pragma once
#include <stdint.h>
#include "../../CGl/conciseGl.h"

typedef struct {
    uint8_t id;
    uint8_t state;
} Block_t;

#define AIR 0
#define STONE 1
#define GRASS 2
#define DIRT 3

#define CORNER_BOTTOM_LEFT 1
#define CORNER_BOTTOM_RIGHT 2
#define CORNER_TOP_LEFT 4
#define CORNER_TOP_RIGHT 8

unsigned int getUFromBlockID(uint8_t id, uint8_t corner);
unsigned int getVFromBlockID(uint8_t id, uint8_t corner);

void getTargetedBlock(Camera_t* camera);