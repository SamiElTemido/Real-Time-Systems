#ifndef INC_DFILTER_H_
#define INC_DFILTER_H_

typedef struct
{
	float xin[2];
	float yout[2];
	float q[2];
} DeriveFilter;

void dFilter_init(DeriveFilter *, float, float);
void dFilter_setFreq(DeriveFilter *, float, float);
float dFilter_compute(DeriveFilter *, float);
void dFilter_reset(DeriveFilter *);

#endif /* INC_DFILTER_H_ */
