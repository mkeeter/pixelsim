#ifndef SHIP_H
#define SHIP_H

#include <GLFW/glfw3.h>

#include <string>

class Ship
{
public:
    Ship(const std::string& imagename);
    ~Ship();
    void Update();
    void Draw(const int window_width, const int window_height) const;
private:
    void MakeBuffers();
    void LoadImage(const std::string& imagename);
    void MakeTextures();
    void MakeFramebuffers();

    void SetTextureDefaults() const;

    void UpdateAcceleration();
    void UpdateVelocity(const float dt);
    void UpdatePosition(const float dt);
    void DrawRect(const GLuint program);

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

    GLuint accel_tex;   // acceleration of each pixel

    bool tick;

    // Frame-buffer object
    GLuint fbo;
};

#endif
