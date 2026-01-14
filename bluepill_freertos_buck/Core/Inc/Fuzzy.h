#ifndef INC_FUZZY_H_
#define INC_FUZZY_H_

#define FUZZY_ERR_MAX  2900.0f  // Rango máximo de Error (+- 2900)
#define FUZZY_DERR_MAX 1000.0f  // Rango máximo de dError (+- 1000)
#define FUZZY_OUT_MAX  500.0f   // Máximo cambio de DutyCycle por ciclo

const float OUTPUT_WEIGHTS[5] = {-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};

float trimf(float x, float center);

float Fuzzy_Compute(float error, float d_error);


#endif /* INC_FUZZY_H_ */
