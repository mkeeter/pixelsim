#include <cstdint>
#include <cstring>  // memset

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
}

////////////////////////////////////////////////////////////////////////////////

Ship::~Ship()
{
    delete [] data;
    glDeleteBuffers(1, &vertex_buf);
    glDeleteBuffers(1, &color_buf);
    glDeleteBuffers(1, &rect_buf);

    glDeleteTextures(1, &filled_tex);
    glDeleteTextures(1, &output_tex);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void Ship::Draw(const int window_width, const int window_height) const
{

    if (0) {
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

        glDrawArrays(GL_TRIANGLES, 0, pixel_count*2*3);
    }

    if (1) {
        glViewport(0, 0, width, height);
        const GLuint program = Shaders::texture;
        glUseProgram(program);

        glBindBuffer(GL_ARRAY_BUFFER, rect_buf);
        const GLint v = glGetAttribLocation(program, "vertex_position");
        glEnableVertexAttribArray(v);
        glVertexAttribPointer(v, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, filled_tex);
        glUniform1i(glGetUniformLocation(program, "texture"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 12);
    }

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
                vertices.push_back(y-0.5);

                vertices.push_back(x+0.5);
                vertices.push_back(y-0.5);

                vertices.push_back(x+0.5);
                vertices.push_back(y+0.5);

                // Second triangle
                vertices.push_back(x+0.5);
                vertices.push_back(y+0.5);

                vertices.push_back(x-0.5);
                vertices.push_back(y+0.5);

                vertices.push_back(x-0.5);
                vertices.push_back(y-0.5);

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

    glGenFramebuffers(1, &fbo);
}

void Ship::MakeTextures()
{
    {
        GLubyte* const filled = new GLubyte[width*height];
        for (size_t i=0; i < width*height; ++i) {
            filled[i] = data[i*4 + 3] ? 255 : 0;
        }
        glGenTextures(1, &filled_tex);
        glBindTexture(GL_TEXTURE_2D, filled_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, 1, width, height,
                     0, GL_RED, GL_UNSIGNED_BYTE, filled);
        SetTextureDefaults();
        delete [] filled;
    }

    {
        GLubyte* const empty = new GLubyte[width*height];
        for (size_t i=0; i < width*height; ++i)     empty[i] = 0;
        glGenTextures(1, &output_tex);
        glBindTexture(GL_TEXTURE_2D, output_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, 1, width, height,
                0, GL_RED, GL_UNSIGNED_BYTE, empty);
        SetTextureDefaults();
        delete [] empty;
    }
}

void Ship::SetTextureDefaults() const
{
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
