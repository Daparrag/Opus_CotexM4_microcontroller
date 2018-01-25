/* Copyright (c) 2017 
   Written by  */
/**
   @file armv7e_mdct.c
   @brief ARM CortexM4 optimizations for mdct using ARM library
 */

/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SKIP_CONFIG_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#endif

#include "kiss_fft.h"
#include "_kiss_fft_guts.h"
#include "mdct.h"
#include "stack_alloc.h"
#if defined(USE_LIB)
#include <arm_math.h>
#endif

#if DEBUG_MDCT
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#define PRINTDEBUG(...)
#endif

typedef enum mdct_state {MDCT_SUCESS, MDCT_INVALID_LENGTH, MDCT_INITIALIZATION_ERROR} mdct_status;
#if defined(USE_LIB)
#if !defined(FIXED_POINT)
#define  ARMV7E_MDCT_INSTANCE_TYPE_T arm_dct4_instance_f32 /*for fixed & float poit MDCT*/
#define ARMV7E_RFFT_INSTANCE_TYPE_T arm_rfft_instance_f32 /*for fixed & float poit RFFT*/
#define ARMV7E_CFFT_R4_INSTANCE_TYPE_T arm_cfft_radix4_instance_f32 /*for fixed& float poit Radix4_CFFT*/
#define ARMV7E_MDCT_SIZE_TYPE_T uint16_t /*for fixed & float point MDCT length of the DCT-IV*/
#define ARMV7E_MDCT_NORMALIZE_FACT_TYPE_T float32_t /*for fixed & float point MDCT Normalization FACTOR*/                   
#define ARMV7E_MDCT_STATE_TYPE_T  float32_t  
#define ARMV7E_MDCT_INPUT_TYPE_T  float32_t                             
#define ARM_STATUS arm_status        
#else
#define  ARMV7E_MDCT_INSTANCE_TYPE_T arm_dct4_instance_f32 /*for fixed & float poit MDCT*/
#define ARMV7E_RFFT_INSTANCE_TYPE_T arm_rfft_instance_f32 /*for fixed & float poit RFFT*/
#define ARMV7E_CFFT_R4_INSTANCE_TYPE_T arm_cfft_radix4_instance_f32 /*for fixed& float poit Radix4_CFFT*/
#define ARMV7E_MDCT_SIZE_TYPE_T uint16_t /*for fixed & float point MDCT length of the DCT-IV*/
#define ARMV7E_MDCT_NORMALIZE_FACT_TYPE_T float32_t /*for fixed & float point MDCT Normalization FACTOR*/                   
#define ARMV7E_MDCT_STATE_TYPE_T  float32_t  
#define ARMV7E_MDCT_INPUT_TYPE_T  float32_t                             
#define ARM_STATUS arm_status        
#endif
#endif
# define CMSIS_MDCT_LENGTH_SUPPORT_MAX 4
static const int armv7e_mdct_scaled_support[CMSIS_MDCT_LENGTH_SUPPORT_MAX] = {
   4096, 2048,512, 128
};

