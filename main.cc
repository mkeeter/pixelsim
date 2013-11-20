#include <iostream>
#include <chrono>
#include <thread>

#include <GLFW/glfw3.h>
#include <OpenGL/glext.h>

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
    State(WindowSize* ws) :
        window_size(ws), up_pressed(false) {}

    WindowSize* window_size;
    bool up_pressed;
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
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

////////////////////////////////////////////////////////////////////////////////

void GetArgs(int argc, char** argv,
             std::string* filename, WindowSize* window_size, bool* record)
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
        else if (!strcmp(argv[a], "--record"))
        {
            *record = true;
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
    State state(&window_size);

    bool record = false;
    std::string filename;
    GetArgs(argc, argv, &filename, &window_size, &record);

    // Initialize the library
    if (!glfwInit())    return -1;

    glfwWindowHint(GLFW_SAMPLES, 0);    // multisampling!
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

    // Use a callback to update glViewport when the window is resized, by
    // saving a pointer to a WindowSize struct in the window's user pointer
    // field.
    glfwSetWindowUserPointer(window, static_cast<void*>(&state));
    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwSetKeyCallback(window, key_callback);

    // Get the actual framebuffer size (so that it works properly on
    // retina displays, rather than only filling 1/4 of the window)
    glfwGetFramebufferSize(window, &window_size.width, &window_size.height);

    // Initialize the ship!
    Ship ship(filename);
    Shaders::init();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Store the start time of this update loop
        const auto t0 = std::chrono::high_resolution_clock::now();

        // Update the ship
        ship.Update(1.0e0/60, 25);

        // Draw the scene
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ship.Draw(window_size.width, window_size.height);

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
    }

    glfwTerminate();
    return 0;
}
