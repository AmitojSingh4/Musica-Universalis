// includes
// --------

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

// structs
// -------

struct point
{
    GLfloat x;
    GLfloat y;
};

struct bufferData
{
    std::vector<double> string;
    double              time;
};

struct callBackData
{
    point       *axisTicks;
    unsigned int axisTicksVBO;
    int          numberOfTicksOnAxis;
    int          numberOfTicks;
};

// function prototypes
// -------------------

std::vector<double> createString( const int numberOfPoints, const double length, const double height ); // plucked string

std::vector<double> createString( const int numberOfPoints, const double length, const double height, const double width, const double startingLocation, const std::string sign = "positive" ); // pulse string

std::vector<double> createString( const int numberOfPoints, const int mode, const double height ); // standing wave string

void updateFixedString( std::vector<double> &stringVector, std::vector<double> &velocity, const std::vector<double> &mass, const int stringPoints, const double tension, const double deltaLength, const double deltaTime );

void updateFreeString( std::vector<double> &stringVector, std::vector<double> &velocity, const std::vector<double> &mass, const int stringPoints, const double tension, const double deltaLength, const double deltaTime );

void updateFreeDispersiveString( std::vector<double> &stringVector, std::vector<double> &velocity, const std::vector<double> &mass, const int stringPoints, const double tension, const double deltaLength, const double deltaTime, const double dampingCoefficient );

void makeAxisTicks( point *axisTicks, int numberOfTicks, float tickSize, GLFWwindow *window );

void pushToBuffer( std::queue<bufferData> &buffer, const std::vector<double> &stringVector, const double time );

void writeToFile( std::queue<bufferData> buffer, std::ofstream &data, const double deltaLength );

// opengl function prototypes
// --------------------------

void initialiseGLFW();

void initialiseGLAD();

void initialiseShaders( unsigned int &vertexShader, unsigned int &fragmentShader, unsigned int &shaderProgram, int &colour );

void initialiseVboVao( unsigned int &VBO, unsigned int &VAO, point *graph, unsigned int &shaderProgram );

void initialiseAxesVboVao( unsigned int &axesVBO, unsigned int &axesVAO, point *axes, unsigned int &shaderProgram );

void initialiseAxisTicksVboVao( unsigned int &axisTicksVBO, unsigned int &axisTicksVAO, point *axisTicks, unsigned int &shaderProgram, const int numberOfTicks );

void processInput( GLFWwindow *window, float &updateSpeed, bool &saveData );

void rendering( unsigned int &shaderProgram, unsigned int &VAO, unsigned int &axesVAO, unsigned int &axisTicksVAO, const int numberOfPoints, int colourLocation, const int numberOfticks );

void eventSwap( GLFWwindow *window );

void framebuffer_size_callback( GLFWwindow *window, int width, int height );

// shader sources
// --------------

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
    "uniform vec3 colour;\n"
    "void main() {\n"
    "   FragColor = vec4( colour, 1.0f);\n"
    "}\0";
// clang-format on

