#pragma once
#include "./chunk.h"
#include <stdlib.h>

//element of chunk stack
typedef struct chunkStack{
    fullChunk* chunk;
    struct chunkStack* previous; //why do i have to put struct here? i dont know but the compiler complains otherwise.
} chunkStack;

//creates a chunk stack
chunkStack* initialiseChunkStack();

//pushes given chunk pointer onto given chunk pointer stack (creates new element)
void chunkLinkListPush(chunkStack** head, fullChunk* chunk);

//pops the top element off a given chunk pointer stack (removes element)
fullChunk* chunkLinkListPop(chunkStack** head);