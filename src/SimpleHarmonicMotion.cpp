// #include
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <format>
#include <fstream>

// function prototypes
void eulerIntegration( std::ofstream &file );

// main
int main() {
    std::string   fileName = "SimpleHarmonicMotionData.dat"; // name of file to save data to
    std::ofstream data( fileName );
    eulerIntegration( data );
    data.close();
    return EXIT_SUCCESS;
}

// functions
void eulerIntegration( std::ofstream &file ) {
    // initial conditions
    const double initialVelocity = 0.0; // initial velocity (meters/seccond)
    const double initialPosition = 1.0; // initial position (meters)
    // time
    double deltaTime = 0.001; // delta time between steps (secconds)
    double time      = 0.0;   // time (secconds)
    double timeLimit = 20.0;  // maximum time value (secconds)
    // position and velocity values
    double velocity = initialVelocity;
    double position = initialPosition;
    // oscillator variables
    double mass = 1.0; // mass (kilograms)
    double k    = 1.0; // spring constant (newtons/meter)
    // damping variables
    double dampingConstant = 1.0; // damping constant (kilograms/seccond)
    // driving variables
    double drivingForce            = 1.0; // driving force (newtons)
    double drivingAngularFrequency = 1.0; // driving angular frequency (radians/seccond)

    while( time <= timeLimit ) {
        file << std::format("{}\t{}\n", time, position );
        velocity += ( -k * position - dampingConstant * velocity + drivingForce * cos( drivingAngularFrequency * time ) ) * deltaTime / mass;
        position += velocity * deltaTime;
        time += deltaTime;
    }
    return;
}
