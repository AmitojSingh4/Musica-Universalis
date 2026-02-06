#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <format>
#include <fstream>
#include <vector>
#include <numbers>
#include <queue>

// function prototypes
struct point
{
    GLfloat x;
    GLfloat y;
};

struct bufferData
{
    std::vector<float> string;
    float              time;
};

std::vector<float> createString( const int numberOfPoints, const float length, const float height ); // plucked string

std::vector<float> createString( const int numberOfPoints, const float length, const float height, const float width, const float startingLocation, const std::string sign = "positive" ); // pulse string

std::vector<float> createString( const int numberOfPoints, const int mode, const float height ); // standing wave string

void pushToBuffer( std::queue<bufferData> &buffer, const std::vector<float> &stringVector, const float time );

void writeToFile( std::queue<bufferData> &buffer, std::ofstream &data, const float deltaLength );

void initialiseGLFW();

void initialiseGLAD();

void initialiseShaders( unsigned int &vertexShader, unsigned int &fragmentShader, unsigned int &shaderProgram );

void initialiseVboVao( unsigned int &VBO, unsigned int &VAO, point *graph, unsigned int &shaderProgram );

void processInput( GLFWwindow *window, unsigned long &sleepTime, bool &saveData );

void rendering( unsigned int &shaderProgram, unsigned int &VAO, const int numberOfPoints );

void eventSwap( GLFWwindow *window );

void framebuffer_size_callback( GLFWwindow *window, int width, int height );

// clang-format off
const char *vertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "   FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\0";
// clang-format on

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
    bool          saveData = false;
    std::string   fileName = "WavesOnStringsData.dat"; // name of file to save data to
    std::ofstream data( "../../data/" + fileName );
    if( !data ) {
        std::cerr << format( "Error: could not open file, {}\n\n", fileName );
        abort();
    }
    data << "t\tx\ty\n";

    // system variables
    const float length         = 100; // length of the string in the x direction (meters)
    const int   numberOfPoints = 101; // number of points in the string, can only be odd
    const float height         = 1.0; // amplitude of peaks in the y direction (meters)

    point graph[numberOfPoints];

    std::queue<bufferData> buffer;

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
    initialiseVboVao( VBO, VAO, graph, shaderProgram );

    // time variables
    float         time      = 0.0; // time (secconds)
    float         deltaTime = 0.1; // delta time between steps (secconds)
    unsigned long sleepTime = 100; // time between rendered frames, does not effect the moddel only rendering (millisecconds)
    int intTime = 0; // 
    // string variables
    const int          stringPoints = stringVector.size();
    const float        tension      = 10.0;                           // tension along the string (newtons)
    std::vector<float> mass( stringPoints, 1.0 );                     // mass of the string (kg) - mass is uniform accross the string
    const float        deltaLength = length / ( numberOfPoints - 1 ); // the distance between points (meters)
    // temporary vectors
    std::vector<float> velocity( stringPoints, 0.0 );
    std::vector<float> temporaryString( stringPoints, 0.0 );
    temporaryString.at( 0 )                = stringVector.at( 0 );                // failsafe lines, however are unused
    temporaryString.at( stringPoints - 1 ) = stringVector.at( stringPoints - 1 ); // also a failsafe line

    while( !glfwWindowShouldClose( window ) ) {
        _sleep( sleepTime );
        processInput( window, sleepTime, saveData );

        // update the string for non edge points
        for( int i = 1; i < stringPoints - 1; i++ ) {
            velocity.at( i ) += ( ( tension / mass.at( i ) ) * ( ( stringVector.at( i - 1 ) - 2 * stringVector.at( i ) + stringVector.at( i + 1 ) ) / pow( deltaLength, 2 ) ) ) * deltaTime;
            temporaryString.at( i ) = stringVector.at( i ) + velocity.at( i ) * deltaTime;
            if( i > 2 ) {
                stringVector.at( i - 2 ) = temporaryString.at( i - 2 );
            }
        }
        stringVector.at( stringPoints - 2 ) = temporaryString.at( stringPoints - 2 ); // last 2 points that dont get changed
        stringVector.at( stringPoints - 3 ) = temporaryString.at( stringPoints - 3 );

        // copy the data
        for( int i = 0; i < numberOfPoints; i++ ) {
            float x    = ( i ) / 50.0;
            graph[i].x = x - 1;
            graph[i].y = stringVector[i];
        }

        if( saveData ) {
            writeToFile( buffer, data, deltaLength );
            saveData = false;
        }
        else if( !saveData && time + 1e-4 >= intTime ) {
            // buffer data
            pushToBuffer( buffer, stringVector, time );
            std::cout << time << std::endl;
            intTime += 1;
        }

        time += deltaTime;

        // save data to the buffer
        glBufferData( GL_ARRAY_BUFFER, sizeof( graph ), graph, GL_STATIC_DRAW );
        rendering( shaderProgram, VAO, numberOfPoints );

        eventSwap( window );
    }

    glfwTerminate();
    data.close();
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

