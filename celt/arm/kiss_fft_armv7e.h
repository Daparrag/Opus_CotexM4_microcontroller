/*Copyright (c) 2013, Xiph.Org Foundation and contributors.

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.*/

#ifndef KISS_FFT_ARMv7E_H
#define KISS_FFT_ARMv7E_H

#ifdef HAVE_CONFIG_H /*new_D*/
#include "config.h"
#endif

#include "kiss_fft_armv5e.h"
#include "_kiss_fft_guts.h"
#include "arch.h"
#include "os_support.h"
#include "mathops.h"
#include "stack_alloc.h"
/*Complex Multiplication*/
void C_MUL32_armv7e(opus_val32 * in_cplxA, opus_val32 * in_cplxB, opus_val32 out_dest, opus_val32 nsamples){

}
/*Complex Conjugates input vector*/
void CPLX_CONJ_armv7e(opus_val32 * in_cplxA,opus_val32 * dest_vect,opus_val32 nsamples){

}


static void armv7e_kf_bfly2(kiss_fft_cpx * Fout,
                     int m,
                     int N)
{

	kiss_fft_cpx * Fout2 =(Fout+1);
	opus_uint32 blkCnt; /*loop unrolling*/

   int i;
   (void)m;
#ifdef CUSTOM_MODES
      if (m==1)
   {
      celt_assert(m==1);
      blkCnt = N >> 1u;
      kiss_fft_scalar acc1, acc2, acc3, acc4;
      kiss_fft_scalar * OPUS_RESTRICT tmp1;
      kiss_fft_scalar * OPUS_RESTRICT tmp2;
      kiss_fft_scalar * OPUS_RESTRICT tmp3;
      kiss_fft_scalar * OPUS_RESTRICT tmp4;
      while(blkCnt > 0u){
      	tmp1 = (kiss_fft_scalar *)Fout; 	/*Fout[2i].r*/
      	tmp2 = (kiss_fft_scalar *)Fout+1; 	/*Fout[2i+1].r*/
      	tmp3 = tmp1 + 1; 	   				/*Fout[2i].i*/
      	acc1 = *tmp1 + *tmp2;  				/*Fout[2i].r + Fout[2i+1].r*/
      	tmp4 = tmp2 + 1;       				/*Fout[2i+1].i*/
      	acc2 = *tmp1 - *tmp2;  				/*Fout[2i].r - Fout[2i+1].r*/
      	*tmp1 = acc1; 		   				/*Fout[2i].r = Fout[2i].r + Fout[2i+1].r*/
      	*tmp2 = acc2; 		   				/*Fout[2i+1].r = Fout[2i].r - Fout[2i+1].r*/
      	acc3  = *tmp3 + *tmp4; 				/*Fout[2i].i + Fout[2i+1].i*/
      	acc4  = *tmp3 - *tmp4; 				/*Fout[2i].i - Fout[2i+1].i*/
      	*tmp3 = acc3; 		   				/*Fout[2i].i = Fout[2i].i + Fout[2i+1].i*/
      	*tmp4 = acc4;						/*Fout[2i+1].i = Fout[2i].i - Fout[2i+1].i*/
      	Fout += 2;
      	blkCnt--;
      }

      /*if the block is not multiple of 4 compute the remaining samples here*/
	/** No loop unrolling is used. */
       blkCnt = N % 0x4u;
       while(blkCnt > 0u){
        kiss_fft_cpx t;
         Fout2 = Fout + 1;
         t = *Fout2;
         C_SUB( *Fout2 ,  *Fout , t );
         C_ADDTO( *Fout ,  t );
         Fout += 2;
         blkCnt--;
       }

      }else
#endif
      {
          opus_val16 tw;
          tw = QCONST16(0.7071067812f, 15);
          /* We know that m==4 here because the radix-2 is just after a radix-4  why*/
          celt_assert(m==4);



          kiss_fft_scalar * OPUS_RESTRICT xp1 = (kiss_fft_scalar *)Fout; 		/*Fout[8i]*/
          kiss_fft_scalar * OPUS_RESTRICT xp2 = (kiss_fft_scalar *)(Fout+2);	/*Fout[8i+2]*/
          kiss_fft_scalar * OPUS_RESTRICT xp4 = (kiss_fft_scalar *)(Fout+4);	/*Fout[8i+4]*/
          kiss_fft_scalar * OPUS_RESTRICT xp6 = (kiss_fft_scalar *)(Fout+6);	/*Fout[8i+6]*/
          


          for(i=0; i<(N>>3);i++){
            kiss_fft_scalar tmp1, tmp2, tmp3,tmp4;

          	  tmp1 = *xp1 + *xp4;					  /* Fout[8i].r + Fout[8i+4].r*/
          	  tmp2 = *xp1 - *xp4;					  /* Fout[8i].r - Fout[8i+4].r*/
              *xp1 = tmp1;                  /* Fout[8i].r = Fout[8i].r + Fout[8i+4].r*/    
          	  tmp3 = *xp2 + *(xp6+1);	 			/* Fout[8i+2].r + Fout[8i+6].i*/
              *xp4 = tmp2;                  /* Fout[8i+4].r = Fout[8i].r - Fout[8i+4].r*/
          	  tmp4 = *xp2 - *(xp6+1);				/* Fout[8i+2].r - Fout[8i+6].i*/

              *xp2 = tmp3;                  /* Fout[8i+2].r = Fout[8i+2].r + Fout[8i+6].i*/
              tmp1 = *(xp2+1) - *(xp6);     /* Fout[8i+2].i - Fout[8i+6].r*/
              tmp2 = *(xp2+1) + *(xp6);     /* Fout[8i+2].i + Fout[8i+6].r*/

              *xp6 = tmp4;                  /*  Fout[8i+6].r = Fout[8i+2].r - Fout[8i+6].i*/
              tmp3 = *(xp1+1) + *(xp4+1);    /* Fout[8i].i + Fout[8i+4].i*/ 

              *(xp2+1)=tmp1;                /* Fout[8i+2].i = Fout[8i+2].i - Fout[8i+6].r*/
              tmp4 = *(xp1+1) - *(xp4+1);   /* Fout[8i].i - Fout[8i+4].i*/
          	  

              *(xp6+1) = tmp2;              /* Fout[8i+6].i = Fout[8i+2].i + Fout[8i+6].r*/
              *(xp1+1) = tmp3;              /* Fout[8i].i = Fout[8i].i + Fout[8i+4].i*/
              *(xp4+1) = tmp4;              /* Fout[8i+4].i = Fout[8i].i - Fout[8i+4].i*/
          	  
          	  

          	  xp1 += 8;
          	  xp2 = xp1+2;
          	  xp4 = xp1+4;
          	  xp4 = xp1+6;
            }

            kiss_fft_scalar acc1, acc2, acc3, acc4,acc5 ;
            kiss_fft_scalar * OPUS_RESTRICT tmpx1;
            kiss_fft_scalar * OPUS_RESTRICT tmpx2;
            kiss_fft_scalar * OPUS_RESTRICT tmpx3;
            kiss_fft_scalar * OPUS_RESTRICT tmpx4;
            kiss_fft_scalar * OPUS_RESTRICT tmpx5;
            kiss_fft_scalar * OPUS_RESTRICT tmpx6;

            for(i=0; i<(N>>3);i++){

            tmpx1 = (kiss_fft_scalar *)(Fout2 + 4);                    /* Fout[8i+5].r */
            tmpx2 = (tmpx1 + 1);                                       /* Fout[8i+5].i */
            tmpx3 = (kiss_fft_scalar *)(Fout2 + 6);                    /* Fout[8i+7].r */
            acc1  =  S_MUL((*tmpx1 + *tmpx2),tw);                       /* (Fout[8i+5].r + Fout[8i+5].i) * tw */

            tmpx4 = (tmpx3 + 1);                                        /* Fout[8i+7].i */
            acc2  =  S_MUL((*tmpx2 - *tmpx1),tw);                       /* ((Fout[8i+5].i - Fout[8i+5].r) * tw) */
            
            tmpx5 = (kiss_fft_scalar *)(Fout2);                         /* Fout[8i+1].r */
            tmpx6 = (tmpx5+1);                                          /* Fout[8i+1].i */
            acc3  = S_MUL ((*tmpx4 - *tmpx3),tw);                       /* (Fout[8i+7].i - Fout[8i+7].r) * tw */

            

            *tmpx1 = (opus_int32)(*tmpx5 - acc1);                       /* Fout[8i+5].r = Fout[8i+1].r-(Fout[8i+5].r + Fout[8i+5].i) * tw */
            acc4  = S_MUL ((- *tmpx4 - *tmpx3),tw);                     /* (-Fout[8i+7].i - Fout[8i+7].r) * tw */


            

            *tmpx2 = (*tmpx6) - acc2;                                   /* Fout[8i+5].i = Fout[8i+1].i - (Fout[8i+5].r - Fout[8i+5].i) * tw */
            acc5 =  *tmpx5 + acc1;                                      /* Fout[8i+1].r - (Fout[8i+5].r + Fout[8i+5].i) * tw */


            tmpx1 = (kiss_fft_scalar *) (Fout2 + 2);                    /* Fout[8i+3].r */
            tmpx2 = tmpx1 + 1;                                          /* Fout[8i+3].i*/
            acc1 = *tmpx6 + acc2;                                       /* Fout[8i+1].i + (Fout[8i+5].i - Fout[8i+5].r) * tw */

            *tmpx3 =  *tmpx1 - acc3;                                     /* Fout[8i+7].r = (Fout[8i+7].r - (Fout[8i+7].i - Fout[8i+7].r) * tw  */
            acc2   =  *tmpx1 + acc3;                                    /* Fout[8i+3].r + ( (Fout[8i+7].i - Fout[8i+7].r) * tw ) */

            *tmpx4 = *tmpx2 - acc4;                                     /* Fout[8i+7].i = Fout[8i+7].i - (-Fout[8i+7].i - Fout[8i+7].r) * tw*/
            acc3   = *tmpx2 + acc4;                                     /* Fout[8i+3].i + (-Fout[8i+7].i - Fout[8i+7].r) * tw */
            *tmpx5 = acc5;                                              /* Fout[8i+1].r = Fout[8i+1].r + (Fout[8i+5].r + Fout[8i+5].i) * tw */
            *tmpx6 = acc1;                                              /* Fout[8i+1].i = Fout[8i+1].i + (Fout[8i+5].i - Fout[8i+5].r) * tw */
            *tmpx1 = acc2;                                              /* Fout[8i+3].r = Fout[8i+3].r + ( (Fout[8i+7].i - Fout[8i+7].r) * tw )*/
            *tmpx2 = acc3;                                              /*  Fout[8i+3].i = Fout[8i+3].i + (-Fout[8i+7].i - Fout[8i+7].r) * tw */
        	  Fout2 += 8;
          }


      }

}


#endif /*KISS_FFT_ARMv7E_H*/
