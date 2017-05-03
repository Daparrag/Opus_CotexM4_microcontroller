/**
 ******************************************************************************
 * @file    tracer_profiling.h
 * @author  Central Lab
 * @version V1.0.0
 * @date    28-January-2016
 * @brief   Header for codec_profiling.c module.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TRACER_PROFILING_H
#define __TRACER_PROFILING_H
/* Includes ------------------------------------------------------------------*/
#include "main_usb.h"
#include "cube_hal.h"
#include "stm32xx_it.h"

#define	PROF_ON	  	1
#define PROF_OFF  	0
#define	PROF_BUSY	2

#define MAX_ENTRIES   10

struct profile {
	int state;
	int already_setup;
};

struct profile_fun{
	int16_t num_child;
	int32_t deph_level;
	void* fun_addrs;
	int cycles;
	int16_t count;
	void* parent_addrs;
	struct profile_fun *next_fun[MAX_ENTRIES];
};

extern struct profile _profparam;




/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
void __attribute__ ((constructor,no_instrument_function)) trace_begin (void);
void __attribute__ ((destructor,no_instrument_function)) trace_end(void);
void __attribute__ ((no_instrument_function)) __cyg_profile_func_enter(void *func, void *caller);
void __attribute__ ((no_instrument_function)) __cyg_profile_func_exit(void *func, void *caller);
void __attribute__ ((no_instrument_function)) start_profile(int start);
void __attribute__ ((no_instrument_function)) Error_profile(void);

#endif /* __TRACER_PROFILING_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
