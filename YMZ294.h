#include "Arduino.h"


// █████████████████████████████████████████████████████████████████████████████████████████
// ███████████████████▄ ▄█▄ ▄█▄  █  ▄██ ▄▄▄ ██████▀▄▄▄▀███▀▄▄▄▄▀████▀▄ █████████████████████
// █████████████████████ █ ████ ▄▀▄ █████▀▄██████████▀▄███ ████ ███ ██ █████████████████████
// ██████████████████████ █████ █▄█ ████▀▄█████████▀▄██████▄▄▄▄ ██▄▄▄▄ ▄████████████████████
// █████████████████████▀ ▀███▀ ▀█▀ ▀██ ▀▀▀ ██████ ▀▀▀ ███▀▀▀▀▄██████▀ ▀████████████████████
// █████████████████████████████████████████████████████████████████████████████████████████
 

#ifndef _YMZ294_DRIVER_H
#define _YMZ294_DRIVER_H


//██████████████████████████████████████████████████████
//███████████▄███████████████████▄ ████████████▀▀▀██████
//██▄ ▀▄▄▀█▄▄ ███▄ ▀▄▄▀██████▀▄▄▄▀ ██▀▄▄▄▄▀█▀▀ ▀▀▀██████
//███ ███ ███ ████ ███ ██████ ████ ██ ▄▄▄▄▄███ █████████
//███ ▀▀▀▄█▀▀ ▀▀█▀ ▀█▀ ▀█████▄▀▀▀▄ ▀█▄▀▀▀▀▀█▀▀ ▀▀▀█  ███
//██▀ ▀█████████████████████████████████████████████████
//██████████████████████████████████████████████████████
#define D0_PIN 0
#define YMZ294_WR 11
#define YMZ294_A0 12


//██████████████████████████████████████
//████████████▄███████▄ ████▄███████████
//█▄ ▀▄▀▀▄▀█▄▄ ███▀▄▄▄▀ ██▄▄ ███████████
//██ ██ ██ ███ ███ ████ ████ ███████████
//█▀ ▀█ ▀█ █▀▀ ▀▀█▄▀▀▀▄ ▀█▀▀ ▀▀█████████
//██████████████████████████████████████

/**
 * note 0 is note off.
 * note 11 is the lower note YM294 can produce
 * tone 8 is the audible limit
 */
int midiToPitch[] = {
	0, 3608, 3405, 3214, 3034, 2863, 2703, 2551, 2408, 2273, 4095, 4050, 3822, 
	3608, 3405, 3214, 3034, 2863, 2703, 2551, 2408, 2273, 2145, 2025, 1911, 1804, 
	1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012, 956, 902, 851, 
	804, 758, 716, 676, 638, 602, 568, 536, 506, 478, 451, 426, 402, 379, 358, 
	338, 319, 301, 284, 268, 253, 239, 225, 213, 201, 190, 179, 169, 159, 150, 
	142, 134, 127, 119, 113, 106, 100, 95, 89, 84, 80, 75, 71, 67, 63, 60, 56, 
	53, 50, 47, 45, 42, 40, 38, 36, 34, 32, 30, 28, 27, 25, 24, 22, 21, 20, 19, 
	18, 17, 16, 15, 14, 13, 13, 12, 11, 11, 10, 9, 9, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5
};


//█████████████████████████████████████████████████████████████████████████████████████████
//█████████████████████▄█████▀██████████████████████████▄ ███████████▀█████████████████████
//██▄ ▄█▄ ▄█▄ ▀▄▄▄███▄▄ ████▄ ▄▄▄███▀▄▄▄▄▀██████████▀▄▄▄▀ ██▀▄▄▄▄▀██▄ ▄▄▄███▀▄▄▄▄▀█████████
//███ █ █ ███ █████████ █████ ██████ ▄▄▄▄▄██████████ ████ ██▀▄▄▄▄ ███ ██████▀▄▄▄▄ █████████
//███▄▀▄▀▄██▀ ▀▀▀████▀▀ ▀▀███▄▀▀▀▄██▄▀▀▀▀▀██████████▄▀▀▀▄ ▀█▄▀▀▀▄ ▀██▄▀▀▀▄██▄▀▀▀▄ ▀████████
//█████████████████████████████████████████████████████████████████████████████████████████

/**
 * Write data through PORTD.
 *
 * instruction   clk  count  total clk
 * PORTD         2    2      4
 * digitalWrite  55   6      330
 * ------------------------- 334
 */
void writeData(unsigned char address, byte data) {
	// write address
	digitalWrite(YMZ294_A0, LOW);
	PORTD = address;
	digitalWrite(YMZ294_WR, LOW);
	digitalWrite(YMZ294_WR, HIGH);
	
	// write data
	digitalWrite(YMZ294_A0, HIGH);
	PORTD = data;
	digitalWrite(YMZ294_WR, LOW);
	digitalWrite(YMZ294_WR, HIGH);
}


