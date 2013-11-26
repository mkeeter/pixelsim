#ifndef SHIP_H
#define SHIP_H

#include <GLFW/glfw3.h>

#include <string>

#include "constants.h"

class Ship
{
public:
    Ship(const std::string& imagename, const bool pinned);
    ~Ship();

    bool thrustEnginesOn;
    bool leftEnginesOn;
    bool rightEnginesOn;

    void Update(const float dt=0.1, const int steps=5);
    void Draw(const int window_width, const int window_height) const;

private:
    enum NodeType {EMPTY=0, SHIP=1,
                   THRUST=SHIP_ENGINE_THRUST,
                   LEFT  =SHIP_ENGINE_LEFT,
                   RIGHT =SHIP_ENGINE_RIGHT};

    void MakeBuffers();
    void LoadImage(const std::string& imagename);
    void MakeTextures();
    void MakeFramebuffer();
    void MakeVertexArray();

    void SetTextureDefaults() const;

    // Calculate derivatives of state_tex[source], storing them
    // in derivative_tex[out]
    void GetDerivatives(const int source, const int out);

    // Applies derivative_tex[source] to state_tex[tick], storing
    // new state in state_tex[!tick]
    void ApplyDerivatives(const float dt, const int source);

    void GetNextState(const float dt);

    void RenderToFBO(const GLuint program, const GLuint tex);

    void PrintTextureValues();

    size_t width;
    size_t height;
    uint8_t* data;

    // Number of filled pixels
    size_t pixel_count;

    // Buffers
    GLuint vertex_buf;
    GLuint color_buf;
    GLuint rect_buf;

    // Textures
    GLuint filled_tex;  // boolean storing occupancy

    GLuint state_tex[2];   // position & velocity of each pixel
    GLuint derivative_tex[4]; // derivatives of position and velocity (for RK4)

    bool tick;

    // Frame-buffer object
    GLuint fbo;

    // Vertex array object
    GLuint vao;

    const bool pinned;
};

#endif
