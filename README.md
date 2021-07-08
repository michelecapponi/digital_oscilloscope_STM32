# digital_oscilloscope_STM32
The goal of this project was to design a simple digital storage oscilloscope (DSO). It is a measurement instrument, able to acquire analog signals from different input channels, digitize them, and show the resulting waveforms on the screen of a standard PC. 
The project is made up of two-part:
Acquisition system. This part of the system has been realized on an MCU-based prototype board, programmed in C, and based on an STM32 microcontroller. It digitizes input signals, synchronizes
samples to trigger conditions, and send acquired data to a PC, through a standard RS232 full-duplex serial line.
Graphical user interface. This subsystem is built with a Python program, running on a standard PC. It's able to configure the acquisition board and display acquired data on the PC screen.

