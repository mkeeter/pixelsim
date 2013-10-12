#include <iostream>

#include "extensions.h"

static void (*genFBO)(GLsizei, GLuint);
static void (*bindFBO)(GLenum, GLuint);
static void (*FBOtex2D)(GLenum, GLuint);

bool EXT::Init()
{
    bool success = true;

    // Make sure that GL_EXT_framebuffer_object is supported
    success &= glfwExtensionSupported("GL_ARB_framebuffer_object");

    // Load pointers to all of these functions
    genFBO = (decltype(genFBO))(
            glfwGetProcAddress("glGenFramebuffersARB"));
    success &= (void*)genFBO != NULL;

    bindFBO = (decltype(bindFBO))(
            glfwGetProcAddress("glBindFramebufferARB"));
    success &= (void*)bindFBO != NULL;

    FBOtex2D = (decltype(FBOtex2D))(
            glfwGetProcAddress("glFramebufferTexture2DARB"));
    success &= (void*)FBOtex2D != NULL;
    glBindFramebuffer(0, 0);
    return success;
}
