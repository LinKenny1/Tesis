/*
 * Copyright 2016-2018 NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @file    Hand_Analisis.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK22F51212.h"
#include "fsl_debug_console.h"
#include "arm_math.h"
#include "fsl_ftm.h"
#include "Features_Calculator.h"
#include "Functions.h"


/*ADC Constants*/
#define ADC1_Start ADC_Interrupt_Handler
#define ADC2_Start ADC2_Interrupt_Handler
#define Vref 3.300
#define bits 65536.0

/*
 * These constants are the number of window measures, number of trained gestures
 * and number of neurons in hidden layer. They are defined on Features_Calculator.h.
 * If the number of gestures changes, update the value.
 * */
#define N size
#define G NG
#define Ne HL

volatile uint32_t valor1;			//Variable to save the ADC measure value.
volatile uint32_t valor2;			//Variable to save the ADC measure value.
volatile bool ADC_Flag1 = false;	//Flag for ADC measure
volatile bool ADC_Flag2 = false;	//Flag for ADC measure


//Initialize the ADCs
void ADC1_Start(void){
	ADC_Flag1 = true;
	valor1 = ADC16_GetChannelConversionValue(ADC_1_PERIPHERAL,0u);
}
void ADC2_Start(void){
	ADC_Flag2 = true;
	valor2 = ADC16_GetChannelConversionValue(ADC_2_PERIPHERAL,0u);
}

/*
 * Neural Network Data
 * */
//Array to save all the inputs (features) plus the bias.
float input[Ne+1];

/*
 * BW1 & BW2 are the NN weight arrays.
 * These are the arrays that have to be refreshed when the NN it's retrained.
 * They are "converted" to matrix later.
 * */
float BW1[]= {0.387013,-3.7217,-0.105211,5.82028,0.0086538,-0.254089,5.01615,-0.00977728,-9.2619,-0.0363502,-0.27898,-2.50321,2.69253,-0.220488,18.1042,0.191931,0.0132072,-0.771766,-0.230785,8.11876,0.248018,-0.0145511,	-1.57895,-2.37682,0.419658,20.8802,0.0355196,0.711133,0.237763,-0.0628312,12.2687,0.0226422,-0.0326549,	4.51197,-2.06664,0.0976717,22.2227,0.065327,0.0229626,-6.30569,0.0540829,4.5099,0.037585,0.0392922,	-0.610813,-7.31294,0.56197,22.1472,-0.10213,0.590431,2.35736,0.410547,-11.4643,-0.0679317,-0.0324921,	-1.65986,-2.60551,0.0487882,-0.854315,0.246824,0.607218,1.58287,0.0515319,-13.9679,0.138308,0.723637,	-2.6013,0.718456,0.0670275,-15.363,-0.0229018,-0.148285,0.426316,0.00704314,-7.39401,-0.0368661,-0.179606,	-2.51109,-1.29187,-0.225269,34.8558,0.283184,0.0282652,2.77611,-0.504365,10.5615,0.209483,0.059099,	0.37877,1.25601,-0.0367713,-26.1169,0.102869,0.00856349,4.68644,-0.219841,-13.3948,0.1744,-0.135423,	-1.86717,7.12173,-0.145154,-5.56873,-0.108048,0.831623,3.07379,-0.178763,-10.2247,-0.0128999,-0.24963};
float BW2[]= {0.245772,0.0403525,-1.00947,-0.891614,-0.260954,-0.0671591,-0.118392,-0.0357653,-1.58065,-0.314836,-0.654904,	0.0503466,0.592063,-0.00178819,-0.629867,-0.851089,-1.1726,0.979834,-0.839649,1.75353,-0.573732,-0.457148,	0.542396,0.760147,0.168547,0.859029,-0.600638,0.0801297,0.753935,-0.396098,1.17579,0.533974,1.14622,0.17879,-0.813973,-1.19739,0.750112,-0.405469,0.67901,0.593883,0.0430763,-0.196537,-0.689156,0.792834};

/*
 * These arrays are for the neurons.
 * Layer1 its the hidden layer
 * Layer12 its the same as 1 but with the extra 1 of the Bias
 * and Layer 2 it's the output layer.
 *  */
float Layer1[Ne];
float Layer12[Ne+1];
float Layer2[G];

/*Gesture identification number will be used to inform the application which gesture is.*/
int ID;


/*
 * @brief   Application entry point.
 */
