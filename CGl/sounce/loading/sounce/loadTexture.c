#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include "../include/loadTexture.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

//loads and configures a texture from path specified
int loadTexturePNG(unsigned int* glTextureUnsignedInt, char* path){
    stbi_set_flip_vertically_on_load(1);
    Texture_t texture;

    texture.image = stbi_load(path, &texture.width, &texture.height, &texture.numChannels, 0);
    if(!texture.image){
        printf("Failed to load alpha texture :-(\n");
        return 1;
    }
    
    //generate the texture obj
    glGenTextures(1, glTextureUnsignedInt);
    glBindTexture(GL_TEXTURE_2D, *glTextureUnsignedInt);
    //set its params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    printf("image width %d image height %d\n", texture.width, texture.height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.image);
    printf("here with path %s\n", path);
    // glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(texture.image);
    return 0;
}

int loadTextureNoAlpha(unsigned int* glTextureUnsignedInt, char* path){
    stbi_set_flip_vertically_on_load(1);
    Texture_t texture;

    texture.image = stbi_load(path, &texture.width, &texture.height, &texture.numChannels, 0);
    if(!texture.image){
        printf("Failed to load no alpha texture :-(\n");
        return 1;
    }
    
    //generate the texture obj
    glGenTextures(1, glTextureUnsignedInt);
    glBindTexture(GL_TEXTURE_2D, *glTextureUnsignedInt);
    //set its params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.image);
    printf("here with path %s\n", path);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(texture.image);
    return 0;
}