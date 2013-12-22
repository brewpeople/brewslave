#include "TempKalman.h"

TempKalmanFilter::TempKalmanFilter(const float& temp, const float& delta_t) : delta_t(delta_t), last_updated(millis())
{		
	// constant slope
    F[0][0] = 1;
    F[1][0] = 0;
    F[0][1] = delta_t;
    F[1][1] = 1;

    Matrix.Transpose((float*) F, 2, 2, (float*) F_T);
    
    R[0][0] = 1;
    
    I[0][0] = 1;
    I[1][0] = 0;
    I[0][1] = 0;
    I[1][1] = 1;
    
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
    H[0][1] = 0;
    
    Matrix.Transpose((float*) H, 1, 2, (float*) H_T);

    // initial state
    x_k[0][0] = temp;
    x_k[1][0] = 0.0;
		
}

void TempKalmanFilter::get_prediction(float& temp, float& slope)
{
	float delta_t_sec = ((long unsigned int) millis() - last_updated) / 1000.0;
	slope = x_k[1][0];
	temp = x_k[0][0] + delta_t_sec * slope;	
}
void TempKalmanFilter::update(const float& temp_meas, float& temp, float& slope)
{	
    Z[0][0] = temp_meas;
	// Kalman I: predict
    // x_k_m = F * x_k
    Matrix.Multiply((float*)F, (float*)x_k, 2, 2, 1, (float*)x_k_m);

    // P_k_m = F * P_k * F^T + Q
    Matrix.Multiply((float*)F, (float*)P_k, 2, 2, 2, (float*)tmp_22_a);
    Matrix.Multiply((float*)tmp_22_a, (float*)F_T, 2, 2, 2, (float*)tmp_22_b);
    Matrix.Add((float*) tmp_22_b, (float*) Q, 2, 2, (float*) P_k_m);


    // Kalman II: update
    
    // K = P_k_m * H^T * (H * P_k_m * H^T + R)^-1

    Matrix.Multiply((float*) H, (float*) P_k_m, 1, 2, 2, (float*) tmp_12);

 //   Matrix.Print((float*) tmp_12, 1, 2, "H * P_k_m");

    Matrix.Multiply((float*) tmp_12, (float*) H_T, 1, 2, 1, (float*) tmp_11);
 //   Matrix.Print((float*) tmp_11, 1, 1, "H * P_k_m * H_T");
    
    Matrix.Add((float*) tmp_11, (float*) R, 1, 1, (float*) tmp_11);
    Matrix.Invert((float*)tmp_11, 1);
 //   Matrix.Print((float*) tmp_11, 1, 1, "(H * P_k_m * H_T + R)^-1");

    Matrix.Multiply((float*) P_k_m, (float*) H_T, 2, 2, 1, (float*) tmp_21_a);
 //   Matrix.Print((float*) tmp_21_a, 2, 1, "P_K_m * H^T");
    
    Matrix.Multiply((float*) tmp_21_a, (float*) tmp_11, 2, 1, 1, (float*) K);
 //   Matrix.Print((float*) K, 2, 1, "K =");

    // x_k  =x_k_m + K * (Z_k - H * x_k_m)
    Matrix.Multiply((float*) H, (float*) x_k_m, 1, 2, 1, (float*) tmp_11);
    Matrix.Subtract((float*) Z, (float*) tmp_11, 1, 1, (float*) tmp_11);
    Matrix.Multiply((float*) K, (float*) tmp_11, 2, 1, 1, (float*) tmp_21_a);
    Matrix.Add((float*) x_k_m, (float*) tmp_21_a, 2, 1, (float*) x_k);
    

    // P_k = (I - K * H ) * P_k_m
    Matrix.Multiply((float*) K, (float*) H, 2, 1, 2, (float*) tmp_22_a);
    Matrix.Subtract((float*) I, (float*) tmp_22_a, 2, 2, (float*) tmp_22_b);
    Matrix.Multiply((float*) tmp_22_b, (float*) P_k_m, 2, 2, 2, (float*) P_k);
    
    unsigned long time_new = millis();

 //   Matrix.Print((float*) x_k_m, 2, 1, "x_k_m =");
 //   Matrix.Print((float*) P_k_m, 2, 2, "P_k_m =");

 //   Matrix.Print((float*) K, 2, 1, "Kalman gain =");
 //   Matrix.Print((float*) Z, 1, 1, "Z =");

  //  Matrix.Print((float*) x_k, 2, 1, "x_k =");
  //  Matrix.Print((float*) P_k, 2, 2, "P_k =");
  
  last_updated = millis();
  
  temp = x_k[0][0];
  slope = x_k[1][0];
	
}
