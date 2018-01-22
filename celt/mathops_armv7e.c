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

//void

/* FIXME: It is necessary to complete this file*/

#ifdef FIXED_POINT

opus_val32 frac_div32(opus_val32 a, opus_val32 b)
{
   opus_val16 rcp;
   opus_val32 result, rem;
   int shift = celt_ilog2(b)-29;
   a = VSHR32(a,shift);
   b = VSHR32(b,shift);
   /* 16-bit reciprocal */
   rcp = ROUND16(celt_rcp(ROUND16(b,16)),3);
   result = MULT16_32_Q15(rcp, a);
   rem = PSHR32(a,2)-MULT32_32_Q31(result, b);
   result = ADD32(result, SHL32(MULT16_32_Q15(rcp, rem),2));
   if (result >= 536870912)       /*  2^29 */
      return 2147483647;          /*  2^31 - 1 */
   else if (result <= -536870912) /* -2^29 */
      return -2147483647;         /* -2^31 */
   else
      return SHL32(result, 2);
}

/** Reciprocal sqrt approximation in the range [0.25,1) (Q16 in, Q14 out) */
opus_val16 celt_rsqrt_norm(opus_val32 x)
{
   opus_val16 n;
   opus_val16 r;
   opus_val16 r2;
   opus_val16 y;
   /* Range of n is [-16384,32767] ([-0.5,1) in Q15). */
   n = x-32768;
   /* Get a rough initial guess for the root.
      The optimal minimax quadratic approximation (using relative error) is
       r = 1.437799046117536+n*(-0.823394375837328+n*0.4096419668459485).
      Coefficients here, and the final result r, are Q14.*/
   r = ADD16(23557, MULT16_16_Q15(n, ADD16(-13490, MULT16_16_Q15(n, 6713))));
   /* We want y = x*r*r-1 in Q15, but x is 32-bit Q16 and r is Q14.
      We can compute the result from n and r using Q15 multiplies with some
       adjustment, carefully done to avoid overflow.
      Range of y is [-1564,1594]. */
   r2 = MULT16_16_Q15(r, r);
   y = SHL16(SUB16(ADD16(MULT16_16_Q15(r2, n), r2), 16384), 1);
   /* Apply a 2nd-order Householder iteration: r += r*y*(y*0.375-0.5).
      This yields the Q14 reciprocal square root of the Q16 x, with a maximum
       relative error of 1.04956E-4, a (relative) RMSE of 2.80979E-5, and a
       peak absolute error of 2.26591/16384. */
   return ADD16(r, MULT16_16_Q15(r, MULT16_16_Q15(y,
              SUB16(MULT16_16_Q15(y, 12288), 16384))));
}

/** Sqrt approximation (QX input, QX/2 output) */
opus_val32 celt_sqrt(opus_val32 x)
{
   int k;
   opus_val16 n;
   opus_val32 rt;
   static const opus_val16 C[5] = {23175, 11561, -3011, 1699, -664};
   if (x==0)
      return 0;
   else if (x>=1073741824)
      return 32767;
   k = (celt_ilog2(x)>>1)-7;
   x = VSHR32(x, 2*k);
   n = x-32768;
   rt = ADD16(C[0], MULT16_16_Q15(n, ADD16(C[1], MULT16_16_Q15(n, ADD16(C[2],
              MULT16_16_Q15(n, ADD16(C[3], MULT16_16_Q15(n, (C[4])))))))));
   rt = VSHR32(rt,7-k);
   return rt;
}

#define L1 32767
#define L2 -7651
#define L3 8277
#define L4 -626

static OPUS_INLINE opus_val16 _celt_cos_pi_2(opus_val16 x)
{
   opus_val16 x2;

   x2 = MULT16_16_P15(x,x);
   return ADD16(1,MIN16(32766,ADD32(SUB16(L1,x2), MULT16_16_P15(x2, ADD32(L2, MULT16_16_P15(x2, ADD32(L3, MULT16_16_P15(L4, x2
                                                                                ))))))));
}

#undef L1
#undef L2
#undef L3
#undef L4

opus_val16 celt_cos_norm(opus_val32 x)
{
   x = x&0x0001ffff;
   if (x>SHL32(EXTEND32(1), 16))
      x = SUB32(SHL32(EXTEND32(1), 17),x);
   if (x&0x00007fff)
   {
      if (x<SHL32(EXTEND32(1), 15))
      {
         return _celt_cos_pi_2(EXTRACT16(x));
      } else {
         return NEG32(_celt_cos_pi_2(EXTRACT16(65536-x)));
      }
   } else {
      if (x&0x0000ffff)
         return 0;
      else if (x&0x0001ffff)
         return -32767;
      else
         return 32767;
   }
}

/** Reciprocal approximation (Q15 input, Q16 output) */
opus_val32 celt_rcp(opus_val32 x)
{
   int i;
   opus_val16 n;
   opus_val16 r;
   celt_assert2(x>0, "celt_rcp() only defined for positive values");
   i = celt_ilog2(x);
   /* n is Q15 with range [0,1). */
   n = VSHR32(x,i-15)-32768;
   /* Start with a linear approximation:
      r = 1.8823529411764706-0.9411764705882353*n.
      The coefficients and the result are Q14 in the range [15420,30840].*/
   r = ADD16(30840, MULT16_16_Q15(-15420, n));
   /* Perform two Newton iterations:
      r -= r*((r*n)-1.Q15)
         = r*((r*n)+(r-1.Q15)). */
   r = SUB16(r, MULT16_16_Q15(r,
             ADD16(MULT16_16_Q15(r, n), ADD16(r, -32768))));
   /* We subtract an extra 1 in the second iteration to avoid overflow; it also
       neatly compensates for truncation error in the rest of the process. */
   r = SUB16(r, ADD16(1, MULT16_16_Q15(r,
             ADD16(MULT16_16_Q15(r, n), ADD16(r, -32768)))));
   /* r is now the Q15 solution to 2/(n+1), with a maximum relative error
       of 7.05346E-5, a (relative) RMSE of 2.14418E-5, and a peak absolute
       error of 1.24665/32768. */
   return VSHR32(EXTEND32(r),i-16);
}

#endif




