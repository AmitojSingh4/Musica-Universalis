#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <format>
#include <fstream>
#include <vector>
#include <numbers>

// function prototypes
struct point {
    GLfloat x;
    GLfloat y;
};

std::vector<float> createString( const int numberOfPoints, const float length, const float height ); // plucked string

std::vector<float> createString( const int numberOfPoints, const float length, const float height, const float width, const float startingLocation, const std::string sign = "positive" ); // pulse string

std::vector<float> createString( const int numberOfPoints, const int mode, const float height ); // standing wave string

void initialiseGLFW();

void initialiseGLAD();

void initialiseShaders( unsigned int &vertexShader, unsigned int &fragmentShader, unsigned int &shaderProgram );

void initialiseVboVao(unsigned int &VBO, unsigned int &VAO, point *graph, unsigned int &shaderProgram);

void input( GLFWwindow *window );

void rendering( unsigned int &shaderProgram, unsigned int &VAO, const int numberOfPoints );

void eventSwap( GLFWwindow *window );

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
    // initialises GLFW
    initialiseGLFW();

    // create the window and checks if its opened
    GLFWwindow *window = glfwCreateWindow( 800, 600, "WavesOnStrings", NULL, NULL );
    if( window == NULL ) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback ); // resizes the viewport if the size of the window is changed

    // initialises GLAD and set some parameters
    initialiseGLAD();

    // file and data saving
    std::string   fileName = "WavesOnStringsData.dat"; // name of file to save data to
    std::ofstream data( "../../data/" + fileName );
    if( !data ) {
        std::cerr << format( "Error: could not open file, {}\n\n", fileName );
        abort();
    }

    // system variables
    const float length         = 100; // length of the string in the x direction (meters)
    const int   numberOfPoints = 101; // number of points in the string, can only be odd
    const float height         = 1.0; // amplitude of peaks in the y direction (meters)

    point graph[numberOfPoints];

    // initial shape of string
    // std::vector<float> stringVector = createString( numberOfPoints, length, height ); // 1
    std::vector<float> stringVector = createString( numberOfPoints, 3, height ); // 3
    // std::vector<float> stringVector = createString( numberOfPoints, length, height, 5, 50 ); // 2

    // initialises shaders
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;
    initialiseShaders( vertexShader, fragmentShader, shaderProgram );

    // initialises VBO and VAO
    unsigned int VBO;
    unsigned int VAO;
    initialiseVboVao(VBO, VAO, graph, shaderProgram);
    
    float       deltaTime = 0.1;  // delta time between steps (secconds)
    // string variables
    const int          stringPoints = stringVector.size();
    const float        tension      = 10.0;       // tension along the string (newtons)
    std::vector<float> mass( stringPoints, 1.0 ); // mass of the string (kg) - mass is uniform accross the string
    // temporary vectors
    std::vector<float> velocity( stringPoints, 0.0 );
    std::vector<float> temporaryString( stringPoints, 0.0 );
    temporaryString.at( 0 )                = stringVector.at( 0 );
    temporaryString.at( stringPoints - 1 ) = stringVector.at( stringPoints - 1 );
    const float deltaLength = length / ( numberOfPoints - 1 );

    while( !glfwWindowShouldClose( window ) ) {
        _sleep(100);
        input(window);

        for( int i = 1; i < stringPoints - 1; i++ ) {
            velocity.at( i ) += ( ( tension / mass.at( i ) ) * ( ( stringVector.at( i - 1 ) - 2 * stringVector.at( i ) + stringVector.at( i + 1 ) ) / pow( deltaLength, 2 ) ) ) * deltaTime;
            temporaryString.at( i ) = stringVector.at( i ) + velocity.at( i ) * deltaTime;
            if( i > 2 ) {
                stringVector.at( i - 2 ) = temporaryString.at( i - 2 );
            }
        }
        stringVector.at( stringPoints - 2 ) = temporaryString.at( stringPoints - 2 ); // last 2 points that dont get changed
        stringVector.at( stringPoints - 3 ) = temporaryString.at( stringPoints - 3 );

        for( int i = 0; i < numberOfPoints; i++ ){
            float x = (i)/50.0;
            graph[i].x = x-1;
            graph[i].y = stringVector[i];
        }

        // save data to the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(graph), graph, GL_STATIC_DRAW);
        rendering( shaderProgram, VAO, numberOfPoints );

        eventSwap( window );
    }

    glfwTerminate();
    return 0;
}

// functions
std::vector<float> createString( const int numberOfPoints, const float length, const float height ) {
    std::vector<float> stringVector( numberOfPoints, 0.0 );
    const float        gradiant = height / ( length / 2.0 ); // gradient per point on the string
    for( int i = 0; i <= ( ( numberOfPoints - 1 ) / 2 ); i++ ) {
        stringVector.at( i )                      = i * gradiant; // fills left to midpoint
        stringVector.at( numberOfPoints - 1 - i ) = i * gradiant; // fills right to midpoint
    }
    return stringVector;
}

std::vector<float> createString( const int numberOfPoints, const float length, const float height, const float width, const float startingLocation, const std::string sign ) {
    std::vector<float> stringVector( numberOfPoints, 0.0 );
    float              signValue = 1.0;
    if( sign != "positive" ) {
        signValue = -1.0;
    }
    const int widthPoints           = int( ( width / length ) * ( numberOfPoints - 1.0 ) + 0.5 );          // the width in terms of points on the string
    const int startingLocationPoint = int( ( startingLocation / length ) * ( numberOfPoints - 1 ) + 0.5 ); // starting location in the vector
    for( int i = startingLocationPoint; i <= startingLocationPoint + widthPoints; i++ ) {
        stringVector.at( i ) = height * signValue * sin( ( i - startingLocationPoint ) * 2.0 * std::numbers::pi_v<float> / widthPoints );
    }
    return stringVector;
}

std::vector<float> createString( const int numberOfPoints, const int mode, const float height ) {
    std::vector<float> stringVector( numberOfPoints, 0.0 );
    for( int i = 0; i < stringVector.size(); i++ ) {
        float heightValue    = height * sin( i * ( mode / 2.0 ) * 2.0 * std::numbers::pi / ( numberOfPoints - 1 ) );
        stringVector.at( i ) = heightValue;
    }
    return stringVector;
}

void initialiseGLFW(){
    // initialises glfw
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

#ifdef __APPLE__
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
#endif
}

void initialiseGLAD(){
    // initialises glad and check if its initialised correctly
    if( !gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) ) {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    // lets opengl know the size of the window it can write to
    glViewport( 0, 0, 800, 600 );

    // when screen is cleared uses this colour
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
}

void initialiseShaders( unsigned int &vertexShader, unsigned int &fragmentShader, unsigned int &shaderProgram ){
    // vertex shader
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
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infolog << std::endl;
    }

    // shader program
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
}

void initialiseVboVao(unsigned int &VBO, unsigned int &VAO, point *graph, unsigned int &shaderProgram){
    // vbo
    glGenBuffers(1, &VBO);
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
}

void input( GLFWwindow *window ){
    processInput( window );
}

void rendering( unsigned int &shaderProgram, unsigned int &VAO, const int numberOfPoints ){
        glClear( GL_COLOR_BUFFER_BIT );
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_LOOP, 0, numberOfPoints);
}

void eventSwap( GLFWwindow *window ){
    glfwSwapBuffers( window );
    glfwPollEvents();
}

void framebuffer_size_callback( GLFWwindow *window, int width, int height ) {
    glViewport( 0, 0, width, height );
}

void processInput( GLFWwindow *window ) {
    if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ) {
        glfwSetWindowShouldClose( window, true );
    }
}
