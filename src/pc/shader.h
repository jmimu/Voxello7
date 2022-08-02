#ifndef SHADER_H
#define SHADER_H

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
struct Shader
{
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;
};

unsigned int compileOneShader(const char* path, int isVertex);
struct Shader *createShader(const char* path_vert, const char* path_frag);
void deleteShader(struct Shader * s);

extern float verticesData[28];
extern unsigned int indicesData[6];

#endif // SHADER_H
