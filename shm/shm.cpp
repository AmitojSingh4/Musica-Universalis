// #include
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <format>
#include <vector>
#include <random>
#include <fstream>

using namespace std;

// function prototypes
void eulerIntegration( const double initialVelocity, const double initialPosition, ofstream &file );

// main
int main() {
    ofstream data( "shm-data.dat" );
    eulerIntegration( 0, 1, data );
    data.close();
    return EXIT_SUCCESS;
}

// functions
void eulerIntegration( const double initialVelocity, const double initialPosition, ofstream &file ) {
    // initial velocity (meters/seccond)
    // initial position (meters)
    double deltaTime               = 0.001; // delta time between steps (secconds)
    double time                    = 0.0;   // time (secconds)
    double velocity                = initialVelocity;
    double position                = initialPosition;
    double mass                    = 1.0; // mass (kilograms)
    double k                       = 1.0; // spring constant (newton/meters)
    double dampingConstant         = 1.0; // damping constant
    double drivingForce            = 1.0; // driving force
    double drivingAngularFrequency = 1.0; // driving angular frequency

    while( time < 20 ) {
        file << time << "\t" << position << "\n";
        velocity += ( -k * position - dampingConstant * velocity + drivingForce * cos( drivingAngularFrequency * time ) ) * deltaTime / mass;
        position += velocity * deltaTime;
        time += deltaTime;
    }
    return;
}
