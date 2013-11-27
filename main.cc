#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

#include <GLFW/glfw3.h>

#include <png.h>

#include "ship.h"
#include "shaders.h"

////////////////////////////////////////////////////////////////////////////////

struct WindowSize
{
    WindowSize(int w, int h) : width(w), height(h) {}
    int width, height;
};

struct State
{
    State(WindowSize* ws, Ship* s) :
        window_size(ws), ship(s) {}

    WindowSize* window_size;
    Ship*       ship;
};

////////////////////////////////////////////////////////////////////////////////

void resize_callback(GLFWwindow* window, int width, int height)
{
    WindowSize* ws = static_cast<State*>(
            glfwGetWindowUserPointer(window))->window_size;
    ws->width = width;
    ws->height = height;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Ship* ship = static_cast<State*>(
            glfwGetWindowUserPointer(window))->ship;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_UP)
    {
        if (action == GLFW_PRESS)           ship->thrustEnginesOn = true;
        else if (action == GLFW_RELEASE)    ship->thrustEnginesOn = false;
    }
    else if (key == GLFW_KEY_LEFT)
    {
        if (action == GLFW_PRESS)           ship->leftEnginesOn = true;
        else if (action == GLFW_RELEASE)    ship->leftEnginesOn = false;
    }
    else if (key == GLFW_KEY_RIGHT)
    {
        if (action == GLFW_PRESS)           ship->rightEnginesOn = true;
        else if (action == GLFW_RELEASE)    ship->rightEnginesOn = false;
    }
}

////////////////////////////////////////////////////////////////////////////////

void SaveImage(const std::string& filename,
               const size_t width, const size_t height)
{
    png_structp png_ptr = png_create_write_struct(
            PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    FILE* output = fopen(filename.c_str(), "wb");

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    GLubyte* pixels = new GLubyte[width*height*3];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    GLubyte** rows = new GLubyte*[height];
    for (size_t i=0; i < height; ++i)
    {
        rows[i] = &pixels[(height - i - 1)*width*3];
    }

    png_init_io(png_ptr, output);
    png_set_rows(png_ptr, info_ptr, rows);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    fclose(output);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    delete [] rows;
    delete [] pixels;
}

////////////////////////////////////////////////////////////////////////////////

void GetArgs(int argc, char** argv,
             std::string* filename, WindowSize* window_size,
             bool* record, bool* track, float* scale)
{
    if (argc < 2)
    {
        std::cout << "Usage: pixelsim [...] image.png" << std::endl;
        exit(-1);
    }

    // The last argument is the target filename.
    *filename = argv[--argc];
    // Verify that it at least ends in ".png"
    if (filename->rfind(".png") != filename->length() - 4)
    {
        std::cerr << "[pixelsim]    Error: Invalid image name '"
                  << *filename << "'" << std::endl;
        exit(-1);
    }

    // Attempt to open the file to verify that it exists.
    FILE* input = fopen(filename->c_str(), "rb");
    if (input == NULL)
    {
        std::cerr << "[pixelsim]    Error: Cannot open file '"
                  << *filename << "'" << std::endl;
        exit(-1);
    }
    fclose(input);


    // Parse any other arguments that may have been provided.
    for (int a=1; a < argc; ++a)
    {
        if (!strcmp(argv[a], "--size"))
        {
            if (++a >= argc)
            {
                std::cerr << "[pixelsim]    Error: No size provided!"
                          << std::endl;
                exit(-1);
            }

            const std::string size = argv[a];
            const size_t x = size.find("x");
            if (x == std::string::npos)
            {
                std::cerr << "[pixelsim]    Error: Invalid size specification '"
                          << size << "'" << std::endl;
                exit(-1);
            }

            const std::string w = size.substr(0, x);
            const std::string h = size.substr(x + 1, size.size() - x - 1);

            window_size->width = std::atoi(w.c_str());
            window_size->height = std::atoi(h.c_str());

            if (window_size->height == 0 || window_size->width == 0)
            {
                std::cerr << "[pixelsim]    Error: Invalid size specification '"
                          << size << "'" << std::endl;
                exit(-1);
            }
        }
        else if (!strcmp(argv[a], "--scale"))
        {
            if (++a >= argc)
            {
                std::cerr << "[pixelsim]    Error: No scale provided!"
                          << std::endl;
                exit(-1);
            }
            *scale = std::atof(argv[a]);
            if (*scale == 0)
            {
                std::cerr << "[pixelsim]    Error: Invalid scale specification '"
                          << argv[a] << "'" << std::endl;
                exit(-1);
            }
        }
        else if (!strcmp(argv[a], "--record"))
        {
            *record = true;
        }
        else if (!strcmp(argv[a], "--track"))
        {
            *track = true;
        }
        else
        {
            std::cerr << "[pixelsim]    Error: Unrecognized argument '"
                      << argv[a] << "'" << std::endl;
            exit(-1);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    WindowSize window_size(640, 480);

    bool record = false;
    bool track  = false;
    float scale = 0.9;
    std::string filename;
    GetArgs(argc, argv, &filename, &window_size, &record, &track, &scale);

    // Initialize the library
    if (!glfwInit())    return -1;

    glfwWindowHint(GLFW_SAMPLES, 8);    // multisampling!
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* const window = glfwCreateWindow(
            window_size.width, window_size.height, "pixelsim", NULL, NULL);

    if (!window)
    {
        std::cerr << "[pixelsim]    Error: failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize the ship!
    Ship ship(filename);
    Shaders::init();

    // Store pointers to window and ship objects.  They will be
    // modified by resize and key-press callbacks, respectively.
    State state(&window_size, &ship);

    // Use a callback to update glViewport when the window is resized, by
    // saving a pointer to a WindowSize struct in the window's user pointer
    // field.
    glfwSetWindowUserPointer(window, static_cast<void*>(&state));
    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwSetKeyCallback(window, key_callback);

    // Get the actual framebuffer size (so that it works properly on
    // retina displays, rather than only filling 1/4 of the window)
    glfwGetFramebufferSize(window, &window_size.width, &window_size.height);

    // Loop until the user closes the window
    size_t frame=0;
    while (!glfwWindowShouldClose(window))
    {
        // Store the start time of this update loop
        const auto t0 = std::chrono::high_resolution_clock::now();

        // Update the ship
        ship.Update(1.0e0/60, 50);

        // Draw the scene
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ship.Draw(window_size.width, window_size.height, track, scale);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();

        // Sleep to maintain a framerate of 60 FPS
        std::this_thread::sleep_for(
                std::chrono::microseconds(std::micro::den / 61) -
                (std::chrono::high_resolution_clock::now() - t0));

        // Print the FPS (for debugging)
        std::cout << std::chrono::seconds(1) /
                     (std::chrono::high_resolution_clock::now() - t0)
                  << std::endl;

        if (record && frame)
        {
            std::stringstream ss;
            ss << std::setw(3) << std::setfill('0') << frame;
            SaveImage("frames/" + ss.str() + ".png",
                      window_size.width, window_size.height);
        }
        frame++;
    }

    glfwTerminate();
    return 0;
}