//█████████████████████████████████████████████████████████████████████████
//███████████████████████████▀████████████████████████▄░███████████████████
//██▀▄▄▄▀░██▀▄▄▄▄▀██▄░▀▄▄▀██▄░▄▄▄███▄░▀▄▄▄██▀▄▄▄▄▀█████░████▀▄▄▄▄░█████████
//██░███████░████░███░███░███░███████░██████░████░█████░█████▄▄▄▄▀█████████
//██▄▀▀▀▀▄██▄▀▀▀▀▄██▀░▀█▀░▀██▄▀▀▀▄██▀░▀▀▀███▄▀▀▀▀▄███▀▀░▀▀██░▀▀▀▀▄█████████
//█████████████████████████████████████████████████████████████████████████

//-------------------------------------------------------------------------
// OSC PITCH & NOISE

void toneA(unsigned int i) { writeData(0, i & 0xff); writeData(1, (i >> 8) & 0x0f); }
void toneB(unsigned int i) { writeData(2, i & 0xff); writeData(3, (i >> 8) & 0x0f); }
void toneC(unsigned int i) { writeData(4, i & 0xff); writeData(5, (i >> 8) & 0x0f); }
void noise(byte i) { writeData(6, i & 0x1f); } 

//-------------------------------------------------------------------------
// MIXER

byte YMZ294mixerState = 0;

void mixer(byte i)   { YMZ294mixerState = (~i) & 0x3f; writeData(7, YMZ294mixerState); }
void enableOscA()    { YMZ294mixerState &= 0x3e;  writeData(7, YMZ294mixerState); }
void enableOscB()    { YMZ294mixerState &= 0x3d;  writeData(7, YMZ294mixerState); }
void enableOscC()    { YMZ294mixerState &= 0x3b;  writeData(7, YMZ294mixerState); }
void enableNoiseA()  { YMZ294mixerState &= 0x37;  writeData(7, YMZ294mixerState); }
void enableNoiseB()  { YMZ294mixerState &= 0x2f;  writeData(7, YMZ294mixerState); }
void enableNoiseC()  { YMZ294mixerState &= 0x1f;  writeData(7, YMZ294mixerState); }
void disableOscA()   { YMZ294mixerState |= 0x01;  writeData(7, YMZ294mixerState); }
void disableOscB()   { YMZ294mixerState |= 0x02;  writeData(7, YMZ294mixerState); }
void disableOscC()   { YMZ294mixerState |= 0x04;  writeData(7, YMZ294mixerState); }
void disableNoiseA() { YMZ294mixerState |= 0x08;  writeData(7, YMZ294mixerState); }
void disableNoiseB() { YMZ294mixerState |= 0x10;  writeData(7, YMZ294mixerState); }
void disableNoiseC() { YMZ294mixerState |= 0x20;  writeData(7, YMZ294mixerState); }
void muteAll()       { YMZ294mixerState = 0;      writeData(7, YMZ294mixerState); }

//-------------------------------------------------------------------------
// VOLUME

void volumeA(byte i) { writeData(8,  i & 0x1f); }
void volumeB(byte i) { writeData(9,  i & 0x1f); }
void volumeC(byte i) { writeData(10, i & 0x1f); }
void volumeAll(byte i) {
	byte vol = i & 0x1f;
	writeData(8, vol);
	writeData(8, vol);
	writeData(8, vol);
}

//-------------------------------------------------------------------------
// ENVELOPPE

void envTime(unsigned int i) { writeData(11, i & 0xff); writeData(12, (i >> 8) & 0xff); }
void envShape(byte i) { writeData(13, i & 0x0f); }


//███████████████████▀█████████████████████████████
//██▀▄▄▄▄░██▀▄▄▄▄▀██▄░▄▄▄███▄░██▄░██▄░▀▄▄▀█████████
//███▄▄▄▄▀██░▄▄▄▄▄███░███████░███░███░███░█████████
//██░▀▀▀▀▄██▄▀▀▀▀▀███▄▀▀▀▄███▄▀▀▄░▀██░▀▀▀▄█████████
//██████████████████████████████████▀░▀████████████
//█████████████████████████████████████████████████
// makes the setup for the Yamaha YMZ294 chip
void setupYMZ294() {
	pinMode(YMZ294_WR, OUTPUT);
	pinMode(YMZ294_A0, OUTPUT);

	// init pins
	for(int i = 0; i < 8; i++){
		pinMode(i + D0_PIN, OUTPUT);
	}
	
	mixer(7); // init mixer (enable all osc, disable noise)
	volumeAll(0); // set all volume to 0
}


#endif _YMZ294_DRIVER_H