// main
// ----

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

    // callback data for axis ticks
    callBackData callbackData;

    // initialises GLAD and set some parameters
    initialiseGLAD();

    // file and data saving
    bool          saveData     = false;
    double         autoSaveTime = 0.0;                     // 0.0 = no auto save (secconds)
    std::string   fileName     = "WavesOnStringsData.dat"; // name of file to save data to
    std::ofstream data( "../../data/" + fileName );
    if( !data ) {
        std::cerr << format( "Error: could not open file, {}\n\n", fileName );
        abort();
    }
    data << "t\tx\ty\n";

    // system variables
    const double length         = 100; // length of the string in the x direction (meters)
    const int   numberOfPoints = 101; // number of points in the string, can only be odd
    const double height         = 1.0; // amplitude of peaks in the y direction (meters)

    // holds the string data
    point graph[numberOfPoints];

    // holds the axes
    // clang-format off
    point axes[4] = {
        // x axis 
        {-1.0f, 0.0f},
        { 1.0f, 0.0f},
        // y axis
        {0.0f, -1.0f},
        {0.0f,  1.0f}
    };
    // clang-format on

    // axis ticks
    const int numberOfTicksOnAxis = 10; // number of ticks per half a full axis
    const int numberOfTicks       = ( 2 * numberOfTicksOnAxis + 1 ) * 4;
    point     axisTicks[numberOfTicks];
    makeAxisTicks( axisTicks, numberOfTicksOnAxis, 0.01f, window );

    // holds the buffered data
    std::queue<bufferData> buffer;

    // initial shape of string
    // std::vector<double> stringVector = createString( numberOfPoints, length, height ); // 1
    std::vector<double> stringVector = createString( numberOfPoints, 3, height ); // 3
    // std::vector<double> stringVector = createString( numberOfPoints, length, height, 5, 50 ); // 2

    // initialises shaders
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;
    int          colourLocation;
    initialiseShaders( vertexShader, fragmentShader, shaderProgram, colourLocation );

    // initialises axis VBO and VAO
    unsigned int axesVBO;
    unsigned int axesVAO;
    initialiseAxesVboVao( axesVBO, axesVAO, axes, shaderProgram );

    // initialises axis ticks VBO and VAO
    unsigned int axisTicksVBO;
    unsigned int axisTicksVAO;
    initialiseAxisTicksVboVao( axisTicksVBO, axisTicksVAO, axisTicks, shaderProgram, numberOfTicks );

    // axis callback data
    callbackData.axisTicks           = axisTicks;
    callbackData.axisTicksVBO        = axisTicksVBO;
    callbackData.numberOfTicksOnAxis = numberOfTicksOnAxis;
    callbackData.numberOfTicks       = numberOfTicks;
    glfwSetWindowUserPointer( window, &callbackData );

    // initialises VBO and VAO
    unsigned int VBO;
    unsigned int VAO;
    initialiseVboVao( VBO, VAO, graph, shaderProgram );

    // time variables
    double time        = 0.0; // time (secconds)
    double deltaTime   = 0.1; // delta time between steps (secconds)
    int   intTime     = 0;   // integer time used for buffering data
    double realTime    = 0.0; // the in world real time that has passed
    float updateSpeed = 1.0; // the speed at which the string is updated
    // string variables
    const int          stringPoints = stringVector.size();
    const double        tension      = 10.0;                                  // tension along the string (newtons)
    std::vector<double> mass( stringPoints, 1.0 );                            // mass of the string (kg) - mass is uniform accross the string
    const double        deltaLength        = length / ( numberOfPoints - 1 ); // the distance between points (meters)
    const double        dampingCoefficient = 1.0;                             // damping coefficient in the free dispersive string
    // velocity vector
    std::vector<double> velocity( stringPoints, 0.0 );

    // enables vsync
    glfwSwapInterval( 1 );

    while( !glfwWindowShouldClose( window ) ) {
        // frame time calculation
        double        currentTime  = glfwGetTime();
        static double previousTime = currentTime;
        double        frameTime    = currentTime - previousTime;
        previousTime              = currentTime;

        processInput( window, updateSpeed, saveData );

        // auto save
        if( autoSaveTime != 0.0 && time + 1e-4 > autoSaveTime ) {
            saveData     = true;
            autoSaveTime = 0.0;
        }

        // saving and buffering data
        if( saveData ) {
            saveData = false;
            writeToFile( buffer, data, deltaLength );
        }
        else if( !saveData && time + 1e-4 >= intTime ) { // push to buffer every int seccond
            // buffer data
            pushToBuffer( buffer, stringVector, time );
            std::cout << std::format("Time: {:.1f}", time) << std::endl;
            intTime += 1;
        }

        // updates string
        if( realTime + 1e-4 >= time ) {
            // updateFixedString( stringVector, velocity, mass, stringPoints, tension, deltaLength, deltaTime );
            // updateFreeString( stringVector, velocity, mass, stringPoints, tension, deltaLength, deltaTime );
            updateFreeDispersiveString( stringVector, velocity, mass, stringPoints, tension, deltaLength, deltaTime, dampingCoefficient );
            // copy the data
            for( int i = 0; i < numberOfPoints; i++ ) {
                float x    = ( i ) / 50.0;
                graph[i].x = x - 1;
                graph[i].y = static_cast<float>(stringVector[i]);
            }
            time += deltaTime;
        }

        // precise time
        realTime += frameTime * static_cast<double>(updateSpeed);
        //std::cout << realTime << "\t" << time << std::endl;

        // save data to the buffer
        glBindBuffer( GL_ARRAY_BUFFER, VBO );
        glBufferData( GL_ARRAY_BUFFER, numberOfPoints * sizeof( point ), graph, GL_DYNAMIC_DRAW );
        rendering( shaderProgram, VAO, axesVAO, axisTicksVAO, numberOfPoints, colourLocation, numberOfTicks );

        eventSwap( window );
    }

    glfwTerminate();
    data.close();
    return 0;
}

