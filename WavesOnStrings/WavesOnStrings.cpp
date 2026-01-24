// #include
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <format>
#include <fstream>
#include <vector>
#include <numbers>

using namespace std;

// function prototypes
vector<double> createString( const int numberOfPoints, const double length, const double height ); // plucked string

vector<double> createString( const int numberOfPoints, const double length, const double height, const double width, const double startingLocation, const string sign = "positive" ); // pulse string

vector<double> createString( const int numberOfPoints, const int mode, const double height ); // standing wave string

void updateString( vector<double> &stringVector, ofstream &file, const double deltaLength );

// main
int main() {
    // file and data saving
    string   fileName = "WavesOnStringsData.dat"; // name of file to save data to
    ofstream data( fileName );
    if( !data ) {
        cerr << format( "Error: could not open file, {}\n\n", fileName );
        abort();
    }
    // creating string vector
    const double length         = 100; // length of the string in the x direction (meters)
    const int    numberOfPoints = 101; // number of points in the string, can only be odd
    const double height         = 0.1;
    // testing
    // vector<double> stringVector = createString( numberOfPoints, length, height ); // 1
    vector<double> stringVector = createString( numberOfPoints, 3, height ); // 3
    // vector<double> stringVector = createString( numberOfPoints, length, height, 5, 50 ); // 2
    updateString( stringVector, data, length / ( numberOfPoints - 1 ) );

    data.close();

    return EXIT_SUCCESS;
}

// functions
vector<double> createString( const int numberOfPoints, const double length, const double height ) {
    vector<double> stringVector( numberOfPoints, 0.0 );
    const double   gradiant = height / ( length / 2.0 ); // gradient per point on the string
    for( int i = 0; i <= ( ( numberOfPoints - 1 ) / 2 ); i++ ) {
        stringVector.at( i )                      = i * gradiant; // fills left to midpoint
        stringVector.at( numberOfPoints - 1 - i ) = i * gradiant; // fills right to midpoint
    }
    return stringVector;
}

vector<double> createString( const int numberOfPoints, const double length, const double height, const double width, const double startingLocation, const string sign ) {
    vector<double> stringVector( numberOfPoints, 0.0 );
    double         signValue = 1.0;
    if( sign != "positive" ) {
        signValue = -1.0;
    }
    const int widthPoints           = int( ( width / length ) * ( numberOfPoints - 1.0 ) + 0.5 );          // the width in terms of points on the string
    const int startingLocationPoint = int( ( startingLocation / length ) * ( numberOfPoints - 1 ) + 0.5 ); // starting location in the vector
    for( int i = startingLocationPoint; i <= startingLocationPoint + widthPoints; i++ ) {
        stringVector.at( i ) = height * signValue * sin( ( i - startingLocationPoint ) * 2.0 * numbers::pi / widthPoints );
    }
    return stringVector;
}

vector<double> createString( const int numberOfPoints, const int mode, const double height ) {
    vector<double> stringVector( numberOfPoints, 0.0 );
    for( int i = 0; i < stringVector.size(); i++ ) {
        double heightValue   = height * sin( i * ( mode / 2.0 ) * 2.0 * numbers::pi / ( numberOfPoints - 1 ) );
        stringVector.at( i ) = heightValue;
    }
    return stringVector;
}

void updateString( vector<double> &stringVector, ofstream &file, const double deltaLength ) {
    // time
    double       deltaTime = 0.1;  // delta time between steps (secconds)
    double       time      = 0.0;  // time (secconds)
    const double timeLimit = 20.0; // maximum time value (secconds)
    // string variables
    const int      stringPoints = stringVector.size();
    const double   tension      = 10.0;       // tension along the string (newtons)
    vector<double> mass( stringPoints, 1.0 ); // mass of the string (kg) - mass is uniform accross the string
    // temporary vectors
    vector<double> velocity( stringPoints, 0.0 );
    vector<double> temporaryString( stringPoints, 0.0 );
    temporaryString.at( 0 )                = stringVector.at( 0 );
    temporaryString.at( stringPoints - 1 ) = stringVector.at( stringPoints - 1 );

    file << "t\tx\ty\n";
    while( time <= timeLimit ) {
        file << format( "{}\t{}\t{}\n", time, 0, stringVector.at( 0 ) );
        for( int i = 1; i < stringPoints - 1; i++ ) {
            file << format( "{}\t{}\t{}\n", time, i * deltaLength, stringVector.at( i ) );
            velocity.at( i ) += ( ( tension / mass.at( i ) ) * ( ( stringVector.at( i - 1 ) - 2 * stringVector.at( i ) + stringVector.at( i + 1 ) ) / pow( deltaLength, 2 ) ) ) * deltaTime;
            temporaryString.at( i ) = stringVector.at( i ) + velocity.at( i ) * deltaTime;
            if( i > 2 ) {
                stringVector.at( i - 2 ) = temporaryString.at( i - 2 );
            }
        }
        file << format( "{}\t{}\t{}\n", time, ( stringPoints - 1 ) * deltaLength, stringVector.at( stringPoints - 1 ) );
        stringVector.at( stringPoints - 2 ) = temporaryString.at( stringPoints - 2 ); // last 2 points that dont get changed
        stringVector.at( stringPoints - 3 ) = temporaryString.at( stringPoints - 3 );
        time += deltaTime;
    }
}
