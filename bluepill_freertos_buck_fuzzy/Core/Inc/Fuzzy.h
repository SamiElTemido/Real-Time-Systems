#ifndef INC_FUZZY_H_
#define INC_FUZZY_H_

#define FUZZY_ERR_MAX  1500.0f  // Rango máximo de Error (+- 2900)
#define FUZZY_DERR_MAX 50.0f  // Rango máximo de dError (+- 1000)
#define FUZZY_OUT_MAX  40.0f   // Máximo cambio de DutyCycle por ciclo



float trimf(float x, float center);

float Fuzzy_Compute(float error, float d_error);


#endif /* INC_FUZZY_H_ */
