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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    WindowSize window_size(640, 480);
    GLFWwindow* const window = glfwCreateWindow(
            window_size.width, window_size.height, "pixelFEM", NULL, NULL);

    if (!window)
    {
        std::cerr << "Failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Use a callback to update glViewport when the window is resized, by
    // saving a pointer to a WindowSize struct in the window's user pointer
    // field.
    glfwSetWindowUserPointer(window, static_cast<void*>(&window_size));
    glfwSetFramebufferSizeCallback(window, window_size_callback);

    // Get the actual framebuffer size (so that it works properly on
    // retina displays, rather than only filling 1/4 of the window)
    glfwGetFramebufferSize(window, &window_size.width, &window_size.height);

    // Initialize the ship!
    Ship ship(argv[1]);
    Shaders::init();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Store the start time of this update loop
        const auto t0 = std::chrono::high_resolution_clock::now();

        // Update the ship
        ship.Update(1.0e0/60, 10);

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
