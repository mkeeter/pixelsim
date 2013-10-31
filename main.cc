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

////////////////////////////////////////////////////////////////////////////////

void window_size_callback(GLFWwindow* window, int width, int height)
{
    WindowSize* ws = static_cast<WindowSize*>(
            glfwGetWindowUserPointer(window));
    ws->width = width;
    ws->height = height;

    glViewport(0, 0, width, height);
}

////////////////////////////////////////////////////////////////////////////////

bool check_extensions()
{
    // Make sure that all of the extensions we need are supported
    std::string extensions[] = {
        "GL_ARB_framebuffer_object",
        "GL_ARB_texture_rg", // Used for GL_R8 internal texture format
        "GL_ARB_texture_float"};
    for (auto s : extensions) {
        if (!glfwExtensionSupported(s.c_str()))
        {
            std::cerr << s << " not supported!" << std::endl;
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "Usage: pixelFEM imagename.png" << std::endl;
        exit(-1);
    }

    // Initialize the library
    if (!glfwInit())    return -1;
    glfwWindowHint(GLFW_SAMPLES, 0);    // multisampling!

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* const window = glfwCreateWindow(
            640, 480, "pixelFEM", NULL, NULL);

    if (!window)
    {
        std::cerr << "Failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Check that all of the required extensions exist.
    if (!check_extensions()) {
        glfwTerminate();
        return -1;
    }

    // Use a callback to update glViewport when the window is resized, by
    // saving a pointer to a WindowSize struct in the window's user pointer
    // field.
    WindowSize window_size(640, 480);
    glfwSetWindowUserPointer(window, static_cast<void*>(&window_size));
    glfwSetWindowSizeCallback(window, window_size_callback);

    // Initialize the ship!
    Ship ship(argv[1]);
    Shaders::init();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Store the start time of this update loop
        const auto t0 = std::chrono::high_resolution_clock::now();

        // Update the ship
        ship.Update(0.1f);

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
