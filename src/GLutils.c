#ifndef GL_UTILS_H
#define GL_UTILS_H
#define GL_UTILS_INCLUDE_IMPLEMENTATION

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "thirdparty/include/glad.h"
#include <GLFW/glfw3.h>

GLint ShaderFromSource(const char *shader_source_path, GLint shader_type);
GLint ShaderFromSPIRV(const char *shader_source_path, GLint shader_type,
                      const char *entry);
GLint LinkShaders(uint32_t shader_count, ...);

#endif // #define GL_UTILS_H

#ifdef GL_UTILS_INCLUDE_IMPLEMENTATION

static inline const char *_glutils_read_file(const char *path)
{
    // open file
    FILE *file = fopen(path, "r");
    if (file == 0)
    {
        fprintf(stderr, "Couldnt read file %s\n", path);
        return NULL;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    size_t shader_source_size = ftell(file);
    rewind(file);

    // allocate enough memory for the source code
    char *shader_source =
        calloc((shader_source_size + 1), sizeof *shader_source);
    if (shader_source == NULL)
    {
        fprintf(stderr, "Call to calloc failed in OpenglCompileShader");
    }

    // read from file
    fread(shader_source, sizeof *shader_source, shader_source_size, file);
    return shader_source;
}

// on error returns -1
GLint ShaderFromSource(const char *shader_source_path, GLint shader_type)
{
    // open shader source file
    const char *shader_source = _glutils_read_file(shader_source_path);
    if (shader_source == NULL)
    {
        return -1;
    }

    // compile the shader
    GLint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const char *const *)&shader_source, NULL);
    glCompileShader(shader);

    // check for compilation errors
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == 0)
    {
        char infolog[512];
        glGetShaderInfoLog(shader, 512, NULL, infolog);
        fprintf(
            stderr,
            "Failed to compile shader at %s, gathering information...\n%s\n",
            shader_source_path, infolog);
    }

    return shader;
}

GLint LinkShader(uint32_t shader_count, ...)
{
    // create program
    GLint program = glCreateProgram();

    // parse argumenr list and attach shaders
    va_list arg_list;
    va_start(arg_list, shader_count);
    for (uint32_t i = 0; i < shader_count; i++)
    {
        glAttachShader(program, va_arg(arg_list, GLint));
    }
    va_end(arg_list);

    // link program
    glLinkProgram(program);

    // check for linking errors
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == 0)
    {
        char infolog[512];
        glGetProgramInfoLog(program, 512, NULL, infolog);
        fprintf(stderr,
                "Failed to link shaders, gathering information...\n%s\n",
                infolog);
    }

    return program;
}

#endif // #ifdef GL_UTILS_INCLUDE_IMPLEMENTATION
