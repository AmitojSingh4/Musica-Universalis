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
    const double length         = 100;  // length of the string in the x direction (meters)
    const int    numberOfPoints = 1001; // number of points in the string, can only be odd
    const double height         = 0.1;
    // testing
    // vector<double> stringVector = createString( numberOfPoints, length, height ); // 1
    // vector<double> stringVector = createString( numberOfPoints, 2, height ); // 3
    vector<double> stringVector = createString( numberOfPoints, length, height, 5, 50 ); // 2

    for( int i = 0; i < stringVector.size(); i++ ) {
        data << format( "{}\t{}\n", i, stringVector.at( i ) );
    }

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
    const int widthPoints = int( ( width / length ) * ( numberOfPoints - 1.0 ) + 0.5 ); // the width in terms of points on the string
    const int startingLocationPoint = int( (startingLocation / length ) * (numberOfPoints - 1) + 0.5); // starting location in the vector
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
