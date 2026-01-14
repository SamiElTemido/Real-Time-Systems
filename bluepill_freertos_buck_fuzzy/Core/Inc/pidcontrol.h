#ifndef INC_PIDCONTROL_H_
#define INC_PIDCONTROL_H_

#include <stdint.h>

#define TS		0.001
#define EMAX	999
#define UMAX	999.0

typedef struct
{
	uint8_t enabled;
	int32_t err;
	int32_t prepos;
	int32_t sumError;
	int16_t offset;
	float Kp;
	float Td;
	float Ti;
	float uOut;
} PIDControl;

void pid_init(PIDControl *);
void pid_reset(PIDControl *);
float pid_compute(PIDControl *, int32_t, int32_t, float);

#endif /* INC_PIDCONTROL_H_ */
