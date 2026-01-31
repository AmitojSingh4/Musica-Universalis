#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

// function prototypes
void framebuffer_size_callback( GLFWwindow *window, int width, int height );

void processInput(GLFWwindow* window);

// main
int main() {
    // initialises glfw
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

#ifdef __APPLE__
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
#endif

    // create the window and checks if its opened
    GLFWwindow *window = glfwCreateWindow( 800, 600, "WavesOnStrings", NULL, NULL );
    if( window == NULL ) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );

    // resizes the viewport if the size of the window is changed
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

    // initialises glad and check if its initialised correctly
    if( !gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) ) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // lets opengl know the size of the window it can write to
    glViewport( 0, 0, 800, 600 );

    glClearColor(0.2f,0.3f,0.3f,1.0f);

    while( !glfwWindowShouldClose( window ) ) {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// functions
void framebuffer_size_callback( GLFWwindow *window, int width, int height ) {
    glViewport( 0, 0, width, height );
}

void processInput(GLFWwindow* window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}