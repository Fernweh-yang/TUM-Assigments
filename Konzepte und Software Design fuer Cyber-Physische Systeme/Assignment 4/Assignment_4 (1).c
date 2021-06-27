/*
Konzepte und Software Design für Cyber-Physische Systeme

Assignment 4: Basics of control theory and PID controller design
Group: Lim, Seokkyun; Mijacevic, Matej; Xu, Yang

*/
#include <stdio.h>
#include "drone_simulation.h"

static char DEFAULT_FLIGHT_LOG[] = "flight.csv";

float PIDControl(float height, float tar_height){
	
	//PID-Controller gains
	float K_p = 5.0, K_d = 1.4, K_i = 0.05;
	
	// Values that have to keep a state in between of invocations
	static float e_integral = 0.0f;
	static float e_prev = 0.0f;
	
	// Error Calculations including integral and derivative error
	float e = tar_height - height;
	e_integral += e*TIME_STEP;
	float e_derivative = (e - e_prev)/TIME_STEP;
	
	// Calculation of Controller Output
	float output = K_p * e + K_i * e_integral + K_d * e_derivative;
	
	// Setting the new "previous error"
	e_prev = e;
	
	// Limitation of Controller Output
	if(output < MIN_ACCELERATION){
		return MIN_ACCELERATION;
	} else if(output > MAX_ACCELERATION){
		return MAX_ACCELERATION;
	}
	
	return output;
}
	
	

int main(void){
	
	// Simulation parameters
	float sim_time = 10.0f;  // Simulation time
	float tar_height = 1.0f; // Target height of the drone
	float time, height;
    char *flight_log = DEFAULT_FLIGHT_LOG;
	
	// Starting the Simulation
	sim_start(flight_log);

	// Simulation
    do {
        time = sim_advance_time();
        height = sim_get_height();
        sim_set_acceleration(PIDControl(height, tar_height));
    } while (time < sim_time);

	// Stopping the Simulation
    sim_end();
	
    return 0;

}