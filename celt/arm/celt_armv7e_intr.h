/*
 * celt_armv7e_intr.h
 *
 *  Created on: Nov 2, 2017
 *      Author: Homer
 */

#ifndef CELT_ARMV7E_INTR_H_
#define CELT_ARMV7E_INTR_H_


#include "arm_math.h"

/** saturate a 32-bits signed value  */
#undef SATURATE
static OPUS_INLINE  opus_val32 SATURATE_armv7e (opus_val32 value, opus_val8 sat)
{
	return __SSAT(value,sat);
}

#define SATURATE (x,a)	(SATURATE_armv7e(x,a))

#undef SATURATE

/** sum two 16-bits signed values in parallel  */
#undef ADD2_16
static OPUS_INLINE  opus_val32 ADD2_16_armv7e (opus_val32 val_a, opus_val32 val_b)
{
	return __SADD16(val_a,val_b);
}

#define ADD2_16(a,b)	(ADD2_16_armv7e(a,b))

/** Subtract two 16-bits signed values in parallel  */
#undef SUB2_16(a,b)
static OPUS_INLINE  opus_val32 SUB2_16_armv7e (opus_val32 val_a, opus_val32 val_b)
{
	return __SSUB16(a,b);
}
#define SUB2_16(a,b)	( SUB2_16_armv7e(a,b))



/**
  * @brief  This function enables to exchange two halfwords of one operand and perform one 16-bit
  * 		integer subtraction
  * @param  opus_val32 val1: first operand for the addition in the low halfword,
  * 						 and the first operand for the subtraction in the high halfword.
  * @param  opus_val32 val2: second operand for the addition in the high halfword,
  * 						and the second operand for the subtraction	in the low halfword.
  * @retval opus_val32 res : res[15:0]= val1[15:0] + val2[31:16]
  * 						 res[15:0]= val1[31:16] - val2[15:0]
  */
#undef ADD_SUB_H
static OPUS_INLINE  opus_val32 ADD_SUB_H_armv7e (opus_val32 val1, opus_val32 val2)
{
	return __SSAX(a,b);
}
#define ADD_SUB_H(a,b)	(ADD_SUB_H_armv7e(a,b))

/**
  * @brief  This function enables to exchange two halfwords of the second operand, add the high halfwords
  * 		and subtract the low halfword
  * @param  opus_val32 val1: first operand for the subtraction in the high halfword,
  * 						 and the first operand for the addition in the lower halfword.
  * @param  opus_val32 val2: second operand for the subtraction in the high halfword,
  * 						and the second operand for the addition	in the low halfword.
  * @retval opus_val32 res : res[15:0]= val1[15:0] - val2[31:16]
  * 						 res[15:0]= val1[31:16] + val2[15:0]
  */
#undef SUB_ADD_H
static OPUS_INLINE  opus_val32 SUB_ADD_H_armv7e (opus_val32 val1, opus_val32 val2)
{
	return __UASX(a,b);
}

#define SUB_ADD_H(a,b)	(SUB_ADD_H_armv7e(a,b))





/**This function counts The number of leading zeros of a data value */
#undef EC_CLZ
static OPUS_INLINE  opus_val32 EC_CLZ_armv7e (opus_val32 _x)
{
	return __CLZ(_x);
}
#define	EC_CLZ(_x)		(EC_CLZ_armv7e(_x))









#endif /* CELT_ARMV7E_INTR_H_ */
