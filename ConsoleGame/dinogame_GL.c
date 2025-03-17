#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <OpenGl/gl3.h>
#include <stdio.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 200

int main(){
    if(!glfwInit()){
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    

    // configure glfw
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // set opengl major version to 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // set opengl minor version to 3 
                                                   // togetheri they specify opengl 3.3

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // use core profile which is the modern opengl
    // core profile removes depracated features and requires using modern opengl practices

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // makes our code compatible with future opengl versions
    // especially important for macOS compatibility


    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, // with of the window 
        WINDOW_HEIGHT, // height of the window
        "Dino Game", // title of the window
        NULL, // monitor to use (NULL = windowed mode)
        NULL // Window to share recources with (NULL = no sharing)
    );
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    // main game loop 
    while (!glfwWindowShouldClose(window))
    {
        /* code */
        // clear the screen to teal color
        glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup    
    glfwTerminate();

    return 0;
}