int main(void) {

  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
  	/* Init FSL debug console. */
    BOARD_InitDebugConsole();

    /*ADC1 data arrays*/
	float samples[N]={0.0f};				//Array of last measures.
	float *measures = &samples[0];			//Pointer to the samples array.
	float abs_samples[N]={0.0f};			//Absolute value of each of the last 12 measures.
	float *abs_measures = &abs_samples[0];	//Pointer to the absolute value array.

    float samples_minus_one[N]={0.0f};		//Array of last measures, shifted one.
    float *SMO=&samples_minus_one[0];
    float samples_plus_one[N]={0.0f};		//Array of last measures, shifted one.
    float *SPO=&samples_plus_one[0];

    /*ADC2 data arrays*/
	float samples2[N]={0.0f};					//Array of last measures.
	float *measures2 = &samples2[0];			//Pointer to the samples array.
	float abs_samples2[N]={0.0f};				//Absolute value of each of the last 12 measures.
	float *abs_measures2 = &abs_samples2[0];	//Pointer to the absolute value array.

    float samples_minus_one2[N]={0.0f};		//Array of last measures, shifted one.
    float *SMO2=&samples_minus_one2[0];
    float samples_plus_one2[N]={0.0f};		//Array of last measures, shifted one.
    float *SPO2=&samples_plus_one2[0];


    /*
     * Used to keep track of how many measures are left to start the features extraction
     * */
    int ADC_Cycles=(N-1);

    /*
     * Structures to save results (all 5 features)
     * "Feat" structure its defined on Features_Calculator.C
     * If more channels are measured, more structures need to be initialized.
     * */
    //Channel 1
    Feat results;
    //Channel 2
    Feat results2;


    /*
     * NN Matrices
     * */
    //Matrix with the weights for Layer 1
    arm_matrix_instance_f32 NN1 = {Ne, (Ne+1), &BW1[0]};

    //Vector with the inputs (features)
    arm_matrix_instance_f32 X = {(Ne+1),1, &input[0]};

    //Matrix with the weights for Layer 2
    arm_matrix_instance_f32 NN2 = {G, (Ne+1), &BW2[0]};

    //Hidden layer vector
    arm_matrix_instance_f32 L1 = {Ne,1,&Layer1[0]};

    //Hidden layer vector with bias
    arm_matrix_instance_f32 L12 = {(Ne+1),1,&Layer12[0]};

    //Output layer vector
    arm_matrix_instance_f32 L2 = {G,1,&Layer2[0]};


    /*
     * Enter the main loop. May the force be with you...
     * */

     while(1)
     {
    	 /*Get N measures from ADC*/
    	 while(ADC_Cycles>=0){

        	 ADC_Flag1 = false;
        	 ADC_Flag2 = false;

        	 ADC16_SetChannelConfig(ADC_1_PERIPHERAL, 0U, &ADC_1_channelsConfig[0]);

        	 ADC16_SetChannelConfig(ADC_2_PERIPHERAL, 0U, &ADC_2_channelsConfig[0]);

        	/*Wait for the measures to finish*/
        	 while(!(ADC_Flag1&ADC_Flag2))
        	 {
        	 }

        	 /*Transform the measure to volts*/
        	 results.VOLT=valor1*((Vref/bits));
        	 results2.VOLT=valor2*((Vref/bits));

        	 /*Save the measure on the window array*/
        	 samples[ADC_Cycles]=results.VOLT;
        	 samples2[ADC_Cycles]=results2.VOLT;

        	 /*
        	  * Filling the auxiliary arrays
        	  * these have the same values but shifted positions
        	  * */
        	 if (ADC_Cycles>0){
        		 samples_minus_one[ADC_Cycles-1]=results.VOLT;
        		 samples_minus_one2[ADC_Cycles-1]=results2.VOLT;
        	 }
        	 if (ADC_Cycles<(N-1)){
        		 samples_plus_one[ADC_Cycles+1]=results.VOLT;
        		 samples_plus_one2[ADC_Cycles+1]=results2.VOLT;
        	 }

        	 ADC_Cycles=ADC_Cycles-1;
    	 }

    	 //Reset the ADC cycle.
    	 ADC_Cycles=N-1;

    	 //The auxiliary arrays have the last or first position on 0
    	 samples_minus_one[N-1]=0.0f;
    	 samples_plus_one[0]=0.0f;
    	 samples_minus_one2[N-1]=0.0f;
    	 samples_plus_one2[0]=0.0f;





   	         /*
   	          * Call the Features function
   	          * It extracts the features
   	          * and returns a "result" structure filled
   	          * */

   	         Features(measures, abs_measures, SMO, SPO, &results);
   	         Features(measures2, abs_measures2, SMO2, SPO2, &results2);

   	    	 /**
   	  	     * Slope Sign Changes:
   	  	     * This feature is done here instead of Features_Calculator.c like the rest of the
   	   	     * features because it's harder to access each member of the samples
   	   	     * array using the pointers, so yeah because i'm too lazy to do it the smart way...
   	   	     *
   	   	     * Counts the times the slope changes sign changes passing a threshold
   	   	     * Because the results are counters they have to be reset.
   	   	     * */

   	    	 results.SSC=0;
   	    	 results2.SSC=0;

   	    	 for(int n=0;n<N;n++)
   	    	 {
   	    		 //Channel 1
   	    		 if (((samples[n]-samples_minus_one[n])*(samples[n]-samples_plus_one[n]))>0.00001f) (results.SSC)++;
   	    		 //Channel 2
   	    		 if (((samples2[n]-samples_minus_one2[n])*(samples2[n]-samples_plus_one2[n]))>0.00001f) (results2.SSC)++;
   	    	 }

   	         /*
   	          * Pass all the features to the corresponding array.
   	          * The "X" vector it's linked to "input[]" memory position
   	          * so this is filling the input vector.
   	          * input[0] is the first Bias.
   	          * If channels are added the rest of the features have to be saved.
   	          * */
   	     	 input[0] = 1.0f; 			//Bias
   	     	 input[1] = results.MEAN;
   	     	 input[2] = results.VAR;
   	     	 input[3] = results.WL;
   	     	 input[4] = results.SSC;
   	     	 input[5] = results.WAMP;
   	     	 input[6] = results2.MEAN;
   	     	 input[7] = results2.VAR;
   	     	 input[8] = results2.WL;
   	     	 input[9] = results2.SSC;
   	     	 input[10] = results2.WAMP;

   	     	 /*
   	     	  * (1st Weight matrix)*(Input matrix) = first layer
   	     	  * then pass the result to the "Tansig" transfer function and get
   	     	  * the Hidden Layer.
   	     	  * Tansig it's defined on Functions.c
   	     	  * */
   	     	 arm_mat_mult_f32 (&NN1,&X,&L1);
    	     tansig (Layer1, (sizeof(Layer1)/sizeof(Layer1[0])));

    	     /*
    	      * Add the second Bias.
    	      * Basically copy the vector with the extra 1.
    	      * */
    	     for(int i=0;i<(sizeof(Layer1)/sizeof(Layer1[0]));i++)
    	     {
    	    	 Layer12[i+1]=Layer1[i];
    	     }
   	     	 Layer12[0]=1;


   	     	 /*
   	     	  * (2nd Weight matrix)*(Hidden layer matrix) = second layer
   	     	  * then pass the result to the "SoftMax" transfer function and get
   	     	  * the Output Layer.
   	     	  * SoftMax it's defined on Functions.c
   	     	  * */
   	     	 arm_mat_mult_f32 (&NN2,&L12,&L2);
   	     	 softmax(Layer2, (sizeof(Layer2)/sizeof(Layer2[0]))); //Functions.C

   	     	 /*
   	     	  * We search which gesture was made based on which has the highest probability.
   	     	  * The rule is that the probability must be greater than the rest of probabilities combined.
   	     	  * "Gesture()" is defined on Functions.c
   	     	  * Each gesture is represented by a number, ex:0,1,2,3 for 4 gestures.
   	     	  * */
   	     	 ID = Gesture(&Layer2[0]);


   	     	 /*
   	     	  * Send all 10 features, used for training
   	     	  * */
   	     	//PRINTF("%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\n",results.MEAN, results.VAR, results.WL, results.SSC, results.WAMP, results2.MEAN, results2.VAR, results2.WL, results2.SSC, results2.WAMP);

   	     	 /*
   	     	  * Send features of 1st signal channel. Just for tests.
   	     	  * */
   	     	 //PRINTF("%f, %f, %f, %f, %f\r\n", results.MEAN, results.VAR, results.WL, results.SSC, results.WAMP);

   	     	 /*
   	     	  * Send features of 2nd signal channel. Just for tests.
   	     	  * */
   	     	 //PRINTF("%f, %f, %f, %f, %f\n", results2.MEAN, results2.VAR, results2.WL, results2.SSC, results2.WAMP);

   	     	 /*
   	     	  * Send NN output. These are the probabilities.
   	     	  * */
   	     	 //PRINTF("%f, %f, %f, %f\n",Layer2[0],Layer2[1],Layer2[2],Layer2[3]);


   	     	 /*
   	     	  * Send the gesture ID. IT'S BEEN 84 YEARS!!!!
   	     	  * */
   	     	 PRINTF("%d",(ID));

     }

    return 0 ;
}
