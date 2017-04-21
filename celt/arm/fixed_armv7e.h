#ifndef FIXED_ARMv7E_H
#define FIXED_ARMv7E_H

/** 16x32 multiplication, followed by a 16-bit shift right. Results fits in 32 bits */
#undef MULT16_32_Q16 /*New_D*/
static OPUS_INLINE opus_val32 MULT16_32_Q16_armv7e(opus_val16 a, opus_val32 b){
	int rd;
	__asm__(
		"#MULT16_32_Q16\n\t"
		"smulwb %0, %1, %2\n\t"
		: "=r"(rd)
		: "r"(b),"r"(a)
	);
	return rd;		
}

#define MULT16_32_Q16(a, b) (MULT16_32_Q16_armv7e(a, b))

/** 16x32 multiplication, followed by a 15-bit shift right. Results fits in 32 bits */
#undef MULT16_32_Q15/*new_D*/
static OPUS_INLINE opus_val32 MULT16_32_Q15_armv7e(opus_val16 a, opus_val32 b){
	int rd;
	__asm__(
		"#MULT16_32_Q15\n\t"
		"smulwb %0, %1, %2\n\t"
		: "=r"(rd)
		: "r"(b),"r"(a)
	);
	/*We intentionally don't OR in the high bit of rd_lo for speed.*/
	return SHL32(rd,1);		
}
#define MULT16_32_Q15(a, b) (MULT16_32_Q15_armv7e(a, b))

/** 16x32 multiply, followed by a 15-bit shift right and 32-bit add.
    b must fit in 31 bits.
    Result fits in 32 bits. */
#undef MAC16_32_Q15
static OPUS_INLINE opus_val32 MAC16_32_Q15_armv7e(opus_val32 c, opus_val16 a,
 opus_val32 b)
{
  int res;
  __asm__(
      "#MAC16_32_Q15\n\t"
      "smlawb %0, %1, %2, %3;\n"
      : "=r"(res)
      : "r"(SHL32(b,1)), "r"(a), "r"(c)
  );
  return res;
}
#define MAC16_32_Q15(c, a, b) (MAC16_32_Q15_armv7e(c, a, b))
/** 16x32 multiply, followed by a 16-bit shift right and 32-bit add.
    Result fits in 32 bits. */
#undef MAC16_32_Q16
static OPUS_INLINE opus_val32 MAC16_32_Q16_armv7e(opus_val16 a, opus_val32 b , opus_val32 c){
	int rd;
	__asm__(
		"#MAC16_32_Q16\n\t"
		"smlawb %0, %1, %2, %3\n\t"
		: "=r"(rd)
		: "r"(b), "r"(a), "r"(c)
	);
	return rd;
}
#define MAC16_32_Q16(c, a, b) (MAC16_32_Q16_armv7e(a, b, c))

/** 16x16 multiply-add where the result fits in 32 bits */
#undef MAC16_16
static OPUS_INLINE opus_val32 MAC16_16_armv7e(opus_val32 c, opus_val16 a, opus_val16 b)
{
  int res;
  __asm__(
      "#MAC16_16\n\t"
      "smlabb %0, %1, %2, %3;\n"
      : "=r"(res)
      : "r"(a), "r"(b), "r"(c)
  );
  return res;
}
#define MAC16_16(c, a, b) (MAC16_16_armv7e(c, a, b))

/** 16x16 multiplication where the result fits in 32 bits */
#undef MULT16_16
static OPUS_INLINE opus_val32 MULT16_16_armv7e(opus_val16 a, opus_val16 b)
{
  int res;
  __asm__(
      "#MULT16_16\n\t"
      "smulbb %0, %1, %2;\n"
      : "=r"(res)
      : "r"(a), "r"(b)
  );
  return res;
}
#define MULT16_16(a, b) (MULT16_16_armv7e(a, b))


/** 32x32 multiplication, followed by a 31-bit shift right. Results fits in 32 bits */
#undef MULT32_32_Q31
static OPUS_INLINE opus_val32 MULT_32_32_Q31_armv7e(opus_val32 a, opus_val32 b){
	int rd;
	__asm__(
		"#MULT_32_32_Q31\n\t"
		"smmul %0, %1, %2\n\t"
		: "=&r"(rd)
		: "r"(b), "r"(a)
	);
	return rd;
}
#define MULT32_32_Q31(a,b) (MULT_32_32_Q31_armv7e(a,b))


#ifdef OPUS_ARM_INLINE_MEDIA
#undef SIG2WORD16
static OPUS_INLINE opus_val16 SIG2WORD16_armv6(opus_val32 x)
{
   celt_sig res;
   __asm__(
       "#SIG2WORD16\n\t"
       "ssat %0, #16, %1, ASR #12\n\t"
       : "=r"(res)
       : "r"(x+2048)
   );
   return EXTRACT16(res);
}
#define SIG2WORD16(x) (SIG2WORD16_armv6(x))
#endif /* OPUS_ARM_INLINE_MEDIA */
#endif
