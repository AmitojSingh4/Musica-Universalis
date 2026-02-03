// #include
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <format>
#include <fstream>
#include <vector>
#include <numbers>

// function prototypes
std::vector<float> createString( const int numberOfPoints, const float length, const float height ); // plucked string

std::vector<float> createString( const int numberOfPoints, const float length, const float height, const float width, const float startingLocation, const std::string sign = "positive" ); // pulse string

std::vector<float> createString( const int numberOfPoints, const int mode, const float height ); // standing wave string

void updateString( std::vector<float> &stringVector, std::ofstream &file, const float deltaLength );

// main
int main() {
    // file and data saving
    std::string   fileName = "WavesOnStringsData.dat"; // name of file to save data to
    std::ofstream data( "../../data/"+fileName );
    if( !data ) {
        std::cerr << format( "Error: could not open file, {}\n\n", fileName );
        abort();
    }
    // creating string vector
    const float length         = 100; // length of the string in the x direction (meters)
    const int    numberOfPoints = 101; // number of points in the string, can only be odd
    const float height         = 0.1;
    // testing
    // vector<float> stringVector = createString( numberOfPoints, length, height ); // 1
    // vector<float> stringVector = createString( numberOfPoints, 3, height ); // 3
    std::vector<float> stringVector = createString( numberOfPoints, length, height, 5, 50 ); // 2
    updateString( stringVector, data, length / ( numberOfPoints - 1 ) );

    data.close();

    return EXIT_SUCCESS;
}

// functions
std::vector<float> createString( const int numberOfPoints, const float length, const float height ) {
    std::vector<float> stringVector( numberOfPoints, 0.0 );
    const float   gradiant = height / ( length / 2.0 ); // gradient per point on the string
    for( int i = 0; i <= ( ( numberOfPoints - 1 ) / 2 ); i++ ) {
        stringVector.at( i )                      = i * gradiant; // fills left to midpoint
        stringVector.at( numberOfPoints - 1 - i ) = i * gradiant; // fills right to midpoint
    }
    return stringVector;
}

std::vector<float> createString( const int numberOfPoints, const float length, const float height, const float width, const float startingLocation, const std::string sign ) {
    std::vector<float> stringVector( numberOfPoints, 0.0 );
    float         signValue = 1.0;
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

std::vector<float> createString( const int numberOfPoints, const int mode, const float height ) {
    std::vector<float> stringVector( numberOfPoints, 0.0 );
    for( int i = 0; i < stringVector.size(); i++ ) {
        float heightValue   = height * sin( i * ( mode / 2.0 ) * 2.0 * std::numbers::pi / ( numberOfPoints - 1 ) );
        stringVector.at( i ) = heightValue;
    }
    return stringVector;
}

void updateString( std::vector<float> &stringVector, std::ofstream &file, const float deltaLength ) {
    // time
    float       deltaTime = 0.1;  // delta time between steps (secconds)
    float       time      = 0.0;  // time (secconds)
    const float timeLimit = 50.0; // maximum time value (secconds)
    // string variables
    const int      stringPoints = stringVector.size();
    const float   tension      = 10.0;       // tension along the string (newtons)
    std::vector<float> mass( stringPoints, 1.0 ); // mass of the string (kg) - mass is uniform accross the string
    // temporary vectors
    std::vector<float> velocity( stringPoints, 0.0 );
    std::vector<float> temporaryString( stringPoints, 0.0 );
    temporaryString.at( 0 )                = stringVector.at( 0 );
    temporaryString.at( stringPoints - 1 ) = stringVector.at( stringPoints - 1 );

    file << "t\tx\ty\n";
    while( time <= timeLimit ) {
        file << std::format( "{}\t{}\t{}\n", time, 0, stringVector.at( 0 ) );
        for( int i = 1; i < stringPoints - 1; i++ ) {
            file << std::format( "{}\t{}\t{}\n", time, i * deltaLength, stringVector.at( i ) );
            velocity.at( i ) += ( ( tension / mass.at( i ) ) * ( ( stringVector.at( i - 1 ) - 2 * stringVector.at( i ) + stringVector.at( i + 1 ) ) / pow( deltaLength, 2 ) ) ) * deltaTime;
            temporaryString.at( i ) = stringVector.at( i ) + velocity.at( i ) * deltaTime;
            if( i > 2 ) {
                stringVector.at( i - 2 ) = temporaryString.at( i - 2 );
            }
        }
        file << std::format( "{}\t{}\t{}\n", time, ( stringPoints - 1 ) * deltaLength, stringVector.at( stringPoints - 1 ) );
        stringVector.at( stringPoints - 2 ) = temporaryString.at( stringPoints - 2 ); // last 2 points that dont get changed
        stringVector.at( stringPoints - 3 ) = temporaryString.at( stringPoints - 3 );
        time += deltaTime;
    }
}
