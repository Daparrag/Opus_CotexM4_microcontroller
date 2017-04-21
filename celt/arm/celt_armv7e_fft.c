/* Copyright (c) 2015 
   Written by  */
/**
   @file celt_armv7e_fft.c
   @brief ARMv7e optimizations for fft using CMSIS library
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
#if defined (USE_LIB)
#include <arm_math.h>
#include <arm_const_structs.h>
#endif
#include "os_support.h"
#include "stack_alloc.h"
#include "kiss_fft.h"

#if defined (USE_LIB)
#if !defined(FIXED_POINT)
#else
# define ARMV7E_CFFT_CPX_TYPE_T q31_t
# define ARMV7E_CFFT_INSTANCE_TYPE_T const static arm_cfft_instance_q31
# define ARMV7E_CFFT_INSTANCE_VALUE(length)  arm_cfft_sR_q31_len ## length//arm_cfft_sR_q31_len ## length
# define CMSIS_FFT_C2C_1D_ARMV7E_FUNC arm_cfft_q31
# define CMSIS_CFFT_LENGTH_SUPPORT_MAX 8
#define FFT_INIT_INSTANCE(n, VAR)  switch(n)                                               \
                   { case 4096: VAR=&ARMV7E_CFFT_INSTANCE_VALUE(4096); break;                      \
                     case 2048: VAR=&ARMV7E_CFFT_INSTANCE_VALUE(2048); break;                      \
                     case 1024: VAR=&ARMV7E_CFFT_INSTANCE_VALUE(1024); break;                      \
                     case 512: VAR=&ARMV7E_CFFT_INSTANCE_VALUE(512); break;                        \
                     case 128: VAR=&ARMV7E_CFFT_INSTANCE_VALUE(128); break;                        \
                     case 64: VAR=&ARMV7E_CFFT_INSTANCE_VALUE(64); break;                          \
                     case 32: VAR=&ARMV7E_CFFT_INSTANCE_VALUE(32); break;                          \
                     case 16: VAR=&ARMV7E_CFFT_INSTANCE_VALUE(16); break;                          \
                   }
#endif
#endif
#if defined(CUSTOM_MODES)

# define CMSIS_CFFT_LENGTH_SUPPORT_MAX 8
static const int armv7e_fft_scaled_support[CMSIS_CFFT_LENGTH_SUPPORT_MAX] = {
   4096, 2048, 1024, 512, 128, 64, 32, 16
};

int opus_fft_alloc_armv7e(kiss_fft_state *st){

	int i;
   size_t memneeded = sizeof(struct arch_fft_state);

   st->arch_fft = (arch_fft_state *)opus_alloc(memneeded);
   if (!st->arch_fft)
      return -1;
	
	for (i = 0; i < CMSIS_CFFT_LENGTH_SUPPORT_MAX; i++) {
      if(st->nfft == armv7e_fft_scaled_support[i])
         break;
   }

   if (i == CMSIS_CFFT_LENGTH_SUPPORT_MAX) {
      /* This nfft length  is not supported in armv7e */
      st->arch_fft->is_supported = 0;
      st->arch_fft->priv = NULL;
   }

   else {
   		st->arch_fft->is_supported = 1;
   		st->arch_fft->priv = NULL;
   }
    return 0;
}

void opus_fft_free_armv7e(kiss_fft_state *st)
{
   if (!st->arch_fft)
      return;
   opus_free(st->arch_fft);
}
#endif




void opus_fft_armv7e(const kiss_fft_state *st,
                   const kiss_fft_cpx *fin,
                   kiss_fft_cpx *fout
                   	)
{
	size_t buflen = sizeof(kiss_fft_cpx)*(st->nfft);
	if(!st->arch_fft->is_supported){
		/* This nfft length (scaled fft) not supported in arm-CMSIS lib */
		opus_fft_c(st, fin, fout);
	}
	else{


#if !defined(FIXED_POINT)
		/*use the float fft*/
#else
#if defined (USE_LIB)
	  ARMV7E_CFFT_INSTANCE_TYPE_T *SC;//=&arm_cfft_sR_q31_len16 
	  FFT_INIT_INSTANCE(st->nfft,SC);
	  memcpy(fout, fin, buflen);
	  CMSIS_FFT_C2C_1D_ARMV7E_FUNC(SC,(ARMV7E_CFFT_CPX_TYPE_T*)fout,1,1);
		/*use the fixed_point fft */
#endif
#endif		
	}
}


void opus_ifft_armv7e(const kiss_fft_state *st,
                    const kiss_fft_cpx *fin,
                    kiss_fft_cpx *fout)
{
	size_t buflen = sizeof(kiss_fft_cpx)*(st->nfft);
	if(!st->arch_fft->is_supported){
		/* This nfft length (scaled fft) not supported in arm-CMSIS lib */
		opus_ifft_c(st, fin, fout);
	}else{

#if !defined(FIXED_POINT)
		/*use the float fft*/
#else
#if defined (USE_LIB)
	  ARMV7E_CFFT_INSTANCE_TYPE_T *SC;//=&arm_cfft_sR_q31_len16 
	  FFT_INIT_INSTANCE(st->nfft,SC);
	  memcpy(fout, fin, buflen);
	  CMSIS_FFT_C2C_1D_ARMV7E_FUNC(SC,(ARMV7E_CFFT_CPX_TYPE_T*)fout,0,1);
		/*use the fixed_point fft */
#endif
#endif			
	}

}
