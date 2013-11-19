#include <cstdint>
#include <cstring>  // memset
#include <cmath>

#include <iostream>
#include <vector>

#include <png.h>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include "ship.h"
#include "shaders.h"

////////////////////////////////////////////////////////////////////////////////

Ship::Ship(const std::string& imagename)
{
    LoadImage(imagename);
    MakeTextures();
    MakeBuffers();
    MakeFramebuffer();
    MakeVertexArray();
}

////////////////////////////////////////////////////////////////////////////////

Ship::~Ship()
{
    delete [] data;
    glDeleteBuffers(1, &vertex_buf);
    glDeleteBuffers(1, &color_buf);
    glDeleteBuffers(1, &rect_buf);

    GLuint* textures[] = {
        &filled_tex, &pos_tex[0],&pos_tex[1], &vel_tex[0], &vel_tex[1],
        &dpos_tex[0], &dpos_tex[1], &dpos_tex[2], &dpos_tex[3],
        &dvel_tex[0], &dvel_tex[1], &dvel_tex[2], &dvel_tex[3]
    };

    for (auto t : textures)     glDeleteTextures(1, t);

    glDeleteFramebuffers(1, &fbo);
    glDeleteVertexArrays(1, &vao);
}

////////////////////////////////////////////////////////////////////////////////

void Ship::GetDerivatives(const int source, const int out)
{
    GetAcceleration(source, out);
    GetVelocity(source, out);
}

void Ship::GetAcceleration(const int source, const int accel_out)
{
    const GLuint program = Shaders::acceleration;
    glUseProgram(program);

    // Load boolean occupancy texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, filled_tex);
    glUniform1i(glGetUniformLocation(program, "filled"), 0);

    // Load RGB32F position and velocity textures
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pos_tex[source]);
    glUniform1i(glGetUniformLocation(program, "pos"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, vel_tex[source]);
    glUniform1i(glGetUniformLocation(program, "vel"), 2);

    // Load various uniform values
    glUniform2i(glGetUniformLocation(program, "ship_size"), width, height);

    glUniform1f(glGetUniformLocation(program, "k"), 10000.0f);
    glUniform1f(glGetUniformLocation(program, "c"), 100.0f);
    glUniform1f(glGetUniformLocation(program, "m"), 1.0f);
    glUniform1f(glGetUniformLocation(program, "I"), 1.0f);

    RenderToFBO(program, dvel_tex[accel_out]);
}

void Ship::GetVelocity(const int source, const int vel_out)
{
    const GLuint program = Shaders::copy;
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vel_tex[source]);
    glUniform1i(glGetUniformLocation(program, "texture"), 0);

    RenderToFBO(program, dpos_tex[vel_out]);
}


////////////////////////////////////////////////////////////////////////////////

void Ship::ApplyDerivatives(const float dt, const int source)
{
    ApplyAcceleration(dt, source);
    ApplyVelocity(dt, source);
}

void Ship::ApplyAcceleration(const float dt, const int source)
{
    const GLuint program = Shaders::velocity;
    glUseProgram(program);

    // Bind old velocity texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vel_tex[tick]);
    glUniform1i(glGetUniformLocation(program, "vel"), 0);

    // Bind acceleration texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, dvel_tex[source]);
    glUniform1i(glGetUniformLocation(program, "accel"), 1);

    // Set time-step value
    glUniform1f(glGetUniformLocation(program, "dt"), dt);

    RenderToFBO(program, vel_tex[!tick]);
}

void Ship::ApplyVelocity(const float dt, const int source)
{
    const GLuint program = Shaders::position;
    glUseProgram(program);

    // Bind old position texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pos_tex[tick]);
    glUniform1i(glGetUniformLocation(program, "pos"), 0);

    // Bind velocity texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, dpos_tex[source]);
    glUniform1i(glGetUniformLocation(program, "vel"), 1);

    // Set time-step value
    glUniform1f(glGetUniformLocation(program, "dt"), dt);

    RenderToFBO(program, pos_tex[!tick]);
}

////////////////////////////////////////////////////////////////////////////////

