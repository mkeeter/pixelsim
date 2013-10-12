#include <iostream>

#include <GLFW/glfw3.h>
#include <OpenGL/glext.h>

#include "ship.h"
#include "shaders.h"

struct WindowSize
{
    WindowSize(int w, int h) : width(w), height(h) {}
    int width, height;
};


void window_size_callback(GLFWwindow* window, int width, int height)
{
    WindowSize* ws = static_cast<WindowSize*>(
            glfwGetWindowUserPointer(window));
    ws->width = width;
    ws->height = height;

    glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "Usage: pixelFEM imagename.png" << std::endl;
        exit(-1);
    }

    // Initialize the library
    if (!glfwInit())    return -1;

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* const window = glfwCreateWindow(
            640, 480, "pixelFEM", NULL, NULL);

    if (!window)
    {
        std::cerr << "Failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Add a pointer to a struct that will be updated in the
    // resize callback.
    WindowSize window_size(640, 480);
    glfwSetWindowUserPointer(window, static_cast<void*>(&window_size));

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Make sure that GL_EXT_framebuffer_object is supported
    if (!glfwExtensionSupported("GL_ARB_framebuffer_object"))
    {
        std::cerr << "GL_EXT_framebuffer_object not supported!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Use a callback to update glViewport when the window is resized
    glfwSetWindowSizeCallback(window, window_size_callback);

    // Turn on vsync to limit framerate
    // (no way this could backfire!)
    glfwSwapInterval(1);

    // Initialize the ship!
    Ship ship(argv[1]);
    Shaders::init();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Update the ship
        ship.Update();

        // Draw the scene
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ship.Draw(window_size.width, window_size.height);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
