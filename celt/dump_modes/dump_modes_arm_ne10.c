/* Copyright (c) 2015 Xiph.Org Foundation
   Written by Viswanath Puttagunta */
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

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "modes.h"
#include "dump_modes_arch.h"
#include <NE10_dsp.h>

#if !defined(FIXED_POINT)
# define NE10_FFT_CFG_TYPE_T ne10_fft_cfg_float32_t
# define NE10_FFT_CPX_TYPE_T_STR "ne10_fft_cpx_float32_t"
# define NE10_FFT_STATE_TYPE_T_STR "ne10_fft_state_float32_t"
#else
# define NE10_FFT_CFG_TYPE_T ne10_fft_cfg_int32_t
# define NE10_FFT_CPX_TYPE_T_STR "ne10_fft_cpx_int32_t"
# define NE10_FFT_STATE_TYPE_T_STR "ne10_fft_state_int32_t"
#endif

static FILE *file;

void dump_modes_arch_init(CELTMode **modes, int nb_modes)
{
   int i;

   file = fopen(ARM_NE10_ARCH_FILE_NAME, "w");
   fprintf(file, "/* The contents of this file was automatically generated by\n");
   fprintf(file, " * dump_mode_arm_ne10.c with arguments:");
   for (i=0;i<nb_modes;i++)
   {
      CELTMode *mode = modes[i];
      fprintf(file, " %d %d",mode->Fs,mode->shortMdctSize*mode->nbShortMdcts);
   }
   fprintf(file, "\n * It contains static definitions for some pre-defined modes. */\n");
   fprintf(file, "#include <NE10_types.h>\n\n");
}

void dump_modes_arch_finalize()
{
   fclose(file);
}

void dump_mode_arch(CELTMode *mode)
{
   int k, j;
   int mdctSize;

   mdctSize = mode->shortMdctSize*mode->nbShortMdcts;

   fprintf(file, "#ifndef NE10_FFT_PARAMS%d_%d\n", mode->Fs, mdctSize);
   fprintf(file, "#define NE10_FFT_PARAMS%d_%d\n", mode->Fs, mdctSize);
   /* cfg->factors */
   for(k=0;k<=mode->mdct.maxshift;k++) {
      NE10_FFT_CFG_TYPE_T cfg;
      cfg = (NE10_FFT_CFG_TYPE_T)mode->mdct.kfft[k]->arch_fft->priv;
      if (!cfg)
         continue;
      fprintf(file, "static const ne10_int32_t ne10_factors_%d[%d] = {\n",
              mode->mdct.kfft[k]->nfft, (NE10_MAXFACTORS * 2));
      for(j=0;j<(NE10_MAXFACTORS * 2);j++) {
         fprintf(file, "%d,%c", cfg->factors[j],(j+16)%15==0?'\n':' ');
      }
      fprintf (file, "};\n");
   }

   /* cfg->twiddles */
   for(k=0;k<=mode->mdct.maxshift;k++) {
      NE10_FFT_CFG_TYPE_T cfg;
      cfg = (NE10_FFT_CFG_TYPE_T)mode->mdct.kfft[k]->arch_fft->priv;
      if (!cfg)
         continue;
      fprintf(file, "static const %s ne10_twiddles_%d[%d] = {\n",
              NE10_FFT_CPX_TYPE_T_STR, mode->mdct.kfft[k]->nfft,
              mode->mdct.kfft[k]->nfft);
      for(j=0;j<mode->mdct.kfft[k]->nfft;j++) {
#if !defined(FIXED_POINT)
         fprintf(file, "{%#0.8gf,%#0.8gf},%c",
                 cfg->twiddles[j].r, cfg->twiddles[j].i,(j+4)%3==0?'\n':' ');
#else
         fprintf(file, "{%d,%d},%c",
                 cfg->twiddles[j].r, cfg->twiddles[j].i,(j+4)%3==0?'\n':' ');
#endif
      }
      fprintf (file, "};\n");
   }

   for(k=0;k<=mode->mdct.maxshift;k++) {
      NE10_FFT_CFG_TYPE_T cfg;
      cfg = (NE10_FFT_CFG_TYPE_T)mode->mdct.kfft[k]->arch_fft->priv;
      if (!cfg) {
         fprintf(file, "/* Ne10 does not support scaled FFT for length = %d */\n",
                 mode->mdct.kfft[k]->nfft);
         fprintf(file, "static const arch_fft_state cfg_arch_%d = {\n", mode->mdct.kfft[k]->nfft);
         fprintf(file, "0,\n");
         fprintf(file, "NULL\n");
         fprintf(file, "};\n");
         continue;
      }
      fprintf(file, "static const %s %s_%d = {\n", NE10_FFT_STATE_TYPE_T_STR,
              NE10_FFT_STATE_TYPE_T_STR, mode->mdct.kfft[k]->nfft);
      fprintf(file, "%d,\n", cfg->nfft);
      fprintf(file, "(ne10_int32_t *)ne10_factors_%d,\n", mode->mdct.kfft[k]->nfft);
      fprintf(file, "(%s *)ne10_twiddles_%d,\n",
              NE10_FFT_CPX_TYPE_T_STR, mode->mdct.kfft[k]->nfft);
      fprintf(file, "NULL,\n");  /* buffer */
      fprintf(file, "(%s *)&ne10_twiddles_%d[%d],\n",
              NE10_FFT_CPX_TYPE_T_STR, mode->mdct.kfft[k]->nfft, cfg->nfft);
#if !defined(FIXED_POINT)
      fprintf(file, "/* is_forward_scaled = true */\n");
      fprintf(file, "(ne10_int32_t) 1,\n");
      fprintf(file, "/* is_backward_scaled = false */\n");
      fprintf(file, "(ne10_int32_t) 0,\n");
#endif
      fprintf(file, "};\n");

      fprintf(file, "static const arch_fft_state cfg_arch_%d = {\n",
              mode->mdct.kfft[k]->nfft);
      fprintf(file, "1,\n");
      fprintf(file, "(void *)&%s_%d,\n",
              NE10_FFT_STATE_TYPE_T_STR, mode->mdct.kfft[k]->nfft);
      fprintf(file, "};\n\n");
   }
   fprintf(file, "#endif  /* end NE10_FFT_PARAMS%d_%d */\n", mode->Fs, mdctSize);
}