void Ship::Update(const float dt, const int steps)
{
#if 0
    float tex[(width+1)*(height+1)*3];
    glBindTexture(GL_TEXTURE_2D, pos_tex[tick]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &tex);
    std::cout << "Positions:\n";
    for (int i=0; i < (width+1)*(height+1)*3; i += 3)
    {
        std::cout << tex[i] << ',' << tex[i+1] << ',' << tex[i+2] << "    ";
    }
    std::cout << std::endl;
    glBindTexture(GL_TEXTURE_2D, vel_tex[tick]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &tex);
    std::cout << "Velocities:\n";
    for (int i=0; i < (width+1)*(height+1)*3; i += 3)
    {
        std::cout << tex[i] << ',' << tex[i+1] << ',' << tex[i+2] << "    ";
    }
    std::cout << std::endl;
    GetDerivatives(tick, 0);
    glBindTexture(GL_TEXTURE_2D, dvel_tex[0]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, &tex);
    std::cout << "Accelerations:\n";
    for (int i=0; i < (width+1)*(height+1)*3; i += 3)
    {
        std::cout << tex[i] << ',' << tex[i+1] << ',' << tex[i+2] << "    ";
    }
    std::cout << std::endl << std::endl;
#endif

    const float dt_ = dt / steps;
    for (int i=0; i < steps; ++i) {
        GetDerivatives(tick, 0);    // k1 = f(y)

        ApplyDerivatives(dt_/2, 0);  // Calculate y + dt/2 * k1
        GetDerivatives(!tick, 1);   // k2 = f(y + dt/2 * k1)

        ApplyDerivatives(dt_/2, 1);  // Calculate y + dt/2 * k2
        GetDerivatives(!tick, 2);   // k3 = f(y + dt/2 * k2)

        ApplyDerivatives(dt_, 2);    // Calculate y + dt * k3
        GetDerivatives(!tick, 3);   // k4 = f(y + dt * k3)

        // Update state and swap which texture is active
        GetNextState(dt_);
    }
}

////////////////////////////////////////////////////////////////////////////////

void Ship::GetNextState(const float dt)
{
    GetRK4Sum(pos_tex, dpos_tex, dt);
    GetRK4Sum(vel_tex, dvel_tex, dt);
    tick = !tick;
}

void Ship::GetRK4Sum(GLuint* state, GLuint* derivatives, const float dt)
{
    const GLuint program = Shaders::RK4sum;
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state[tick]);
    glUniform1i(glGetUniformLocation(program, "y"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, derivatives[0]);
    glUniform1i(glGetUniformLocation(program, "k1"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, derivatives[1]);
    glUniform1i(glGetUniformLocation(program, "k2"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, derivatives[2]);
    glUniform1i(glGetUniformLocation(program, "k3"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, derivatives[3]);
    glUniform1i(glGetUniformLocation(program, "k4"), 4);

    glUniform1f(glGetUniformLocation(program, "dt"), dt);

    RenderToFBO(program, state[!tick]);
}

////////////////////////////////////////////////////////////////////////////////

void Ship::Draw(const int window_width, const int window_height) const
{
    glViewport(0, 0, window_width, window_height);

    const GLuint program = Shaders::ship;
    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, color_buf);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, 3*sizeof(GLbyte), 0);

    glUniform2i(glGetUniformLocation(program, "window_size"),
                window_width, window_height);

    glUniform2i(glGetUniformLocation(program, "ship_size"),
                width, height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pos_tex[tick]);
    glUniform1i(glGetUniformLocation(program, "pos"), 0);

    glDrawArrays(GL_TRIANGLES, 0, pixel_count*2*3);
}

////////////////////////////////////////////////////////////////////////////////

void Ship::RenderToFBO(const GLuint program, const GLuint tex)
{
    // Bind the desired texture to the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, tex, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width+1, height+1);

    // Load triangles that draw a flat rectangle from -1, -1, to 1, 1
    glBindBuffer(GL_ARRAY_BUFFER, rect_buf);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);

    // Draw the full rectangle into the FBO
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Switch back to the default framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
                vertices.push_back(x);
                vertices.push_back(height-y-1);

                vertices.push_back(x+1);
                vertices.push_back(height-y-1);

                vertices.push_back(x+1);
                vertices.push_back(height-y);

                // Second triangle
                vertices.push_back(x+1);
                vertices.push_back(height-y);

                vertices.push_back(x);
                vertices.push_back(height-y);

                vertices.push_back(x);
                vertices.push_back(height-y-1);

                // Every vertex gets a color from the image
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


    // Make a screen-filling flat pane used for texture FBO rendering
    GLfloat rect[12] = {
            -1, -1,
             1, -1,
             1,  1,
            -1, -1,
             1,  1,
            -1,  1};
    glGenBuffers(1, &rect_buf);
    glBindBuffer(GL_ARRAY_BUFFER, rect_buf);
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(rect[0]),
                 &rect[0], GL_STATIC_DRAW);

}

