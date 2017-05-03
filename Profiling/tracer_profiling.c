/**
 ******************************************************************************
 * @file    tracer_profiling.c
 * @author  Central Labs
 * @version V1.0.0
 * @date    28-January-2016
 * @brief   This file provides Codec Profiling API
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

/* Includes ------------------------------------------------------------------*/
#include "tracer_profiling.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define PROF_ALLOC(name) name =(struct profile_fun*)malloc(sizeof(struct profile_fun))
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
char string_app[512];
static FILE * Profiling_log;
static char path[]="C:\\Users\\Homer\\Documents\\Profiling_M4-Optimus\\Projects\\Multi\\Applications\\CODEC_OPUS_OPT\\SW4STM32\\STM32F446RE-Nucleo\\CODEC_OPUS_OPT\\CODEC_OPUS_OPT\\cprof.out";
struct profile _profparam = {PROF_OFF, 0};
struct profile_fun * _prof_file;
struct profile_fun * current_parent;
int depth_print;
int32_t global_depth;
char in;
/*tick variables*/
volatile uint32_t *DWT_CYCCNT_PROFILE = (uint32_t *) 0xE0001004; //address of the register
volatile uint32_t *DWT_CONTROL_PROFILE = (uint32_t *) 0xE0001000; //address of the register
volatile uint32_t *SCB_DEMCR_PROFILE = (uint32_t *) 0xE000EDFC; //address of the register
volatile uint32_t av_profile = 0;
extern void  __attribute__ ((no_instrument_function)) initialise_monitor_handles(void);




void __attribute__ ((constructor,no_instrument_function))  trace_begin (void){
	struct profile * p = &_profparam;
	if(p->state!=PROF_BUSY)return;
	if(!p->already_setup){
		Profiling_log= fopen(path,"w+");
		if(Profiling_log!=NULL){
				fseek(Profiling_log,0,SEEK_SET);
				fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>",Profiling_log);
				sprintf(string_app,"<table xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n xsi:noNamespaceSchemaLocation=\"schema_table.xsd\">\n <header> <hdr_name_1>cycles</hdr_name_1> <hdr_name_2>fun_addrs</hdr_name_2> <hdr_name_3>fun_name</hdr_name_3> </header>\n");
				fputs(string_app,Profiling_log);
				fclose(Profiling_log);
				p->already_setup=1;
				p->state=PROF_ON;
		}else {
			Error_profile();
		}

	}

}

int __attribute__ ((no_instrument_function))check_address(void*func){
struct profile_fun *e=current_parent;
int i;
	for(i=0;i < e->num_child;i++){
		if(e->next_fun[i]->fun_addrs==func)return i;/*return the child index*/
	}
	return -1;
}


void __attribute__ ((no_instrument_function))generate_file(struct profile_fun *e){
	int i;
	for(i=0;i < e->num_child;i++){
		Profiling_log=fopen(path,"a");
		fputs("<row>\n",Profiling_log);
		fclose(Profiling_log);
		generate_file(e->next_fun[i]);
	}
/*this is a botton of the tree then map the node data*/
	Profiling_log=fopen(path,"a");
	sprintf(string_app," <cycles>%d</cycles>\n <Func_addr>%lx</Func_addr>\n <Func_name>%s</Func_name>\n<Ncalls>%d</Ncalls>\n",e->cycles,e->fun_addrs,
							__FUNCTION__,
							e->count
					) ;
	fputs(string_app,Profiling_log);
	sprintf(string_app,"</row> \n");
	fputs(string_app,Profiling_log);
	fclose(Profiling_log);
}


void __attribute__ ((destructor,no_instrument_function)) trace_end(void){
		struct profile *p = &_profparam;
		generate_file(current_parent);
		p->state=PROF_OFF;
		p->already_setup=0;
		free(current_parent);
		free(_prof_file);

}

int __attribute__ ((no_instrument_function)) __cyg_profile_insert_new_entry(void *func,struct profile_fun *e){
	struct profile_fun *child;
	if(current_parent->num_child < MAX_ENTRIES){
		PROF_ALLOC (child);
		if(child==NULL){printf("error:overlow memory problem\n");return -1;}/*overlow memory problem*/
		child->num_child=0;
		child->count=1;
		child->cycles=0;
		child->deph_level=global_depth+1;
		child->next_fun[0]=NULL;
		child->fun_addrs=func;
		child->parent_addrs=current_parent;
		current_parent->next_fun[current_parent->num_child]=child;
		current_parent->num_child+=1;
		/*setup the control variables*/
		global_depth+=1;

	}else{
		printf("current_parent cant not allocate a new child\n");
		return -1;

	}
return current_parent->num_child-1;/*if everything is ok return the index of the new child*/
}

