#ifndef SHADERS_H
#define SHADERS_H

#include <GLFW/glfw3.h>

class Shaders
{
public:
    static void init();

    static GLuint ship;
    static GLuint acceleration;
    static GLuint velocity;
    static GLuint position;
    static GLuint copy;
    static GLuint RK4sum;
private:
    static GLuint CompileShader(const std::string& filename);
    static GLuint CreateProgram(const GLuint vert, const GLuint frag);
};

#endif