// functions
// ---------

std::vector<double> createString( const int numberOfPoints, const double length, const double height ) {
    // plucked
    std::vector<double> stringVector( numberOfPoints, 0.0 );
    const double        gradiant = height / ( length / 2.0 ); // gradient per point on the string
    for( int i = 0; i <= ( ( numberOfPoints - 1 ) / 2 ); i++ ) {
        stringVector.at( i )                      = i * gradiant; // fills left to midpoint
        stringVector.at( numberOfPoints - 1 - i ) = i * gradiant; // fills right to midpoint
    }
    return stringVector;
}

std::vector<double> createString( const int numberOfPoints, const double length, const double height, const double width, const double startingLocation, const std::string sign ) {
    // pulse 
    std::vector<double> stringVector( numberOfPoints, 0.0 );
    double              signValue = 1.0;
    if( sign != "positive" ) {
        signValue = -1.0;
    }
    const int widthPoints           = int( ( width / length ) * ( numberOfPoints - 1.0 ) + 0.5 );          // the width in terms of points on the string
    const int startingLocationPoint = int( ( startingLocation / length ) * ( numberOfPoints - 1 ) + 0.5 ); // starting location in the vector
    for( int i = startingLocationPoint; i <= startingLocationPoint + widthPoints; i++ ) {
        stringVector.at( i ) = height * signValue * sin( ( i - startingLocationPoint ) * 2.0 * std::numbers::pi / widthPoints );
    }
    return stringVector;
}

std::vector<double> createString( const int numberOfPoints, const int mode, const double height ) {
    // standing waves
    std::vector<double> stringVector( numberOfPoints, 0.0 );
    for( int i = 0; i < stringVector.size(); i++ ) {
        double heightValue    = height * sin( i * ( mode / 2.0 ) * 2.0 * std::numbers::pi / ( numberOfPoints - 1 ) );
        stringVector.at( i ) = heightValue;
    }
    return stringVector;
}

void updateFixedString( std::vector<double> &stringVector, std::vector<double> &velocity, const std::vector<double> &mass, const int stringPoints, const double tension, const double deltaLength, const double deltaTime ) {
    // temporary vectors
    std::vector<double> temporaryString( stringPoints, 0.0 );
    temporaryString.at( 0 )                = stringVector.at( 0 );                // failsafe lines, however are unused
    temporaryString.at( stringPoints - 1 ) = stringVector.at( stringPoints - 1 ); // also a failsafe line
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
}

void updateFreeString( std::vector<double> &stringVector, std::vector<double> &velocity, const std::vector<double> &mass, const int stringPoints, const double tension, const double deltaLength, const double deltaTime ) {
    // temporary vectors
    std::vector<double> temporaryString( stringPoints, 0.0 );
    // first and last point
    velocity.at( 0 ) += ( ( tension / mass.at( 0 ) ) * ( ( stringVector.at( 1 ) - stringVector.at( 0 ) ) / pow( deltaLength, 2 ) ) ) * deltaTime;
    velocity.at( stringPoints - 1 ) += ( -( tension / mass.at( stringPoints - 1 ) ) * ( ( stringVector.at( stringPoints - 1 ) - stringVector.at( stringPoints - 2 ) ) / pow( deltaLength, 2 ) ) ) * deltaTime;
    temporaryString.at( 0 )                = stringVector.at( 0 ) + velocity.at( 0 ) * deltaTime;
    temporaryString.at( stringPoints - 1 ) = stringVector.at( stringPoints - 1 ) + velocity.at( stringPoints - 1 ) * deltaTime;
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
    stringVector.at( 0 )                = temporaryString.at( 0 );                // first point
    stringVector.at( stringPoints - 1 ) = temporaryString.at( stringPoints - 1 ); // last point
}

