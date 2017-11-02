/*
 * mathops_armv7e.c
 *
 *  Created on: Nov 2, 2017
 *      Author: Homer
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "arm_math.h"
#include "mathops_armv7e.h"


/*Compute floor(sqrt(_val)) with exact arithmetic.
  This has been tested on all possible 32-bit inputs.*/
/*Compute floor(sqrt(_val)) using intrinsicts for ARM cortex M library.*/
/*int and fixed point implementation*/
unsigned isqrt32_armv7e(opus_uint32 _val){
	arm_status res;
	q31_t in = (q31_t)_val;
	q31_t out;

res = arm_sqrt_q31(in, &out);

if(res != ARM_MATH_SUCCESS)  out = isqrt32(_val);

return (unsigned) out;

}

/* FIXME: It is necessary to complete this file*/




