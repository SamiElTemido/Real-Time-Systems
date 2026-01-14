#ifndef INC_PROFILE_H_
#define INC_PROFILE_H_

#include <stdint.h>

#define TS	0.001
typedef enum pstate{STOPPED, RUNNING} ProfileState;

typedef struct
{
	ProfileState state;
	int32_t *setpoint;
	int32_t offset;
	uint16_t k, period;
	float q0, q1, q2, q3;
} Profile;

void profile_init(Profile *, int32_t *);
void profile_reset(Profile *);
void profile_start(Profile *, int32_t, int32_t);
void profile_execute(Profile *);
void profile_start(Profile *, int32_t, int32_t);

float millround(float);
float sign(float);

#endif
