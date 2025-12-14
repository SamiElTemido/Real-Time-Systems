#include "pidcontrol.h"

void pid_init(PIDControl *pid)
{
	pid->enabled = 1;
	pid->Kp = 16.56;
	pid->Ti = 10.0;
	pid->Td = 0.00757;
	pid_reset(pid);
}

void pid_reset(PIDControl *pid)
{
	pid->prepos = 0;
	pid->sumError = 0;
	pid->uOut = 0.0;
	pid->offset = 0;
}

float pid_compute(PIDControl *pid, int32_t ref, int32_t pos, float vel)
{
	if (pid->enabled == 0)
	{
		pid->prepos = 0;
		pid->sumError = 0;
		pid->uOut = pid->offset;
		return pid->uOut;
	}
	pid->err = ref - pos;
	float up = pid->Kp * pid->err;
	pid->sumError += pid->err;
	if (pid->sumError > EMAX) pid->sumError = EMAX;
	if (pid->sumError < -EMAX) pid->sumError = -EMAX;
	float ui = pid->Kp * (TS/pid->Ti) * pid->sumError;
	float ud = pid->Kp * (pid->Td/TS) * (pid->prepos - pos);
	pid->prepos = pos;
	pid->uOut = up + ui + ud;
	if (pid->uOut > UMAX) pid->uOut = UMAX;
	if (pid->uOut < -UMAX) pid->uOut = -UMAX;
	return pid->uOut;
}
