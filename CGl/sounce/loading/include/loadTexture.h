#pragma once
//texture and its info
typedef struct {
    int width;
    int height;
    int numChannels;
    unsigned char* image;
} Texture_t;

int loadTexturePNG(unsigned int* glTextureUnsignedInt, char* path);
int loadTextureNoAlpha(unsigned int* glTextureUnsignedInt, char* path);