////////////////////////////////////////////////////////////////////////////////

void Ship::MakeTextures()
{
    {   // Load a byte-map recording occupancy
        // Bytes are byte-aligned, so set unpack alignment to 1
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLubyte* const filled = new GLubyte[(width+1)*(height+1)];
        memset(filled, 0, sizeof(GLubyte)*(width+1)*(height+1));

        for (size_t y=0; y < height; ++y) {
            for (size_t x=0; x < width; ++x) {
                // Get the pixel's address in the data array:
                uint8_t* const pixel = &data[4*(width*(height-1-y) + x)];

                // Any pixel with an alpha value > 0 is filled.
                const bool pixel_filled = pixel[3] != 0;

                // Only pixels that are pure red count as engines.
                const bool pixel_engine = filled && pixel[0] == 255 &&
                                          pixel[1] == 0  && pixel[2] == 0;

                const size_t indices[] = {
                        y*(width+1) + x, (y+1)*(width+1) + x,
                        y*(width+1) + x + 1, (y+1)*(width+1) + x + 1};


                for (size_t i : indices) {
                    if (!filled[i]) {
                        if (pixel_engine)       filled[i] = 255;
                        else if (pixel_filled)  filled[i] = 128;
                    } else if (filled[i] == 255 && !pixel_engine) {
                        filled[i] = 128;
                    }
                }
            }
        }
        glGenTextures(1, &filled_tex);
        glBindTexture(GL_TEXTURE_2D, filled_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width+1, height+1,
                     0, GL_RED, GL_UNSIGNED_BYTE, filled);
        SetTextureDefaults();
        delete [] filled;
    }


    {   // Make a texture that stores position alone, and initialize it with
        // each pixel centered in the proper position.

        // Floats are 4-byte-aligned.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        GLfloat* const pos = new GLfloat[(width+1)*(height+1)*2];
        size_t i=0;
        for (size_t y=0; y <= height; ++y) {
            for (size_t x=0; x <= width; ++x) {
                pos[i++] = x;
                pos[i++] = y;
            }
        }

        GLuint* textures[] = {&pos_tex[0], &pos_tex[1]};
        for (auto t : textures)
        {
            glGenTextures(1, t);
            glBindTexture(GL_TEXTURE_2D, *t);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width+1, height+1,
                    0, GL_RG, GL_FLOAT, pos);
            SetTextureDefaults();
        }
        delete [] pos;
    }

    {   // Make a set of float textures storing position, velocity,
        // and acceleration.

        // Floats are 4-byte-aligned.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        GLfloat* const empty = new GLfloat[(width+1)*(height+1)*2];
        for (size_t i=0; i < (width+1)*(height+1)*2; i++)   empty[i] = 0;

        GLuint* textures[] = {&vel_tex[0], &vel_tex[1],
                              &dvel_tex[0], &dvel_tex[1],
                              &dvel_tex[2], &dvel_tex[3],
                              &dpos_tex[0], &dpos_tex[1],
                              &dpos_tex[2], &dpos_tex[3]};

        for (auto t: textures) {
            glGenTextures(1, t);
            glBindTexture(GL_TEXTURE_2D, *t);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width+1, height+1,
                    0, GL_RG, GL_FLOAT, empty);
            SetTextureDefaults();
        }
        delete [] empty;
    }
}

void Ship::MakeFramebuffer()
{
    glGenFramebuffers(1, &fbo);
}

void Ship::MakeVertexArray()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); // we'll use this VAO all the time
}

void Ship::SetTextureDefaults() const
{
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
