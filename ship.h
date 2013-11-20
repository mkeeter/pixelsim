#ifndef SHIP_H
#define SHIP_H

#include <GLFW/glfw3.h>

#include <string>

class Ship
{
public:
    Ship(const std::string& imagename);
    ~Ship();

    void BoostOn()  { boost = true; }
    void BoostOff() { boost = false; }

    void Update(const float dt=0.1, const int steps=5);
    void Draw(const int window_width, const int window_height) const;
private:
    void MakeBuffers();
    void LoadImage(const std::string& imagename);
    void MakeTextures();
    void MakeFramebuffer();
    void MakeVertexArray();

    void SetTextureDefaults() const;

    // From pos[source] and vel[source], calculate and store derivatives
    void GetDerivatives(const int source, const int out);

    // From pos[source] and vel[tick], calculate dvel and store in
    // dvel[accel_out]
    void GetAcceleration(const int source, const int accel_out);

    // From vel[source], calculate dpos (hint: it's the same thing)
    // and store in dpos[vel_out]
    void GetVelocity(const int source, const int vel_out);

    // Applies derivatives in the given slot, storing new position
    // and velocity in tock slot.
    void ApplyDerivatives(const float dt, const int source);

    // Store an updated velocity in vel_tex[tock]
    void ApplyAcceleration(const float dt, const int source);

    // Store an updated position in pos_tex[tock]
    void ApplyVelocity(const float dt, const int source);

    void GetNextState(const float dt);
    void GetRK4Sum(GLuint* state, GLuint* derivatives, const float dt);

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

    GLuint pos_tex[2];     // position & rotation of each pixel
    GLuint vel_tex[2];     // velocity of each pixel

    GLuint dpos_tex[4];    // derivative of position (buffers for RK4)
    GLuint dvel_tex[4];    // derivaties of velocity (buffers for RK4)

    bool tick;

    // Frame-buffer object
    GLuint fbo;

    // Vertex array object
    GLuint vao;

    bool boost;
};

#endif
