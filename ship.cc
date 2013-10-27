#include <cstdint>
#include <cstring>  // memset
#include <cmath>

#include <iostream>
#include <vector>

#include <png.h>

#include <GLFW/glfw3.h>
#include <OpenGL/glext.h>

#include "ship.h"
#include "shaders.h"

////////////////////////////////////////////////////////////////////////////////

Ship::Ship(const std::string& imagename)
{
    LoadImage(imagename);
    MakeTextures();
    MakeBuffers();
    MakeFramebuffers();
}

////////////////////////////////////////////////////////////////////////////////

Ship::~Ship()
{
    delete [] data;
    glDeleteBuffers(1, &vertex_buf);
    glDeleteBuffers(1, &color_buf);
    glDeleteBuffers(1, &rect_buf);

    GLuint* textures[] = {&filled_tex, &pos_tex[0], &pos_tex[1],
                          &vel_tex[1], &vel_tex[0], &accel_tex};
    for (auto t : textures)     glDeleteTextures(1, t);

    glDeleteFramebuffers(1, &fbo);
}

////////////////////////////////////////////////////////////////////////////////

void Ship::UpdateAcceleration()
{
    const GLuint program = Shaders::acceleration;
    glUseProgram(program);

    // Load boolean occupancy texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, filled_tex);
    glUniform1i(glGetUniformLocation(program, "filled"), 0);

    // Load RGB32F position and velocity textures
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pos_tex[tick]);
    glUniform1i(glGetUniformLocation(program, "pos"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, vel_tex[tick]);
    glUniform1i(glGetUniformLocation(program, "vel"), 2);

    // Load various uniform values
    glUniform2i(glGetUniformLocation(program, "ship_size"), width, height);
    glUniform1f(glGetUniformLocation(program, "k_linear"), 1.0f);
    glUniform1f(glGetUniformLocation(program, "k_torsional"), 1.0f);
    glUniform1f(glGetUniformLocation(program, "c_linear"), 0.0f);
    glUniform1f(glGetUniformLocation(program, "c_torsional"), 0.0f);
    glUniform1f(glGetUniformLocation(program, "m"), 1.0f);
    glUniform1f(glGetUniformLocation(program, "I"), 1.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, accel_tex, 0);
    DrawRect(program);

    // Switch back to the normal rendering framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Ship::UpdateVelocity(const float dt)
{
    const GLuint program = Shaders::velocity;
    glUseProgram(program);

    // Bind old velocity texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vel_tex[tick]);
    glUniform1i(glGetUniformLocation(program, "vel"), 0);

    // Bind acceleration texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, accel_tex);
    glUniform1i(glGetUniformLocation(program, "accel"), 1);

    // Set time-step value
    glUniform1f(glGetUniformLocation(program, "dt"), dt);

    // Bind the framebuffer to the other velocity texture and render.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, vel_tex[!tick], 0);
    DrawRect(program);

    // Switch back to the normal rendering framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Ship::UpdatePosition(const float dt)
{
    const GLuint program = Shaders::position;
    glUseProgram(program);

    // Bind old position texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pos_tex[tick]);
    glUniform1i(glGetUniformLocation(program, "pos"), 0);

    // Bind velocity texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, vel_tex[tick]);
    glUniform1i(glGetUniformLocation(program, "vel"), 1);

    // Set time-step value
    glUniform1f(glGetUniformLocation(program, "dt"), dt);

    // Bind the framebuffer to the other position texture and render.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, pos_tex[!tick], 0);
    DrawRect(program);

    // Switch back to the normal rendering framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

////////////////////////////////////////////////////////////////////////////////

void Ship::DrawRect(const GLuint program)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);

    // Load a rectangle from -1, -1, to 1, 1
    glBindBuffer(GL_ARRAY_BUFFER, rect_buf);
    const GLint v = glGetAttribLocation(program, "vertex_position");
    glEnableVertexAttribArray(v);
    glVertexAttribPointer(v, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);

    // Draw the full rectangle into the FBO, which is bound to accel_tex
    glDrawArrays(GL_TRIANGLES, 0, 6);
}



void Ship::Update()
{
    UpdateAcceleration();
    UpdatePosition(0.01f);
    UpdateVelocity(0.01f);
    tick = !tick;   // switch buffers
}

////////////////////////////////////////////////////////////////////////////////

void Ship::Draw(const int window_width, const int window_height) const
{
    glViewport(0, 0, window_width, window_height);

#if 1
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    const GLuint program = Shaders::ship;
    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
    const GLint v = glGetAttribLocation(program, "vertex_position");
    glEnableVertexAttribArray(v);
    glVertexAttribPointer(v, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, color_buf);
    const GLint c = glGetAttribLocation(program, "color_in");
    glEnableVertexAttribArray(c);
    glVertexAttribPointer(c, 3, GL_UNSIGNED_BYTE, GL_TRUE, 3*sizeof(GLbyte), 0);

    const GLint w = glGetUniformLocation(program, "window_size");
    glUniform2i(w, window_width, window_height);

    const GLint s = glGetUniformLocation(program, "ship_size");
    glUniform2i(s, width, height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pos_tex[tick]);
    glUniform1i(glGetUniformLocation(program, "pos"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, filled_tex);
    glUniform1i(glGetUniformLocation(program, "filled"), 1);

    glDrawArrays(GL_TRIANGLES, 0, pixel_count*2*3);
#endif

#if 0
    const GLuint program = Shaders::texture;
    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, rect_buf);
    const GLint v = glGetAttribLocation(program, "vertex_position");
    glEnableVertexAttribArray(v);
    glVertexAttribPointer(v, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, accel_tex);
    glUniform1i(glGetUniformLocation(program, "texture"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
#endif

}

////////////////////////////////////////////////////////////////////////////////

// Minimal function to load a .png image.
// Does no error checking: assumes that the file exists and
// is an eight-bit RGBA png image.
void Ship::LoadImage(const std::string& imagename)
{
    png_structp png_ptr = png_create_read_struct(
            PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    FILE* input = fopen(imagename.c_str(), "rb");
    png_init_io(png_ptr, input);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    fclose(input);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    data = new uint8_t[width*height*4];
    png_bytep* rows = png_get_rows(png_ptr, info_ptr);
    for (size_t j=0; j < height; ++j) {
        memmove(&data[j*width*4], rows[j], width*4);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

////////////////////////////////////////////////////////////////////////////////

void Ship::MakeBuffers()
{
    std::vector<GLfloat> vertices;
    std::vector<GLbyte>  colors;

    for (size_t y=0; y < height; ++y) {
        for (size_t x=0; x < width; ++x) {
            if (data[y*width*4 + x*4 + 3]) {
                // First triangle
                vertices.push_back(x-0.5);
                vertices.push_back(height-y-1.5);

                vertices.push_back(x+0.5);
                vertices.push_back(height-y-1.5);

                vertices.push_back(x+0.5);
                vertices.push_back(height-y-0.5);

                // Second triangle
                vertices.push_back(x+0.5);
                vertices.push_back(height-y-0.5);

                vertices.push_back(x-0.5);
                vertices.push_back(height-y-0.5);

                vertices.push_back(x-0.5);
                vertices.push_back(height-y-1.5);

                // Every vertex gets a color and initial position
                for (int v=0; v < 6; ++v) {
                    colors.push_back(data[y*width*4 + x*4]);
                    colors.push_back(data[y*width*4 + x*4 + 1]);
                    colors.push_back(data[y*width*4 + x*4 + 2]);
                }
            }
        }
    }

    // Save the total number of filled pixels
    pixel_count = vertices.size() / 12;

    // Allocate space for the vertices, colors, and position data
    glGenBuffers(1, &vertex_buf);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vertices[0]),
                 &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &color_buf);
    glBindBuffer(GL_ARRAY_BUFFER, color_buf);
    glBufferData(GL_ARRAY_BUFFER, colors.size()*sizeof(colors[0]),
                 &colors[0], GL_STATIC_DRAW);

    GLfloat rect[12] = {
            -1, -1,
             1, -1,
             1,  1,
            -1, -1,
             1,  1,
            -1, 1};
    glGenBuffers(1, &rect_buf);
    glBindBuffer(GL_ARRAY_BUFFER, rect_buf);
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(rect[0]),
                 &rect[0], GL_STATIC_DRAW);

}

void Ship::MakeTextures()
{
    {   // Load a byte-map recording occupancy
        // Bytes are byte-aligned, so set unpack alignment to 1
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLubyte* const filled = new GLubyte[width*height];
        size_t i=0;
        for (size_t y=0; y < height; ++y) {
            for (size_t x=0; x < width; ++x) {
                filled[i++] = data[4*(width*(height-1-y) + x) + 3] ? 255 : 0;
            }
        }
        glGenTextures(1, &filled_tex);
        glBindTexture(GL_TEXTURE_2D, filled_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height,
                     0, GL_RED, GL_UNSIGNED_BYTE, filled);
        SetTextureDefaults();
        delete [] filled;
    }


    {   // Make a texture that stores position alone, and initialize it with
        // each pixel centered in the proper position.

        // Floats are 4-byte-aligned.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        GLfloat* const pos = new GLfloat[width*height*3];
        size_t i=0;
        for (size_t y=0; y < height; ++y) {
            for (size_t x=0; x < width; ++x) {
                pos[i++] = x + (x == 0 && y == 0 ? -0.5 : 0);// + ((rand() % 100) - 50) / 100.;
                pos[i++] = y + (x == 0 && y == 0 ? -0.5 : 0);// + ((rand() % 100) - 50) / 100.;
                pos[i++] = 0;
            }
        }
        // Custom settings for 2x1 image
        /*
        pos[0] = 0;
        pos[1] = 0;
        pos[2] = 0.1;
        pos[3] = 1;
        pos[4] = 0;
        pos[5] = 0;*/

        GLuint* textures[] = {&pos_tex[0], &pos_tex[1]};
        for (auto t : textures)
        {
            glGenTextures(1, t);
            glBindTexture(GL_TEXTURE_2D, *t);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, width, height,
                    0, GL_RGB, GL_FLOAT, pos);
            SetTextureDefaults();
        }
        delete [] pos;
    }

    {   // Make a set of float textures storing position, velocity,
        // and acceleration.

        // Floats are 4-byte-aligned.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        GLfloat* const empty = new GLfloat[width*height*3];
        for (size_t i=0; i < width*height*3; i++)   empty[i] = 0;

        GLuint* textures[] = {&vel_tex[0], &vel_tex[1], &accel_tex};

        for (auto t: textures) {
            glGenTextures(1, t);
            glBindTexture(GL_TEXTURE_2D, *t);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, width, height,
                    0, GL_RGB, GL_FLOAT, empty);
            SetTextureDefaults();
        }
        delete [] empty;
    }
}

void Ship::MakeFramebuffers()
{
    glGenFramebuffers(1, &fbo);
}

void Ship::SetTextureDefaults() const
{
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
