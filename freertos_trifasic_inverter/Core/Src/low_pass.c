/*
 * low_pass.c
 *
 *  Created on: Oct 29, 2025
 *      Author: Samuel
 */
#include <low_pass.h>

float a[FORDER+1]={
		1.000000000000000,-4.438467815579958,8.854915480723745,-9.975142272550256
		,6.658387733940515,-2.487622308316359,0.407475776275259
};
float b[FORDER+1]={
		0.040417532280911,-0.127344926885810,0.231744360220145,-0.270532271549056
		,0.231744360220145,-0.127344926885810,0.040417532280912
};
float filter_compute(float xin)
{
	static float y[FORDER+1]={0};
	static float x[FORDER+1]={0};
	for(int k=FORDER;k>0;k--)
	{
		y[k]=y[k-1];
		x[k]=x[k-1];
	}
	x[0]=xin;
	/*compute filter output*/
	y[0]=b[0]+x[0];

	for (int k = 0; k <= FORDER; k++)
		{
			y[0] += b[k] * x[k];
		}

		for (int k = 1; k <= FORDER; k++)
		{
			y[0] -= a[k] * y[k];
		}

		return y[0];
}
