/*
 * firc.c
 *
 *      Implement C function of FIR Filter
 */

#include <stdio.h>
#include <usbstk5515.h>
#include "firc.h"

/* FIR filter implemented in C
 * Assumption: Int16 coef[TAPS] stores the filter coeff (16-bit)
 *				TAPS is defined as 49
 *				i is the input buffer index
 * 				sum is the filter output (rounded to 16-bit)
 */
Int16 coef[TAPS] = {
	#include "lpf.dat"
};
 
Int16 firc(Uint16 * in, Uint16 i)
{
	Int32 sum;
	Uint16 j;
	Uint32 index;
	sum=0;

	//The actual filter work
	for (j=0; j<TAPS; j++) {
		if(i>=j)
 			index = i - j;
		else 
 			index = TAPS + i - j;
		sum += (Int32)in[index] * (Int32)coef[j];
	}
	sum = sum + 0x00004000;			// So we round rather than truncate.
	return (Int16) (sum >> 15);  	// Conversion from 32 Q30 to 16 Q15.

}
