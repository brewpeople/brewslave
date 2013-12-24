#pragma once

#include <MatrixMath.hpp>

class TempKalmanFilter
{
public:
	TempKalmanFilter(const float& initial_temp, const float& delta_t = 1.0);

	void get_prediction(float& temp, float& slope);
	void update(const float& temp_meas, float& temp, float& slope);
	
private:
	long unsigned int last_updated;
	float delta_t;
	float Q     [2][2];
	float R     [1][1];
	float K     [2][1];
	float H     [1][2];
	float H_T   [2][1];
	float P_k_m [2][2];
	float P_k   [2][2];
	float x_k_m [2][1];
	float x_k   [2][1];
	float F     [2][2];
	float F_T   [2][2];

	float Z     [1][1];
	float I     [2][2];

	float tmp_22_a [2][2];
	float tmp_22_b [2][2];
	float tmp_21_a   [2][1];
	float tmp_21_b   [2][1];
	float tmp_12   [1][2];
	float tmp_11   [1][1];
};
