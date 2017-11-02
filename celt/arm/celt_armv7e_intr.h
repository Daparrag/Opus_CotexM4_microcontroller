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

/**This function counts The number of leading zeros of a data value */
#undef EC_CLZ
static OPUS_INLINE  opus_val32 EC_CLZ_armv7e (opus_val32 _x)
{
	return __CLZ(_x);
}
#define	EC_CLZ(_x)		(EC_CLZ_armv7e(_x))







#endif /* CELT_ARMV7E_INTR_H_ */
