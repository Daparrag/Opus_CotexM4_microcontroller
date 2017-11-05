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


void scale16_vect_armv7e(opus_val16 * in, float scale,opus_val16 * out ,opus_val32 in_length){

	opus_val16 tmp1, tmp2, tmp3, tmp4;
	uint32_t blkCnt = in_length >> 2u;
	/*loop unrolling*/

	while(blkCnt > 0u){
		/*C= A * scale*/
		/*Scale The input and then store the results in the destination buffer.*/
		/*read input samples from source.*/
		tmp1=*in;
		tmp2=*(in+1);
		/*Multiply for and scalar factor.*/
		tmp1 = tmp1 * scale;
		/* read input sample from source */
		tmp3 = *(in+2);
		/*Multiply for and scalar factor.*/
		tmp2 = tmp2 * scale;
		/* read input sample from source */
		tmp4 = *(in+3);
		/* multiply with scaling factor */
		tmp3 = tmp3 * scale;
		tmp4 = tmp4 * scale;
		/* store result in the destination */
		*out = tmp1;
		*(out+1) = tmp2;
		*(out+2) = tmp3;
		*(out+3) = tmp4;

		/* update pointers to process next samples */
		in+=4;
		out+=4;
		blkCnt--;
	}

	/*if the block is not multiple of 4 compute the remaining samples here*/
	/** No loop unrolling is used. */
	 blkCnt = in_length % 0x4u;

	 while(blkCnt > 0u){
		 /* C = A * scale */
		 /* Scale the input and then store the result in the destination buffer. */
		 *out++ = (*in++) * scale;
		 /* Decrement the loop counter */
		 blkCnt--;

	 }
}

/* FIXME: It is necessary to complete this file*/




