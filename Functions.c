/*
 * Functions.c
 *
 *  Created on: Apr 28, 2018
 *      Author: linke
 */
#include <stdio.h>
#include "board.h"
#include "MK22F51212.h"
#include "Functions.h"
#include "Features_Calculator.h"
#include "arm_math.h"
#include "math.h"

float one[NG];

//Tansig transfer function.
void tansig (float A[],int s)
{
	int i;
	float b;
	float c;

	/*
	 * Im not giving a math class, look for the math expression
	 * and this is a recreation of it.
	 * https://la.mathworks.com/help/nnet/ref/tansig.html
	 * */
	for(i=0;i<(s);i++)
	{
		b=-2*A[i];
		c=expf(b);
		A[i] = (2/(1+c))-1;
	}

}

//SoftMax transfer function.
void softmax(float A[], int s)
{
	int i;
	float b;
	float c=0;

	/*
	 * Im not giving a math class, look for the math expression
	 * and this is a recreation of it.
	 * https://la.mathworks.com/help/nnet/ref/softmax.html
	 * */
	for(i=0;i<(s);i++)
	{
		b=expf(A[i]);
		A[i] = b;
		c=c+b;
	}
	for(i=0;i<(s);i++)
	{
		A[i] = A[i]/c;
	}
}

/*
 * Determines which gesture was made.
 * If the probability of one gesture it's greater then the rest combined, that's the gesture.
 */
int Gesture (float A[])
{
	int result;
	float a=0;

	//if the aux array hasn't been filled, fill it.
	if((one[0]!=1.0f))
	{
		arm_fill_f32 (1.0f, &one[0], NG);
	}

	//Sum of all array items done with dot product.
	arm_dot_prod_f32(&A[0], &one[0], NG, &a);

	for(int i=0; i<=NG; i++)
	{
		//Checks if the i item is greater than the rest combined.
		if(A[i]>(a-A[i]))
		{
			result=i;
			break;
		}
	}

	//Returns the index of the gesture, it starts at 0 not 1!
	return result;
}