void clt_mdct_forward_armv7e_fft(const mdct_lookup *l,
                                 kiss_fft_scalar *in,
                                 kiss_fft_scalar * OPUS_RESTRICT out,
                                 const opus_val16 *window,
                                 int overlap, int shift, int stride, int arch){

   int i;
   int N, N2, N4;
   VARDECL(kiss_fft_scalar, f);
   VARDECL(kiss_fft_cpx, f2);
   const kiss_fft_state *st = l->kfft[shift];
   const kiss_twiddle_scalar *trig;

   SAVE_STACK;

   N = l->n;
   trig = l->trig;
   for (i=0;i<shift;i++)
   {
      N >>= 1;
      trig += N;
   }
   N2 = N>>1;
   N4 = N>>2;

   ALLOC(f, N2, kiss_fft_scalar);
   ALLOC(f2, N4, kiss_fft_cpx);

/* Consider the input to be composed of four blocks: [a, b, c, d] */
   /* Window, shuffle, fold */
   {
      /* Temp pointers to make it really clear to the compiler what we're doing */
      const kiss_fft_scalar * OPUS_RESTRICT xp1 = in+(overlap>>1);
      const kiss_fft_scalar * OPUS_RESTRICT xp2 = in+N2-1+(overlap>>1);
      kiss_fft_scalar * OPUS_RESTRICT yp = f;
      const opus_val16 * OPUS_RESTRICT wp1 = window+(overlap>>1);
      const opus_val16 * OPUS_RESTRICT wp2 = window+(overlap>>1)-1;
      for(i=0;i<((overlap+3)>>2);i++)
      {
         /* Real part arranged as -d-cR, Imag part arranged as -b+aR*/
         *yp++ = MULT16_32_Q15(*wp2, xp1[N2]) + MULT16_32_Q15(*wp1,*xp2);
         *yp++ = MULT16_32_Q15(*wp1, *xp1)    - MULT16_32_Q15(*wp2, xp2[-N2]);
         xp1+=2;
         xp2-=2;
         wp1+=2;
         wp2-=2;
      }
      wp1 = window;
      wp2 = window+overlap-1;
      for(;i<N4-((overlap+3)>>2);i++)
      {
         /* Real part arranged as a-bR, Imag part arranged as -c-dR */
         *yp++ = *xp2;
         *yp++ = *xp1;
         xp1+=2;
         xp2-=2;
      }
      for(;i<(overlap<<2);i++)
      {
         /* Real part arranged as a-bR, Imag part arranged as -c-dR */
         *yp++ =  -MULT16_32_Q15(*wp1, xp1[-N2]) + MULT16_32_Q15(*wp2, *xp2);
         *yp++ = MULT16_32_Q15(*wp2, *xp1)     + MULT16_32_Q15(*wp1, xp2[N2]);
         xp1+=2;
         xp2-=2;
         wp1+=2;
         wp2-=2;
      }
   }

    /* Pre-rotation */
   {
      kiss_fft_scalar * OPUS_RESTRICT yp = f;
      const kiss_twiddle_scalar *t = &trig[0];
      for(i=0;i<N4;i++)
      {
         kiss_fft_cpx yc;
         kiss_twiddle_scalar t0, t1;
         kiss_fft_scalar re, im, yr, yi;
         t0 = t[i];
         t1 = t[N4+i];
         re = *yp++;
         im = *yp++;
         yr = S_MUL(re,t0)  -  S_MUL(im,t1);
         yi = S_MUL(im,t0)  +  S_MUL(re,t1);
         yc.r = yr;
         yc.i = yi;
         f2[i] = yc;
      }
   }

opus_fft(st, f2, (kiss_fft_cpx *)f, arch);

/* Post-rotate */
   {
      /* Temp pointers to make it really clear to the compiler what we're doing */
      const kiss_fft_cpx * OPUS_RESTRICT fp = (kiss_fft_cpx *)f;
      kiss_fft_scalar * OPUS_RESTRICT yp1 = out;
      kiss_fft_scalar * OPUS_RESTRICT yp2 = out+stride*(N2-1);
      const kiss_twiddle_scalar *t = &trig[0];
      /* Temp pointers to make it really clear to the compiler what we're doing */
      for(i=0;i<N4;i++)
      {
         kiss_fft_scalar yr, yi;
         yr = S_MUL(fp->i,t[N4+i]) - S_MUL(fp->r,t[i]);
         yi = S_MUL(fp->r,t[N4+i]) + S_MUL(fp->i,t[i]);
         *yp1 = yr;
         *yp2 = yi;
         fp++;
         yp1 += 2*stride;
         yp2 -= 2*stride;
      }
   }
   RESTORE_STACK;   
}


