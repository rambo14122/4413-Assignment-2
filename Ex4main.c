/*
 * Audio Bypass Project
 */

#include <usbstk5515.h>
#include <usbstk5515_i2c.h>
#include <AIC_func.h>
#include <stdio.h>
#include <Dsplib.h>
#include<stdlib.h>
#include "sar.h"
#define TAPS	256
#define TCR0 *((ioport volatile Uint16 *)0x1810)
#define TIMCNT1_0 *((ioport volatile Uint16 *)0x1814)
#define TIME_START 0x8001
#define TIME_STOP 0x8000
#define GainNumber 7


Int16 dbuffer_left[TAPS+2] = {0};
Int16 dbuffer_right[TAPS+2] = {0};
int selection=0;

Int16 Gain[GainNumber]={0x080A,0x100A,0x16A7,0x2000,0x2D34,0x3FD9,0x7F65};
Uint16 start_time;
	Uint16 end_time;
	Uint16 delta_time;
	Int16 origin_band1[TAPS] = {
	#include "band1.dat"
	};

	Int16 origin_band2[TAPS] = {
	#include "band2.dat"
	};
	Int16 origin_band3[TAPS] = {
	#include "band3.dat"
	};
	
	Int16 modified_band1[TAPS];
	Int16 modified_band2[TAPS];
	Int16 modified_band3[TAPS];
	Int16 totalFilter[TAPS];
	
	int GainSelection[3]={3,3,3};
	int BandSelection=0;
	
	Int16 input_right[1], input_left[1]; //AIC inputs
	Int16 output_right[1],output_left[1]; //AIC outputs	
	void getMultiplication(Int16 *,int ,Int16 *);
	void addAllFilters(Int16 *, Int16 *, Int16 *, Int16 *);
	void getKeyPressed(void);
	void Init_Filter(void);
	
	
void main(void)
{
	Init_SAR();
	USBSTK5515_init(); 	//Initializing the Processor
	AIC_init(); 		//Initializing the Audio Codec
	TCR0=TIME_STOP;
	TCR0=TIME_START;
	
	Init_Filter();
	while(1)
	{	
		getKeyPressed();
		AIC_read2(&input_right[0], &input_left[0]);
		start_time=TIMCNT1_0;
		
		fir(&input_right[0],&totalFilter[0],&output_right[0],&dbuffer_right[0],1,TAPS);
		fir(&input_left[0],&totalFilter[0],&output_left[0],&dbuffer_left[0],1,TAPS);
		
		end_time=TIMCNT1_0;
		delta_time=end_time-start_time;
		AIC_write2(output_right[0],output_left[0]);
	}
}
void Init_Filter(void)
{
	getMultiplication(&origin_band1[0],3,&modified_band1[0]);
	getMultiplication(&origin_band2[0],3,&modified_band2[0]);
	getMultiplication(&origin_band3[0],3,&modified_band3[0]);
	addAllFilters(&modified_band1[0],&modified_band2[0],&modified_band3[0],&totalFilter[0]);
}
void getKeyPressed(void)
{
	Uint16 keyPressed=Get_Key_Human();
	if(keyPressed==NoKey)
	{
		return;
	}
	
	if(keyPressed==SW1)
	{
		printf("switch band\n");
		if(BandSelection==2)
		{
			BandSelection=0;
			return;
		}
		BandSelection++;
		return;
	}
	if(keyPressed==SW2)
	{
		printf("switch gain\n");
		if(GainSelection[BandSelection]==(GainNumber-1))
		{
			GainSelection[BandSelection]=0;
		}
		else
		{
			GainSelection[BandSelection]++;
		}
		
		switch(BandSelection)
		{
			case 0:
				getMultiplication(&origin_band1[0],GainSelection[0],&modified_band1[0]);
				break;
			case 1:
				getMultiplication(&origin_band2[0],GainSelection[1],&modified_band2[0]);
				break;
			case 2:
				getMultiplication(&origin_band3[0],GainSelection[2],&modified_band3[0]);
				break;
			default:
				break;
		}
		addAllFilters(&modified_band1[0],&modified_band2[0],&modified_band3[0],&totalFilter[0]);
	}
}

void addAllFilters(Int16 *input1, Int16 *input2, Int16 *input3, Int16 *output)
{
	int i;
	int j;
	for(i=0;i<TAPS;i++)
	{
		output[i]=input1[i]+input2[i]+input3[i];
	}
	for(j=0;j<TAPS;j+=20)
	{
		printf("%hi,",totalFilter[j]);
	}
	printf("\n");
}
void getMultiplication(Int16 *input,int selection,Int16 *output)
{
	int i;
	for(i=0;i<TAPS;i++)
	{
		output[i]=(Int16)(((Int32)input[i]*(Int32)Gain[selection])>>15);
	}
}

