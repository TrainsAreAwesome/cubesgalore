#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

//simple function to create and compile a shader from source and id
int loadShader(unsigned int* shaderPointer, int type, const char* sourcePointer){
    char errmsg[512];
    int success;

    *shaderPointer = glCreateShader(type);
    glShaderSource(*shaderPointer, 1, &sourcePointer, NULL);
    glCompileShader(*shaderPointer);
    glGetShaderiv(*shaderPointer, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(*shaderPointer, 512, NULL, errmsg);
        printf("Error compiling a shader :-(\n%s\n", errmsg);
        return 1;
    }
    return 0;
}

//simple function to create and link a shader programme
int linkShader(unsigned int* programmePointer, unsigned int vertexShader, unsigned int fragmentShader){
    char errmsg[512];
    int success;
    
    *programmePointer = glCreateProgram();
    glAttachShader(*programmePointer, vertexShader);
    glAttachShader(*programmePointer, fragmentShader);
    glLinkProgram(*programmePointer);
    glValidateProgram(*programmePointer);

    glGetProgramiv(*programmePointer, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(*programmePointer, 512, NULL, errmsg);
        printf("Shader linking failed :-(\n%s\n", errmsg);
        return 1;
    }
    return 0;
}

//loads a text file from path specified and returns a pointer to it
//its heap allocated so remember to free() it after use
char* loadTextFile(char* path){
    FILE* file = fopen(path, "r");
    if(file == NULL){
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* string = malloc((length + 1) * sizeof(char));

    char currentChar;
    int i = 0;
    while((currentChar = fgetc(file)) != EOF){
        string[i] = currentChar;
        ++i;
    }
    string[i] = '\0';
    fclose(file);
    return string;
}

//simple function, you pass it a pointer to an unsigned int, along with the vertex and fragment shader source code strings at it compiles and links them
//it deletes the compiled shaders after linking, as we dont need them anymore
//to use, get an unsigned int as your programme and pass a pointer to it in, along with the vertex and fragment shader source code strings
//note that the strings are just regular pointers, not double pointers. after that it gives you a ready to go shader programme, so just use
//glUseProgram() and your ready to go
int loadProgramme(unsigned int* programmePointer, char* vertexShaderSource, char* fragmentShaderSource){
    unsigned int vertexShader, fragmentShader;
    if(loadShader(&vertexShader, GL_VERTEX_SHADER, vertexShaderSource)){
        return 1;
    }
    if(loadShader(&fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderSource)){
        return 1;
    }
    if(linkShader(programmePointer, vertexShader, fragmentShader)){
        return 1;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return 0;
}

int loadShaders(unsigned int* programmePointer, char* vertexShaderPath, char* fragmentShaderPath){
    char* vertexShaderSource = loadTextFile(vertexShaderPath);
    char* fragmentShaderSource = loadTextFile(fragmentShaderPath);
    if(vertexShaderSource == NULL){
        printf("Couldnt read in vertex shader source :-(\n");
        return 1;
    }
    if(fragmentShaderSource == NULL){
        printf("Couldnt read in fragment shader source :-(\n");
        return 1;
    }

    if(loadProgramme(programmePointer, vertexShaderSource, fragmentShaderSource)){
        printf("Something went wrong while compiling shaders :-(\n");
        return 1;
    }
    free(fragmentShaderSource);
    free(vertexShaderSource);
    printf("Shaders compiled sucessully! :-)\n");
    return 0;
}

unsigned int useLoadShaders(char* vertexShaderPath, char* fragmentShaderPath){
    unsigned int programme;
    unsigned int* programmePointer = &programme;
    char* vertexShaderSource = loadTextFile(vertexShaderPath);
    char* fragmentShaderSource = loadTextFile(fragmentShaderPath);
    if(vertexShaderSource == NULL){
        printf("Couldnt read in vertex shader source :-(\n");
        return 1;
    }
    if(fragmentShaderSource == NULL){
        printf("Couldnt read in fragment shader source :-(\n");
        return 1;
    }

    if(loadProgramme(programmePointer, vertexShaderSource, fragmentShaderSource)){
        printf("Something went wrong while compiling shaders :-(\n");
        return 1;
    }
    free(fragmentShaderSource);
    free(vertexShaderSource);
    printf("Shaders compiled sucessully! :-)\n");
    glUseProgram(programme);
    return programme;
}