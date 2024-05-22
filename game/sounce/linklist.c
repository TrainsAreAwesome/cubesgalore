#include "../header/linkklist.h"
// #define DEBUG_CHUNK_STACK

chunkStack* initialiseChunkStack(){
    return calloc(1, sizeof(chunkStack));
}

void chunkLinkListPush(chunkStack** head, fullChunk* chunk){
    chunkStack* newElement = calloc(1, sizeof(chunkStack)); //allocate new element
    #ifdef DEBUG_CHUNK_STACK
    printf("pushed %p to stack\n", chunk);
    #endif

    newElement->previous = *head;
    newElement->chunk = chunk;
    *head = newElement;
    // printf("just pushed %p to stack\n", *head->chunk);
}

fullChunk* chunkLinkListPop(chunkStack** head){
    chunkStack* previos = (*head)->previous; //extract previous element pointer
    fullChunk* chunk = (*head)->chunk; //get data out

    #ifdef DEBUG_CHUNK_STACK
    printf("extracting %p from stack\n", chunk);
    #endif

    if(previos == NULL){ //if this is the bottom of the stack then return NULL
        return NULL;
    }

    #ifdef DEBUG_CHUNK_STACK
    printf("previos: %p\n", previos);
    #endif

    free(*head); //free current element pointer
    *head = previos; //set current element pointer to previos element pointer

    #ifdef DEBUG_CHUNK_STACK
    printf("its not null\n");
    #endif

    return chunk; //return the data
}