void updateFreeDispersiveString( std::vector<double> &stringVector, std::vector<double> &velocity, const std::vector<double> &mass, const int stringPoints, const double tension, const double deltaLength, const double deltaTime, const double dampingCoefficient ) {
    // temporary vectors
    std::vector<double> temporaryString( stringPoints, 0.0 );
    // first and last point
    velocity.at( 0 ) += ( ( tension / mass.at( 0 ) ) * ( ( stringVector.at( 1 ) - stringVector.at( 0 ) ) / pow( deltaLength, 2 ) ) - ( dampingCoefficient / mass.at( 0 ) ) * ( velocity.at( 0 ) ) ) * deltaTime;
    velocity.at( stringPoints - 1 ) +=
      ( -( tension / mass.at( stringPoints - 1 ) ) * ( ( stringVector.at( stringPoints - 1 ) - stringVector.at( stringPoints - 2 ) ) / pow( deltaLength, 2 ) ) - ( dampingCoefficient / mass.at( stringPoints - 1 ) ) * ( velocity.at( stringPoints - 1 ) ) ) * deltaTime;
    temporaryString.at( 0 )                = stringVector.at( 0 ) + velocity.at( 0 ) * deltaTime;
    temporaryString.at( stringPoints - 1 ) = stringVector.at( stringPoints - 1 ) + velocity.at( stringPoints - 1 ) * deltaTime;
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
    stringVector.at( 0 )                = temporaryString.at( 0 );                // first point
    stringVector.at( stringPoints - 1 ) = temporaryString.at( stringPoints - 1 ); // last point
}

void makeAxisTicks( point *axisTicks, int numberOfTicks, float tickSize, GLFWwindow *window ) {
    // window width and height
    int width;
    int height;
    glfwGetFramebufferSize( window, &width, &height );
    // aspect ratio for equivilent sized ticks
    float aspectRatio = static_cast<float>( width ) / static_cast<float>( height );
    int   i           = 0;
    for( int j = -numberOfTicks; j <= numberOfTicks; ++j ) {
        float tick = static_cast<float>( j ) / numberOfTicks;

        // x axis
        axisTicks[i++] = { tick, tickSize * aspectRatio };
        axisTicks[i++] = { tick, -tickSize * aspectRatio };
        // y axis
        axisTicks[i++] = { -tickSize, tick };
        axisTicks[i++] = { tickSize, tick };
    }
}

void pushToBuffer( std::queue<bufferData> &buffer, const std::vector<double> &stringVector, const double time ) {
    // buffers the past 10 values passed into it
    bufferData data;
    data.string = stringVector;
    data.time   = time;
    buffer.push( data );
    if( buffer.size() > 10 ) {
        buffer.pop();
    }
}

void writeToFile( std::queue<bufferData> buffer, std::ofstream &data, const double deltaLength ) {
    // saves the buffered data to the file
    const int initialBufferSize = buffer.size();
    for( int i = 0; i < initialBufferSize; i++ ) {
        std::vector<double> stringVector = buffer.front().string;
        double              time         = buffer.front().time;
        for( int j = 0; j < stringVector.size(); j++ ) {
            data << std::format( "{:.1f}\t{}\t{}\n", time, j * deltaLength, stringVector.at( j ) );
        }
        buffer.pop();
    }
    std::cout << "Data Saved!" << std::endl;
}

// opengl functions
// ----------------

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

void initialiseShaders( unsigned int &vertexShader, unsigned int &fragmentShader, unsigned int &shaderProgram, int &colourLocation ) {
    // error logging
    int  success;
    char infolog[512];
    // vertex shader
    vertexShader = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertexShader, 1, &vertexShaderSource, NULL );
    glCompileShader( vertexShader );

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
    colourLocation = glGetUniformLocation( shaderProgram, "colour" );
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
}

