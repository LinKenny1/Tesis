/*
 * Features_Calculator.h
 *
 *  Created on: Apr 19, 2018
 *      Author: linke
 */

#ifndef FEATURES_CALCULATOR_H_
#define FEATURES_CALCULATOR_H_

#define size 1000	//Number of window measures.
#define NG 4		//Number of Gestures.
#define HL 10		//Number of neurons in hidden layer.

typedef struct Feat {
	float VOLT;			//ADC value transformed. Isn't a feature but just to keep track.
	float MEAN;			//Mean value of the measures window.
	//float IEMG;		//Cumulative value of EMG measures window. Unused
	float VAR;			//Variance.
	float WL;			//Waveform Length.
	float SSC;			//Samples sign count.
	float WAMP;			//Willison Amplitude.
}Feat;

/*Calculates all features for the passed signal.*/
void Features (float *measures, float *abs_measures, float *mmo, float *mpo, Feat *result);

/*MEAN*/
void Mean_Value ( float *abs_output, float *result);

/*IEMG - Unused*/
void IEMG_calculator (float *abs_output, float *ones, float *result);

/*Variance*/
void Variance (float *output, float *result);

/*WaveformLenght*/
void WaveformL (float *output, float *out_minus, float *difout, float *result);

/*Willison Amplitude*/
void WillisonAmp (float*output, float *out_plus, float *result);


#endif /* FEATURES_CALCULATOR_H_ */
