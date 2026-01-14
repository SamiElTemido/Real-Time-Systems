#include "dfilter.h"

void dFilter_init(DeriveFilter *df, float tsamp, float freq)
{
	dFilter_reset(df);
	dFilter_setFreq(df, tsamp, freq);
}

void dFilter_setFreq(DeriveFilter *df, float Ts, float Wc)
{
	float tao;
	if (Wc == 0.0) return;
	tao = 1.0 / Wc;
	df->q[0] = 2.0 / (Ts + 2 * tao);
	df->q[1] = (Ts - 2 * tao) / (Ts + 2 * tao);
}

float dFilter_compute(DeriveFilter *df, float xnew)
{
	df->xin[1] = df->xin[0];
	df->yout[1] = df->yout[0];
	df->xin[0] = xnew;
	df->yout[0] = df->q[0] * (df->xin[0] - df->xin[1]) - df->q[1] * df->yout[1];
	return df->yout[0];
}

void dFilter_reset(DeriveFilter *df)
{
	df->xin[0] = 0.0;
	df->xin[1] = 0.0;
	df->yout[0] = 0.0;
	df->yout[1] = 0.0;
}

