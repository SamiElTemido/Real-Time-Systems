#include "profile.h"
#include <math.h>

void profile_init(Profile *profile, int32_t *reference)
{
	profile_reset(profile);
	profile->setpoint = reference;
}

void profile_reset(Profile *profile)
{
	profile->state = STOPPED;
}

void profile_start(Profile *ptr, int32_t pos, int32_t vel)
{
	float T, a;
	T = millround(fabs(3.0 * pos) / fabs(2.0 * vel));
	a = sign(pos) * fabs(4.0 * vel) / T;
	ptr->period = (int)(T * 1000);

	ptr->q3 = -a/(3.0 * T);
	ptr->q2 = a / 2.0;
	ptr->q1 = 0.0;
	ptr->q0 = 0.0;

	ptr->k = 0;
	ptr->state = RUNNING;
}

void profile_execute(Profile *profile)
{
	float t, pos;
	if (profile->state == STOPPED)
		return;
	t = profile->k * TS;
	pos = profile->q3 *t*t*t + profile->q2 *t*t;
	*profile->setpoint = (int)(pos + profile->offset);
	profile->k = profile->k + 1;
	if (profile->k > profile->period)
		profile->state = STOPPED;
}

float millround(float value)
{
	return ((int)(value * 1000.0)) / 1000.0;
}

float sign(float value)
{
	if (value >= 0)
		return 1.0;
	else
		return -1.0;
}
