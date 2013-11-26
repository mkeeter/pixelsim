#include <iostream>
#include <fstream>

#include <cassert>

#include "shaders.h"

std::string Shaders::constants;

GLuint Shaders::ship = 0;
GLuint Shaders::derivatives = 0;
GLuint Shaders::euler = 0;
GLuint Shaders::RK4sum = 0;

////////////////////////////////////////////////////////////////////////////////

void Shaders::init()
{
    // Load "constants.h" as a string.  We'll paste this string
    // into all of the shaders (to #define a bunch of macros).
    std::ifstream c("constants.h");
    std::string line;
    std::string program;
    while (getline(c, line))    constants += line + '\n';

    ship        = CreateProgram(CompileShader("ship.vert"),
                                CompileShader("ship.frag"));
    derivatives = CreateProgram(CompileShader("texture.vert"),
                                CompileShader("derivatives.frag"));
    euler       = CreateProgram(CompileShader("texture.vert"),
                                CompileShader("euler.frag"));
    RK4sum      = CreateProgram(CompileShader("texture.vert"),
                                CompileShader("rk4.frag"));
}

GLuint Shaders::CompileShader(const std::string& filename)
{
    const std::string extension = filename.substr(filename.find_last_of("."));
    assert(extension == ".vert" || extension == ".frag");

    GLenum type = extension == ".vert" ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    GLuint shader = glCreateShader(type);

    std::ifstream t(filename.c_str());
    std::string line;
    std::string program;

    // Get the first line (the #version directive)
    getline(t, program);
    // Then add all of the constants defined in constants.h
    program += '\n' + constants;

    // Finally read the rest of the shader in
    while (getline(t, line))    program += line + '\n';

    const char* txt = program.c_str();
    glShaderSource(shader, 1, &txt, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar* strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

        std::cerr << "Compile failure in "
                  << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
                  << " shader (" << filename << "):\n" << strInfoLog << std::endl;
        delete [] strInfoLog;
    }

    return shader;
}

GLuint Shaders::CreateProgram(const GLuint vert, const GLuint frag)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar* strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);

        std::cerr << "Linker failure: " << strInfoLog << std::endl;
        delete [] strInfoLog;
    }

    return program;
}