void clt_mdct_backward_armv7e_fft(const mdct_lookup *l,
                            kiss_fft_scalar *in,
                            kiss_fft_scalar * OPUS_RESTRICT out,
                            const opus_val16 * OPUS_RESTRICT window,
                            int overlap, int shift, int stride, int arch){
int i;
int N, N2, N4;
VARDECL(kiss_fft_scalar, f);
const kiss_twiddle_scalar *trig;
const kiss_fft_state *st = l->kfft[shift];

N = l->n;
   trig = l->trig;
   for (i=0;i<shift;i++)
   {
      N >>= 1;
      trig += N;
   }
   N2 = N>>1;
   N4 = N>>2;

   ALLOC(f, N2, kiss_fft_scalar);

/* Pre-rotate */
   {
      /* Temp pointers to make it really clear to the compiler what we're doing */
      const kiss_fft_scalar * OPUS_RESTRICT xp1 = in;
      const kiss_fft_scalar * OPUS_RESTRICT xp2 = in+stride*(N2-1);
      kiss_fft_scalar * OPUS_RESTRICT yp = f;
      const kiss_twiddle_scalar * OPUS_RESTRICT t = &trig[0];
      for(i=0;i<N4;i++)
      {
         kiss_fft_scalar yr, yi;
         yr = S_MUL(*xp2, t[i]) + S_MUL(*xp1, t[N4+i]);
         yi = S_MUL(*xp1, t[i]) - S_MUL(*xp2, t[N4+i]);
         yp[2*i] = yr;
         yp[2*i+1] = yi;
         xp1+=2*stride;
         xp2-=2*stride;
      }
   }

   opus_ifft(st, (kiss_fft_cpx *)f, (kiss_fft_cpx*)(out+(overlap>>1)), arch);

   /* Post-rotate and de-shuffle from both ends of the buffer at once to make
      it in-place. */
   {
      kiss_fft_scalar * yp0 = out+(overlap>>1);
      kiss_fft_scalar * yp1 = out+(overlap>>1)+N2-2;
      const kiss_twiddle_scalar *t = &trig[0];
      /* Loop to (N4+1)>>1 to handle odd N4. When N4 is odd, the
         middle pair will be computed twice. */
      for(i=0;i<(N4+1)>>1;i++)
      {
         kiss_fft_scalar re, im, yr, yi;
         kiss_twiddle_scalar t0, t1;
         re = yp0[0];
         im = yp0[1];
         t0 = t[i];
         t1 = t[N4+i];
         /* We'd scale up by 2 here, but instead it's done when mixing the windows */
         yr = S_MUL(re,t0) + S_MUL(im,t1);
         yi = S_MUL(re,t1) - S_MUL(im,t0);
         re = yp1[0];
         im = yp1[1];
         yp0[0] = yr;
         yp1[1] = yi;

         t0 = t[(N4-i-1)];
         t1 = t[(N2-i-1)];
         /* We'd scale up by 2 here, but instead it's done when mixing the windows */
         yr = S_MUL(re,t0) + S_MUL(im,t1);
         yi = S_MUL(re,t1) - S_MUL(im,t0);
         yp1[0] = yr;
         yp0[1] = yi;
         yp0 += 2;
         yp1 -= 2;
      }
   }

   /* Mirror on both sides for TDAC */
   {
      kiss_fft_scalar * OPUS_RESTRICT xp1 = out+overlap-1;
      kiss_fft_scalar * OPUS_RESTRICT yp1 = out;
      const opus_val16 * OPUS_RESTRICT wp1 = window;
      const opus_val16 * OPUS_RESTRICT wp2 = window+overlap-1;

      for(i = 0; i < overlap/2; i++)
      {
         kiss_fft_scalar x1, x2;
         x1 = *xp1;
         x2 = *yp1;
         *yp1++ = MULT16_32_Q15(*wp2, x2) - MULT16_32_Q15(*wp1, x1);
         *xp1-- = MULT16_32_Q15(*wp1, x2) + MULT16_32_Q15(*wp2, x1);
         wp1++;
         wp2--;
      }
   }
   RESTORE_STACK;
}

