#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include "../dbg.h"


float verticesData[28] = {
     //2d-pos   // colors          //tex coord
    -1.0,  1.0, 1.0f, 0.0f, 0.0f,  0.0, 0.0,
    -1.0, -1.0, 0.0f, 1.0f, 0.0f,  1.0, 0.0,
     1.0, -1.0, 0.0f, 0.0f, 1.0f,  1.0, 1.0,
     1.0,  1.0, 0.0f, 1.0f, 1.0f,  0.0, 1.0,
};

unsigned int indicesData[6] = {
    0,  1,  2,  2,  3,  0, //2 tri = quad / line
};

unsigned int compileOneShader(const char* path, int isVertex)
{
    unsigned int id=0;
    FILE *fp = fopen( path, "r" );
    check(fp,"failed to open file");
    fseek(fp, 0L, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* src = malloc(sz+1);
    fread(src, sz, 1, fp);
    fclose(fp);
    src[sz] = 0;

    printf("shader source: \n%s", src);
    if (isVertex)
        id = glCreateShader( GL_VERTEX_SHADER );
    else
        id = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( id, 1, &src, &sz );
    glCompileShader( id );

    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        fprintf( stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",infoLog);
        return 0;
    }
    printf("shader compilation done\n" );
    free(src);
error:
    return id;
}

struct Shader* createShader(struct Shader* shader, const char* path_vert, const char* path_frag)
{
    if (!shader)
        shader = malloc(sizeof(struct Shader));

    shader->vertexShader = compileOneShader(path_vert, 1);
    shader->fragmentShader = compileOneShader(path_frag, 0);

    if (shader->vertexShader && shader->fragmentShader)
    {
        shader->shaderProgram = glCreateProgram();
        glAttachShader( shader->shaderProgram, shader->vertexShader );
        glAttachShader( shader->shaderProgram, shader->fragmentShader );

        glLinkProgram(shader->shaderProgram);
        glValidateProgram(shader->shaderProgram);
        // check for linking errors
        int success;
        char infoLog[512];
        glGetProgramiv(shader->shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader->shaderProgram, 512, NULL, infoLog);
            fprintf( stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",infoLog);
        }
        glDeleteShader(shader->vertexShader);
        glDeleteShader(shader->fragmentShader);

        return shader;
    }

error:
    deleteShader(shader);
    return NULL;
}

void deleteShader(struct Shader * s)
{
    if (s->shaderProgram)
        glDeleteProgram(s->shaderProgram);
}
