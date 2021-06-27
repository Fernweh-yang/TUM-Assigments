#include "drone_simulation.h"

float PIDcontrol(float time,float *error_sum,float *error_old){
    float K_p = 2.0,K_d = 0.2,K_i = 0.5;
    float target_height = _sim_get_example_trajectory(time);
    sim_set_user_data(target_height);
    float current_height = sim_get_height();
    float error = current_height - target_height;
    float u;
    *error_sum += error;
    u = K_p*error + *error_sum * K_i + K_d*(error-*error_old);
    return u;
}
void main(){
    const char *filename = "flight_data.txt";
    float sim_time = 29.0f;
    float time =0.0f;
    float error_sum = 0.0;
    float error_old = 0.0;
    float acceleration = 0;
	
    sim_start(filename);
    while(time < sim_time){
        time = sim_advance_time();
        acceleration = PIDcontrol(time,&error_sum,&error_old);
		sim_set_acceleration(acceleration);
    }
    sim_end();
}