int clt_mdct_forward_armv7e_dct4(const mdct_lookup *l,
                           kiss_fft_scalar *in,
                           kiss_fft_scalar * OPUS_RESTRICT out,
                           const opus_val16 *window,
                           int overlap, int shift, int stride, int arch){

   int i;
   int N, N2, N4,Nref;
   int ret = MDCT_SUCESS;
   VARDECL(float32_t, f);
   VARDECL(kiss_fft_scalar, fret);

   N = l->n;
   //trig = l->trig;
   for (i=0;i<shift;i++)
   {
      N >>= 1;
   }

   N2 = N>>1;
   N4 = N>>2;
    Nref= N - overlap;
   SAVE_STACK;/*for security we save the stack*/

/*checking if the library support the input length*/

   for (i = 0; i < CMSIS_MDCT_LENGTH_SUPPORT_MAX; i++) {
      if(N2 == armv7e_mdct_scaled_support[i])
         break;
   }

   if (i == CMSIS_MDCT_LENGTH_SUPPORT_MAX) {
      /* This nfft length  is not supported in armv7e */
       PRINTF("An error occour: (MDCT_INVALID_LENGTH) error number:%d\n",MDCT_INVALID_LENGTH);
      return ret=MDCT_INVALID_LENGTH;
   }
/*initialize the variables and structures neeeded by the */
#if defined(USE_LIB)
   VARDECL(ARMV7E_MDCT_STATE_TYPE_T,p_StateBuf);
   ALLOC(f, N2, float32_t);
   ALLOC(fret, N2, kiss_fft_scalar);

/* STRUCTURES_USED_BY_THE_MDCT_COMPUTATION*/
  ARMV7E_MDCT_INSTANCE_TYPE_T S;
  ARMV7E_RFFT_INSTANCE_TYPE_T S_RFFT;
  ARMV7E_CFFT_R4_INSTANCE_TYPE_T  S_CFFT;
  ARMV7E_MDCT_SIZE_TYPE_T NT= N2;
  ARMV7E_MDCT_SIZE_TYPE_T NT2= N4;
  ARMV7E_MDCT_NORMALIZE_FACT_TYPE_T normalize = sqrt(2/N2);
  ALLOC(p_StateBuf,N2,ARMV7E_MDCT_STATE_TYPE_T);
  ARM_STATUS dct_status;

   dct_status=arm_dct4_init_f32(&S,&S_RFFT,&S_CFFT,NT,NT2,normalize);
      if(dct_status!=ARM_MATH_SUCCESS){
         PRINTF("An Error occour:(MDCT_INITIALIZATION_ERROR) instatiate the DCT-IV in the ARM-lib, error number: %d\n",dct_status);
         return ret=MDCT_INITIALIZATION_ERROR;
      }
   /*N-pemutations as a preprocessing*/
  {
      /* Consider the input to be composed of four blocks: [a, b, c, d] */
      /* Window, shuffle, fold */

      /* Temp pointers to make it really clear to the compiler what we're doing */
      const kiss_fft_scalar * OPUS_RESTRICT xp1 = in+(overlap>>1);
      const kiss_fft_scalar * OPUS_RESTRICT xp2 = in+N-1-(overlap>>1);
      kiss_fft_scalar * OPUS_RESTRICT yp = fret;
      const opus_val16 * OPUS_RESTRICT wp1 = window+(overlap>>1);
      const opus_val16 * OPUS_RESTRICT wp2 = window+(overlap>>1)-1;


      for(i=0;i<(overlap>>1);i++){
         *yp++=-(MULT16_32_Q15(*wp2, xp1[Nref]) + MULT16_32_Q15(*wp1, *xp2));
         xp1+=1;
         xp2-=1;
         wp1+=1;
         wp2-=1;
      }
      wp1 = window;
      wp2 = window+overlap-1;

      for(;i<(overlap);i++){
         *yp++= MULT16_32_Q15(*wp1, xp1[-overlap]) - MULT16_32_Q15(*wp2, *xp2);
         xp1+=1;
         xp2-=1;
         wp1+=1;
         wp2-=1; 
      }

      for(;i<N2;i++)
      {
         *yp++ = *xp2;
         xp1+=1;
         xp2-=1;
      }

      for(i=0;i<N2;i++){
          f[i]=(float32_t)(fret[i]);
        }
  }

 arm_dct4_f32(&S,p_StateBuf,f);
 for(i=0;i<N2;i++){
        fret[i]=((int32_t)(f[i]))>>3;
        out[i]=fret[i];
        }
     
  RESTORE_STACK;
  free(f);
  free(p_StateBuf);
#endif
  return ret;     
}


int clt_mdct_backward_armv7e_dct4(const mdct_lookup *l,
                           kiss_fft_scalar *in,
                           kiss_fft_scalar * OPUS_RESTRICT out,
                           const opus_val16 *window,
                           int overlap, int shift, int stride, int arch)
{
return 0;
}



void clt_mdct_forward_armv7e(const mdct_lookup *l,
                           kiss_fft_scalar *in,
                           kiss_fft_scalar * OPUS_RESTRICT out,
                           const opus_val16 *window,
                           int overlap, int shift, int stride, int arch){
#if defined(ARM_MATH_CM4)
#if defined(USED_ARM_FFT)
   clt_mdct_forward_armv7e_fft(l,in,out,window,overlap, shift, stride,arch);   
#else
   int ret;
   ret=clt_mdct_forward_armv7e_dct4(l,in,out,window,overlap, shift, stride,arch);
      if(ret==MDCT_INVALID_LENGTH || ret==MDCT_INITIALIZATION_ERROR){
         clt_mdct_forward_armv7e_fft(l,in,out,window,overlap, shift, stride,arch);
      }
#endif
#endif   
}

void clt_mdct_backward_armv7e(const mdct_lookup *l,
                            kiss_fft_scalar *in,
                            kiss_fft_scalar * OPUS_RESTRICT out,
                            const opus_val16 * OPUS_RESTRICT window,
                            int overlap, int shift, int stride, int arch){
#if defined(ARM_MATH_CM4)
#if defined(USED_ARM_FFT)
   clt_mdct_backward_armv7e_fft(l,in,out,window,overlap, shift, stride,arch);
#else
    int ret;
   ret=clt_mdct_backward_armv7e_dct4(l,in,out,window,overlap, shift, stride,arch);
      if(ret==MDCT_INVALID_LENGTH || ret==MDCT_INITIALIZATION_ERROR){
         clt_mdct_backward_armv7e_fft(l,in,out,window,overlap, shift, stride,arch);
      }
#endif
#endif    
}