void initialiseAxesVboVao( unsigned int &axesVBO, unsigned int &axesVAO, point *axes, unsigned int &shaderProgram ) {
    // vbo
    glGenBuffers( 1, &axesVBO );
    glGenVertexArrays( 1, &axesVAO );
    glBindVertexArray( axesVAO );
    // copy verticies array into a buffer
    glBindBuffer( GL_ARRAY_BUFFER, axesVBO );
    glBufferData( GL_ARRAY_BUFFER, 4 * sizeof( point ), axes, GL_STATIC_DRAW );
    // set vertex attributes pointers
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0 );
    glEnableVertexAttribArray( 0 );
}

void initialiseAxisTicksVboVao( unsigned int &axisTicksVBO, unsigned int &axisTicksVAO, point *axisTicks, unsigned int &shaderProgram, const int numberOfTicks ) {
    // vbo
    glGenBuffers( 1, &axisTicksVBO );
    glGenVertexArrays( 1, &axisTicksVAO );
    glBindVertexArray( axisTicksVAO );
    // copy verticies array into a buffer
    glBindBuffer( GL_ARRAY_BUFFER, axisTicksVBO );
    glBufferData( GL_ARRAY_BUFFER, numberOfTicks * sizeof( point ), axisTicks, GL_STATIC_DRAW );
    // set vertex attributes pointers
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0 );
    glEnableVertexAttribArray( 0 );
}

void processInput( GLFWwindow *window, float &updateSpeed, bool &saveData ) {
    // saving data variables
    static bool saveKeyWasPressed = false;
    bool saveKeyIsPressed = glfwGetKey( window, GLFW_KEY_0 ) == GLFW_PRESS;
    // key presses
    if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS ) {
        glfwSetWindowShouldClose( window, true );
    }
    if( glfwGetKey( window, GLFW_KEY_1 ) == GLFW_PRESS ) {
        updateSpeed = 1.0;
    }
    if( glfwGetKey( window, GLFW_KEY_2 ) == GLFW_PRESS ) {
        updateSpeed = 2.0;
    }
    if( glfwGetKey( window, GLFW_KEY_3 ) == GLFW_PRESS ) {
        updateSpeed = 5.0;
    }
    if( glfwGetKey( window, GLFW_KEY_4 ) == GLFW_PRESS ) {
        updateSpeed = 0.5;
    }
    if( glfwGetKey( window, GLFW_KEY_5 ) == GLFW_PRESS ) {
        updateSpeed = 0.1;
    }
    if( saveKeyIsPressed && !saveKeyWasPressed ) {
        saveData = true;
    }
    saveKeyWasPressed = saveKeyIsPressed;
}

void rendering( unsigned int &shaderProgram, unsigned int &VAO, unsigned int &axesVAO, unsigned int &axisTicksVAO, const int numberOfPoints, int colourLocation, const int numberOfticks ) {
    glClear( GL_COLOR_BUFFER_BIT );
    glUseProgram( shaderProgram );
    // axes
    glUniform3f( colourLocation, 0.75f, 0.75f, 0.75f );
    glLineWidth( 1.0f );
    glBindVertexArray( axesVAO );
    glDrawArrays( GL_LINES, 0, 4 );
    glBindVertexArray( axisTicksVAO );
    glDrawArrays( GL_LINES, 0, numberOfticks );
    // string
    glUniform3f( colourLocation, 0.0f, 0.0f, 0.0f );
    glLineWidth( 1.3f );
    glBindVertexArray( VAO );
    glDrawArrays( GL_LINE_STRIP, 0, numberOfPoints );
}

void eventSwap( GLFWwindow *window ) {
    glfwSwapBuffers( window );
    glfwPollEvents();
}

void framebuffer_size_callback( GLFWwindow *window, int width, int height ) {
    glViewport( 0, 0, width, height );
    // resize axis ticks
    callBackData *data = static_cast<callBackData *>( glfwGetWindowUserPointer( window ) );
    makeAxisTicks( data->axisTicks, data->numberOfTicksOnAxis, 0.01f, window );
    glBindBuffer( GL_ARRAY_BUFFER, data->axisTicksVBO );
    glBufferData( GL_ARRAY_BUFFER, data->numberOfTicks * sizeof( point ), data->axisTicks, GL_STATIC_DRAW );
}
