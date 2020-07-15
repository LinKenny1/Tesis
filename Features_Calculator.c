/*
 * Features_Calculator.c
 *
 *  Created on: Apr 19, 2018
 *      Author: linke
 */

#include <stdio.h>
#include "board.h"
#include "MK22F51212.h"
#include "Features_Calculator.h"
#include "arm_math.h"

/*Auxiliary elements
 * ones, array to be filled with ones.
 * difout array to save the result of subtraction.
 * abs_difout array to save the absolute value of difout.
 * fx variables to save the features.
 * */
float ones[size]={0.0f};
float difout[size]={0.0f};
float abs_difout[size]={0.0f};
float f1, f2, f3, f4, f5;


/*Main function that calls all the other specific features functions
 * and saves the results where it should.
 */

void Features (float *measures, float *abs_measures, float *mmo, float *mpo, Feat *result)
{
	//if the aux array hasn't been filled, fill it.
	if((ones[0]!=1.0f))
	{
		arm_fill_f32 (1.0f, &ones[0], size);
	}

	//Get the absolute value of the measures array
	arm_abs_f32(measures, abs_measures, size);

	Mean_Value (abs_measures, &f1);
	//IEMG_calculator (measures, &ones[0], &f2); //Unused, redundant with MEAN.
	Variance (measures, &f3);
	WaveformL(measures, mmo, ones, &f4);
	WillisonAmp(measures, mpo, &f5);

    result->MEAN=f1;
    //result->IEMG=f2;
    result->WL=f3;
    result->VAR=f4;
    result->WAMP=f5;
}

/*Mean IEMG value*/
void Mean_Value ( float *abs_output, float *result)
{

	arm_mean_f32(abs_output, size, result);		//Calculate the mean value of the last N measures
}

/*IEMG - Unused*/
void IEMG_calculator (float *abs_output, float *ones, float *result)
{
    arm_dot_prod_f32(abs_output, ones, size, result);
}

/*Variance*/
void Variance (float *output, float *result)
{
	arm_var_f32(output, size, result);
}

/*WaveformLenght*/
void WaveformL (float *output, float *out_minus, float *ones, float *result)
{
	//Subtract measures-measures shifted to the left.
    arm_sub_f32(output, out_minus, &difout[0], size);
    //Absolute value.
    arm_abs_f32(&difout[0], &abs_difout[0], size);
    //Sum of all array items.
    arm_dot_prod_f32(&abs_difout[0], ones, size, result);
}

/*Willison Amplitude*/
void WillisonAmp(float*output, float *out_plus, float *result)
{
	//Subtract measures-measures shifted to the right.
	arm_sub_f32(output, out_plus, &difout[0], size);

	//Absolute value.
	arm_abs_f32(&difout[0], &abs_difout[0], size);

	//Because the result its basically a counter it has to be reset on every run.
	*result=0;

	//Checks if the difference its greater than the threshold.
	for(int m=0;m<size;m++)
	{
		if(abs_difout[m]>=0.01f) (*result)++;
	}
}


