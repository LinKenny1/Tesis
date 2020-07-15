/*
 * Functions.h
 *
 *  Created on: Apr 28, 2018
 *      Author: linke
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

//Tansig transfer function.
void tansig (float A[], int s);

//Softmax transfer function.
void softmax(float A[], int s);

//Determines which gesture was made.
int Gesture (float A[]);

#endif /* FUNCTIONS_H_ */

