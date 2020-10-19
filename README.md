# Digital control device using electromyographic signals (EMG)
> This is part of the code used to develop my grade Thesis.

## Table of contents
* [General info](#general-info)
* [Screenshots](#screenshots)
* [Setup](#setup)
* [Status](#status)
* [Contact](#contact)

## General info
For my grade thesis as a electronic Engineer on *"Universidad Simon Bolivar"*, i developed a device that read the EMG signals on the forearm of a person to interpret specific gestures done by the user with his hands. As replicating the device requires the electronic circuit, the microcontroller, a pretrained neural network and the Unity simulation program, all of this developed for the project, this repository alone won't be enough to replicate the whole project as it's just the code implemented on the microcontroller, but it gives the general idea of the signal processing, analysis and the real-time execution of the NN on a DSP microcontroller.
On the "Documents" folder you can find the Thesis paper and the PowerPoint presentation used which describe the whole process of development and results, but both documents are on Spanish.

## Screenshots
![Example screenshot](./Documents/demo.gif)

## Setup
As i stated this files wont be enough but the microcontroller file structure is as follows:

    *Features_Calculator.c - Contains the functions used for signal processing, extraction of signal features to feed the Neural Network.

    *Functions.c - Contains the support functions used for the Neural Network recreation inside the microcontroller and gesture classification.

    *Hand_Analisis.c - Main file. Signal acquisition, neural network execution and serial communication to the receiving program.

## Status
Project is: _finished_

## Contact
Created by [@KennyJaimes](https://www.linkedin.com/in/kennyjaimes/) - feel free to contact me!