void __attribute__ ((no_instrument_function)) __cyg_profile_func_enter(void *func, void *caller){
	struct profile *p = &_profparam;
	if(p->already_setup==0 && p->state==PROF_BUSY){
		trace_begin();
	}else if(p->already_setup==0 && p->state==PROF_OFF){
		return;
	}
	if(_prof_file==NULL){
		/*this is the fist entry i.e the root of the tree*/
		/*task:*/
		/*1) create the root*/
		/*2) setup the currrent_parent as the temporal root of the tree(useful in case of subtrees allocation)*/
		/*3) setup the var "in" which indicate that the function is not finished and the global_depth to 0 as the first tree level*/
		/*4)reset the counter for measure the ticks*/
		/*1. create the root*/
		PROF_ALLOC(_prof_file);
		//PROF_ALLOC(current_parent);
		_prof_file->num_child=0;
		_prof_file->count=1;
		_prof_file->cycles=0;
		_prof_file->deph_level=0;
		_prof_file->fun_addrs=func;
		_prof_file->parent_addrs=NULL;
		_prof_file->next_fun[0]=NULL;
		current_parent=_prof_file;
		//memcpy(current_parent,_prof_file,sizeof(struct profile_fun));/*2.setup the currrent_parent*/
		global_depth=0; in=0;/*3.setup the global_depth and in vars*/
		Profiling_log=fopen(path,"a");
		fputs("<row>\n",Profiling_log);
		fclose(Profiling_log);
		av_profile =0;
		*DWT_CYCCNT_PROFILE = 0; /* 4.reset the counter*/
		*DWT_CONTROL_PROFILE = *DWT_CONTROL_PROFILE | 1; /*start to counter the ticks*/

	}else{
		/*this is not the first element of the tree*/
		/*there are two cases:*/
		/*1) the address correspond to the current parent address*/
	*DWT_CONTROL_PROFILE = *DWT_CONTROL_PROFILE ^ 1; /* disable the counter*/
	av_profile += (*((uint32_t *) DWT_CYCCNT_PROFILE));/*get the current counter value*/

		if(current_parent->fun_addrs==func){
			int index=__cyg_profile_insert_new_entry(func,current_parent->next_fun[current_parent->num_child]);/*insert a new element*/
			/*1.1) should be a recursive call if "in=1" or a function called many times "in=0"*/
			if(index==-1)Error_profile();
			current_parent->count+=1;
				if(!in){
					/*function called many times*/
					current_parent->cycles+=av_profile;/*update the current_parent cyclecount*/

							/*restart the counter*/
							av_profile =0;
							*DWT_CYCCNT_PROFILE = 0; /* 4.reset the counter*/
							*DWT_CONTROL_PROFILE = *DWT_CONTROL_PROFILE | 1; /*start to counter the ticks*/
				}
				in=1;
				current_parent=current_parent->next_fun[index];/*going to the new current parent*/
			}else{
				/*It is necesary to evaluate the address of all the children*/
				/*in case in which exist a children with the same address the current_parent go into th child
				 * otherwise new child will be create*/
				in=1;/*setup the control in value*/
				int32_t child_index;
				if((child_index=check_address(func))!=-1){
					global_depth+=1;/*increment the depth value*/
					current_parent->cycles+=av_profile;/*update the parent cycles*/
					current_parent=current_parent->next_fun[child_index];/*update the current parent*/
					current_parent->count+=1;/*a new call of the same function*/
					/*reset the counter*/

				}else{
					/*this is a new entry therefore is necessary to create a new child*/
					int32_t index=__cyg_profile_insert_new_entry(func,current_parent->next_fun[current_parent->num_child]);
					if(index==-1)Error_profile();
					current_parent->cycles=av_profile;/*update the parent cycles*/
					current_parent=current_parent->next_fun[index];/*update the current parent*/
					/*reset the counter*/
					}
				/*reset the counter*/
					av_profile =0;
					*DWT_CYCCNT_PROFILE = 0; /* 4.reset the counter*/
					*DWT_CONTROL_PROFILE = *DWT_CONTROL_PROFILE | 1; /*start to counter the ticks*/
			}

	}
}



void __attribute__ ((no_instrument_function)) __cyg_profile_func_exit(void *func, void *caller){
	struct profile *p = &_profparam;
	if(p->already_setup!=1 && p->state!=PROF_ON){
		return;
	}

	*DWT_CONTROL_PROFILE = *DWT_CONTROL_PROFILE ^ 1; // disable the counter
	av_profile += (*((uint32_t *) DWT_CYCCNT_PROFILE));
	in=0;
	current_parent->cycles+=av_profile;/*update the cycles in the current_parent or the root*/
	if(global_depth){
		/*global_depth!=0 control variable indicates that we are not in the root of the tree*/
		global_depth-=1;/*decrease the depth*/
		current_parent=current_parent->parent_addrs;
		current_parent->cycles+=av_profile;/*update the cycles in the ancestor parent*/
		if(current_parent==NULL)Error_profile();/*something is wrong*/
	}
}


void __attribute__ ((no_instrument_function)) Error_profile(void) {
	while (1)
		;
}



void __attribute__ ((no_instrument_function)) start_profile(int start){
	if(start){
	struct profile * p = &_profparam;
	p->state=PROF_BUSY;
	p->already_setup=0;
	initialise_monitor_handles();
	printf("SemiHosting is on \n");
	}
}