void pushToBuffer( std::queue<bufferData> &buffer, const std::vector<float> &stringVector, const float time ) {
    bufferData data;
    data.string = stringVector;
    data.time   = time;
    buffer.push( data );
    if( buffer.size() > 20 ) {
        buffer.pop();
    }
}

void writeToFile( std::queue<bufferData> &buffer, std::ofstream &data, const float deltaLength ) {
    for( int i = 0; i < buffer.size(); i++ ) {
        std::vector<float> stringVector = buffer.front().string;
        float              time         = buffer.front().time;
        for( int j = 0; j < stringVector.size(); j++ ) {
            data << std::format( "{:.1f}\t{}\t{}\n", time, j * deltaLength, stringVector.at( j ) );
        }
        buffer.pop();
    }
    std::cout << "Data Saved!" << std::endl;
}

void initialiseGLFW() {
    // initialises glfw
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

#ifdef __APPLE__
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
#endif
}

void initialiseGLAD() {
    // initialises glad and check if its initialised correctly
    if( !gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) ) {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    // lets opengl know the size of the window it can write to
    glViewport( 0, 0, 800, 600 );

    // when screen is cleared uses this colour
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
}

void initialiseShaders( unsigned int &vertexShader, unsigned int &fragmentShader, unsigned int &shaderProgram ) {
    // vertex shader
    vertexShader = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertexShader, 1, &vertexShaderSource, NULL );
    glCompileShader( vertexShader );

    int  success;
    char infolog[512];
    glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &success );
    if( !success ) {
        glGetShaderInfoLog( vertexShader, 512, NULL, infolog );
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << std::endl;
    }

    // fragment shader
    fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragmentShader, 1, &fragmentShaderSource, NULL );
    glCompileShader( fragmentShader );

    glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &success );
    if( !success ) {
        glGetShaderInfoLog( fragmentShader, 512, NULL, infolog );
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infolog << std::endl;
    }

    // shader program
    shaderProgram = glCreateProgram();
    glAttachShader( shaderProgram, vertexShader );
    glAttachShader( shaderProgram, fragmentShader );
    glLinkProgram( shaderProgram );

    glGetProgramiv( shaderProgram, GL_LINK_STATUS, &success );
    if( !success ) {
        glGetProgramInfoLog( shaderProgram, 512, NULL, infolog );
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << std::endl;
    }

    glUseProgram( shaderProgram );
    glDeleteShader( vertexShader );
    glDeleteShader( fragmentShader );
}

void initialiseVboVao( unsigned int &VBO, unsigned int &VAO, point *graph, unsigned int &shaderProgram ) {
    // vbo
    glGenBuffers( 1, &VBO );
    glGenVertexArrays( 1, &VAO );
    glBindVertexArray( VAO );
    // copy verticies array into a buffer
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( graph ), graph, GL_STATIC_DRAW );
    // set vertex attributes pointers
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0 );
    glEnableVertexAttribArray( 0 );
    // use shader program
    glUseProgram( shaderProgram );
    glBindVertexArray( VAO );
}

void processInput( GLFWwindow *window, unsigned long &sleepTime, bool &saveData ) {
    if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ) {
        glfwSetWindowShouldClose( window, true );
    }
    if( glfwGetKey( window, GLFW_KEY_1 ) == GLFW_PRESS ) {
        sleepTime = 100;
    }
    if( glfwGetKey( window, GLFW_KEY_2 ) == GLFW_PRESS ) {
        sleepTime = 75;
    }
    if( glfwGetKey( window, GLFW_KEY_3 ) == GLFW_PRESS ) {
        sleepTime = 50;
    }
    if( glfwGetKey( window, GLFW_KEY_4 ) == GLFW_PRESS ) {
        sleepTime = 25;
    }
    if( glfwGetKey( window, GLFW_KEY_5 ) == GLFW_PRESS ) {
        sleepTime = 1;
    }
    if( glfwGetKey( window, GLFW_KEY_0 ) == GLFW_PRESS ) {
        saveData = true;
    }
}

void rendering( unsigned int &shaderProgram, unsigned int &VAO, const int numberOfPoints ) {
    glClear( GL_COLOR_BUFFER_BIT );
    glUseProgram( shaderProgram );
    glBindVertexArray( VAO );
    glDrawArrays( GL_LINE_LOOP, 0, numberOfPoints );
}

void eventSwap( GLFWwindow *window ) {
    glfwSwapBuffers( window );
    glfwPollEvents();
}

void framebuffer_size_callback( GLFWwindow *window, int width, int height ) {
    glViewport( 0, 0, width, height );
}
