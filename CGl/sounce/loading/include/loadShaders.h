#pragma once
int loadShader(unsigned int* shaderPointer, int type, char* sourcePointer);
int linkShader(unsigned int* programmePointer, unsigned int vertexShader, unsigned int fragmentShader);
int loadProgramme(unsigned int* programmePointer, char* vertexShaderSource, char* fragmentShaderSource);
int loadShaders(unsigned int* programmePointer, char* vertexShaderPath, char* fragmentShaderPath);
unsigned int useLoadShaders(char* vertexShaderPath, char* fragmentShaderPath);
char* loadTextFile(char* path);