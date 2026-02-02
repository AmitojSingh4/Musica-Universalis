#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <numbers>

// function prototypes
void framebuffer_size_callback( GLFWwindow *window, int width, int height );

void processInput( GLFWwindow *window );

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "   FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\0";

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

    // when screen is cleared uses this colour
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    struct point {
        GLfloat x;
        GLfloat y;
    };

    point graph[1000];

    for( int i = 0; i < 1000; i++ ){
        float x = (i)/100.0;
        graph[i].x = x-1;
        graph[i].y = cos(x*3.141592653589793);
    }

    // vertex shader
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infolog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << std::endl;
    }

    // fragment shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infolog << std::endl;
    }

    // shader program
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << std::endl;
    }

    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // vbo
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    // copy verticies array into a buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(graph), graph, GL_STATIC_DRAW);
    // set vertex attributes pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    // use shader program
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);


    while( !glfwWindowShouldClose( window ) ) {
        //
        processInput( window );

        glClear( GL_COLOR_BUFFER_BIT );
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_STRIP, 0, 1000);

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

void processInput( GLFWwindow *window ) {
    if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ) {
        glfwSetWindowShouldClose( window, true );
    }
}
