#include <MatrixMath.hpp>

float Q     [2][2];
float R     [2][2];
float K     [2][2];
float H     [2][1];
float P_k_m [2][2];
float P_k   [2][2];
float x_k_m [2][1];
float x_k   [2][1];
float F     [2][2];


unsigned long time;

void setup() {
    Serial.begin(9600); 

    // constant slope
    F[0][0] = 1;
    F[1][0] = 0;
    F[0][1] = 1;
    F[1][1] = 1;
    
    R[0][0] = 1;
    R[1][0] = 0;
    R[0][1] = 0;
    R[1][1] = 1;
    
    P_k[0][0] = 1;
    P_k[1][0] = 0;
    P_k[0][1] = 0;
    P_k[1][1] = 1;

    x_k[0][0] = 0;
    x_k[1][0] = 0;
    
    Q[0][0] = 1;
    Q[1][0] = 0;
    Q[0][1] = 0;
    Q[1][1] = 1;

    H[0][0] = 1;
    H[1][0] = 0;
    H[0][1] = 0;
    H[1][1] = 0;

}

float slope   = 1.3;
float x_meas  = 10;

void loop(){

    time = millis();

    // run process
    x_meas += slope;

    // Kalman I: predict
    Matrix.Multiply(x_k, F, 2, 2, 1, x_k_m);


    unsigned long time_new = millis();
    Serial.print("Took: ");
    Serial.print(time_new - time);
    Serial.print("\n");
    

}
