#include "YMZ294.h"

char harmonics[] = {-24, -24, -12, -12, -5, 0, 0, 5, 7, 7, 12, 12, 19, 24, 31, 36};

/**
 * FEATURES TO BUILD:
 * 
 * - support all the 3 osc of a YMZ294 chip
 * - add noise channel support / percussion events
 * - LFO pitch modulation
 * - groove feature (modify the time array in real-time)
 * - have a editor on PC to create sequences
 * - support for multiple chip
 * - having several sequences and be able to switch from one to another
 * - external EEPROM memory to store sequence data and edit them separately
 * - display and input part (need design)
 *
 *----------------------------------------------------------------------------------------------
 *
 * YM-event protocol
 *
 *
 *                                                    
 *                ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
 * Time           │ 15 │ 14 │ 13 │ 12 │ 11 │ 10 │ 09 │ 08 │ 07 │ 06 │ 05 │ 04 │ 03 │ 02 │ 01 │ 00 │
 *                ├────┴────┴────┴────┴────┴────┴────┴────┼────┴────┴────┴────┴────┴────┴────┴────┤ 
 *                │                  bar                  │   beat  │quater-nt│  1/16 division    │
 *                └───────────────────────────────────────┴───────────────────────────────────────┘
 *
 *                ┌────┬────┬────┬────┬────┬────┬────┬────┐
 * Chan           │ D7 │ D6 │ D5 │ D4 │ D3 │ D2 │ D1 │ D0 │
 *                ├────┴────┴────┼────┴────┼────┴────┴────┤ 
 *                │ chip number  │ channel │  event type  │
 *                │              │ osc 1-3 │              │
 *                │   (0 - 7)    │ noise:0 │ (see  below) │
 *                └──────────────┴─────────┴──────────────┘
 *
 *
 * Data
 *                ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
 *     type    bit│ 15 │ 14 │ 13 │ 12 │ 11 │ 10 │ 09 │ 08 │ 07 │ 06 │ 05 │ 04 │ 03 │ 02 │ 01 │ 00 │
 * ┌───┬──────────┼────┴────┴────┴────┴────┴────┴────┴────┼────┼────┴────┴────┴────┴────┴────┴────┤ 
 * │000│ note     │              bend duration            │ /G │    note number (0 = note off)    │ /G: retrigger gate
 * ├───┼──────────┼───────────────────┬───────────────────┼────┴──────────────┬───────────────────┤ 
 * │001│ arpege   │       speed       │    transpose 3    │    transpose 2    │    transpose 1    │
 * ├───┼──────────┼───────────────────┴───────────────────┼───────────────────┴───────────────────┤ 
 * │010│ tone mod │  lfo speed, mod amount, attack speed  │                                       │
 * ├───┼──────────┼───────────────────────────────────────┼───────────────────────────────────────┤ 
 * │011│ ░░░░░░░░ │                                       │                                       │
 * ├───┼──────────┼───────────────────────────────────────┼───────────────────────────────────────┤ 
 * │100│ ░░░░░░░░ │                                       │                                       │
 * ├───┼──────────┼───────────────────────────────────────┼───────────────────────────────────────┤ 
 * │101│ ░░░░░░░░ │                                       │                                       │
 * ├───┼──────────┼───────────────────────────────────────┼───────────────────────────────────────┤ 
 * │110│ ░░░░░░░░ │                                       │                                       │
 * ├───┼──────────┼───────────────────────────────────────┼───────────────────────────────────────┤ 
 * │111│ patch    │                                       │                                       │
 * └───┴──────────┴───────────────────────────────────────┴───────────────────────────────────────┘
 *
 *
 *
 */
byte eventChan[] = {8,      8,      8,      8,      8,      8,      8,      8}; // event channel
word eventTime[] = {0x0000, 0x0030, 0x0040, 0x0050, 0x0060, 0x0080, 0x00B0, 0x00D0};
word eventData[] = {0x0032, 0x0037, 0x002E, 0x0025, 0x18A2, 0x2025, 0x0030, 0x003E};
// TODO: arpeg should be an normal event, mixed in eventData 
word arpeg[]     = {0x6973, 0x3073, 0x5095, 0x0000, 0x0000, 0x0000, 0x4123, 0x4321};



// word eventTime[] = {0x0000, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x0080, 0x0090, 0x00a0, 0x00b0, 0x00c0, 0x00d0, 0x00e0, 0x00f0, 0xffff};
int eventSize = 8 - 1;
int head = 0;


volatile word updateReg1Flags = 0;
byte updateReg1Data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile word seqClock = 0;

volatile bool emptyQueue = false;
//word loopPoint = 1024; // 1 bar * 16 beats * 64 divisions
word loopPoint = 0x0100;    // 1 bar * 16 beat * 16 division;

//█████████████████████████████████████████████████
//███████████████████▀█████████████████████████████
//██▀▄▄▄▄ ██▀▄▄▄▄▀██▄ ▄▄▄███▄ ██▄ ██▄ ▀▄▄▀█████████
//███▄▄▄▄▀██ ▄▄▄▄▄███ ███████ ███ ███ ███ █████████
//██ ▀▀▀▀▄██▄▀▀▀▀▀███▄▀▀▀▄███▄▀▀▄ ▀██ ▀▀▀▄█████████
//██████████████████████████████████▀ ▀████████████

int toneValA = 0;
int noteBaseA = 0;     // base note of osc A

//----------- bend module ----------------
// int bendCountA = 0; // bend counter
int bendDuratA = 0; // bend duration
int bendIncrmA = 0; // bend increment
int bendValueA = 0; // bend current value

//--------- arpegio module ---------------
int arpegioCountA = 0; // portamento counter
int arpegioStateA = 0; // portamento state
int arpegioSpeedA = 0; // portamento speed
int arpegioValueA = 0; // portamento value
int arpegioChordA = 0; // portamento chord

int count = 0;
int portamento = 0;

void setup(){
	count = 0;
	pinMode(8, OUTPUT);

	toneValA = 0;
	noteBaseA = 0;

	// bendCountA = 0;
	bendDuratA = 0;
	bendIncrmA = 0;
	bendValueA = 0;
	arpegioCountA = 0;
	arpegioStateA = 0;
	arpegioSpeedA = 0;
	arpegioValueA = 0;
	arpegioChordA = 0;

	// init variables
	head = 0;
	seqClock = 0;
	emptyQueue = false;
	updateReg1Flags = 0;

	// Setup the pin modes for the YMZ294
	setupYMZ294();

	/*
	// analogRead prescallers
	const byte A_PS_16  = bit(ADPS2);
	const byte A_PS_32  = bit(ADPS2) | bit(ADPS0);
	const byte A_PS_64  = bit(ADPS2) | bit(ADPS1);
	const byte A_PS_128 = bit(ADPS2) | bit(ADPS1) | bit(ADPS0);

	// setup fast analogRead
	ADCSRA &= ~A_PS_128; // remove bit set by arduino
	ADCSRA |=  A_PS_16;  // set custom prescaller
	*/

	// setup interrupt
	const byte T1_PS_1    = bit(CS10);
	const byte T1_PS_8    = bit(CS11);
	const byte T1_PS_64   = bit(CS11) | bit(CS10);
	const byte T1_PS_256  = bit(CS12);
	const byte T1_PS_1024 = bit(CS12) | bit(CS10);

	noInterrupts();
	TCNT1  = 0;                    // reinitialize counter register
	TCCR1A = 0;                    // normal operation
	TCCR1B = bit(WGM12) | T1_PS_8; // Clear Timer on Compare, prescaller to 8
	OCR1A  = 14423;                // compare A register value, roughly 130BPM
	TIMSK1 = bit(OCIE1A);          // interrupt on Compare A Match
	interrupts();


    /* ------------
	* OCR1A values for prescaler = 8
	* ┌─────┬──────────┬──────────┬──────────┬──────────┐
	* │ BPM │ 1/16     │ 1/32     │ 1/64     │ 1/256    │ 
	* ├─────┼──────────┼──────────┼──────────┼──────────┤
	* │ 30  │ 62500.00 │ 31250.00 │ 15625.00 │  3906.25 │ 
	* │ 40  │ 46875.00 │ 23437.50 │ 11718.75 │  2929.69 │ 
	* │ 50  │ 37500.00 │ 18750.00 │  9375.00 │  2343.75 │ 
	* │ 60  │ 31250.00 │ 15625.00 │  7812.50 │  1953.13 │ 
	* │ 70  │ 26785.71 │ 13392.86 │  6696.43 │  1674.11 │ 
	* │ 80  │ 23437.50 │ 11718.75 │  5859.38 │  1464.84 │ 
	* │ 90  │ 20833.33 │ 10416.67 │  5208.33 │  1302.08 │ 
	* │ 100 │ 18750.00 │  9375.00 │  4687.50 │  1171.88 │ 
	* │ 110 │ 17045.45 │  8522.73 │  4261.36 │  1065.34 │ 
	* │ 120 │ 15625.00 │  7812.50 │  3906.25 │   976.56 │ 
	* │ 130 │ 14423.08 │  7211.54 │  3605.77 │   901.44 │ 
	* │ 140 │ 13392.86 │  6696.43 │  3348.21 │   837.05 │ 
	* │ 150 │ 12500.00 │  6250.00 │  3125.00 │   781.25 │ 
	* │ 160 │ 11718.75 │  5859.38 │  2929.69 │   732.42 │ 
	* │ 170 │ 11029.41 │  5514.71 │  2757.35 │   689.34 │ 
	* │ 180 │ 10416.67 │  5208.33 │  2604.17 │   651.04 │ 
	* │ 190 │  9868.42 │  4934.21 │  2467.11 │   616.78 │ 
	* │ 200 │  9375.00 │  4687.50 │  2343.75 │   585.94 │ 
	* │ 210 │  8928.57 │  4464.29 │  2232.14 │   558.04 │ 
	* │ 220 │  8522.73 │  4261.36 │  2130.68 │   532.67 │ 
	* │ 230 │  8152.17 │  4076.09 │  2038.04 │   509.51 │ 
	* │ 240 │  7812.50 │  3906.25 │  1953.13 │   488.28 │ 
	* │ 250 │  7500.00 │  3750.00 │  1875.00 │   468.75 │ 
	* │ 260 │  7211.54 │  3605.77 │  1802.88 │   450.72 │ 
	* │ 270 │  6944.44 │  3472.22 │  1736.11 │   434.03 │ 
	* │ 280 │  6696.43 │  3348.21 │  1674.11 │   418.53 │ 
	* │ 290 │  6465.52 │  3232.76 │  1616.38 │   404.09 │ 
	* │ 300 │  6250.00 │  3125.00 │  1562.50 │   390.63 │ 
	* └─────┴──────────┴──────────┴──────────┴──────────┘
	*/

	// init YMZ294 registers
	mixer(7);
	toneA(0);
	toneB(0);
	toneC(0);
	envTime(20000);
	volumeA(0x10);
	volumeB(0x00);
	volumeC(0x10);
}


//████████████████████████████████████████████████████████████████████████████████████████████████
//████████████████████████████████████▄███████████▀█████████████████████████████████████████▀█████
//██▀▄▄▄▄ █▀▄▄▄▄▀█▀▄▄▄▀ ▄███████████▄▄ ███▄ ▀▄▄▀█▄ ▄▄▄██▀▄▄▄▄▀█▄ ▀▄▄▄█▄ ▀▄▄▄█▄ ██▄ █▄ ▀▄▄▀█▄ ▄▄▄██
//███▄▄▄▄▀█ ▄▄▄▄▄█ ████ ██████████████ ████ ███ ██ █████ ▄▄▄▄▄██ ██████ ██████ ███ ██ ███ ██ █████
//██ ▀▀▀▀▄█▄▀▀▀▀▀█▄▀▀▀▄ ████  ██████▀▀ ▀▀█▀ ▀█▀ ▀█▄▀▀▀▄█▄▀▀▀▀▀█▀ ▀▀▀██▀ ▀▀▀███▄▀▀▄ ▀█ ▀▀▀▄██▄▀▀▀▄█
//████████████████████▀ ▀███████████████████████████████████████████████████████████▀ ▀███████████
/**
 * For one YMZ294 chip a maximum of 14 registers have to be refreshed
 * Writing one register of YMZ294 takes around 340 clock cycle.
 * So, if we need to refresh all registers, this will takes:
 * 4760 clk + for loop + if condition + ...
 * ┌────┬───────────────────────────┐
 * │ 01 │ osc A tune LSB            │
 * │ 02 │ osc A tune MSB            │
 * │ 03 │ osc B tune LSB            │
 * │ 04 │ osc B tune MSB            │
 * │ 05 │ osc C tune LSB            │
 * │ 06 │ osc C tune MSB            │
 * │ 07 │ noise tune                │
 * │ 08 │ mixer                     │
 * │ 09 │ volume A                  │
 * │ 10 │ volume B                  │
 * │ 11 │ volume C                  │
 * │ 12 │ enveloppe time            │
 * │ 13 │ enveloppe time            │
 * │ 14 │ enveloppe shape / restart │
 * └────┴───────────────────────────┘
 */
ISR(TIMER1_COMPA_vect) {
	// increment sequencer position
	seqClock++;
	if (seqClock == loopPoint) {
		seqClock = 0;
	}

	emptyQueue = true;

	if (updateReg1Flags == 0) {
		return;
	}

	// update register for YMZ294 chip 1
	for (int i = 0; i < 14; i++) {
		if ((updateReg1Flags >> i) & 1) {
			writeData(i, updateReg1Data[i]);
		}
	}

	updateReg1Flags = 0;
}


//█████████████████████████████████████████
//████▄ ███████████████████████████████████
//█████ ████▀▄▄▄▄▀██▀▄▄▄▄▀██▄ ▀▄▄▀█████████
//█████ ████ ████ ██ ████ ███ ███ █████████
//███▀▀ ▀▀██▄▀▀▀▀▄██▄▀▀▀▀▄███ ▀▀▀▄█████████
//██████████████████████████▀ ▀████████████


void loop(){

	if (emptyQueue) {
		// enqueue next commands
		emptyQueue = false;
		// test if there is an event for this time

		// TODO: change this if to a while (several events can be set at the same time)
		if (eventTime[head] == seqClock) {
			int nextCommand = eventData[head];

			// save previous tone frequency
			int tonePrev = toneValA;

			// get note number
			noteBaseA = nextCommand & 0x7F;
			// TODO: if noteNumber == 0 then it's a note off message

			// compute the new tone from note number
			toneValA = midiToPitch[noteBaseA];
			// get the bend duration
			bendDuratA = (nextCommand & 0xFF00) >> 8;
			if (bendDuratA > 0) {
				bendIncrmA = (tonePrev - toneValA) / bendDuratA;
				bendValueA = bendIncrmA * bendDuratA;
				toneValA += bendValueA;
			} else {
				bendIncrmA = 0;
				bendValueA = 0;
			}


			updateReg1Data[0] = toneValA & 0xFF;        // fine tune
			updateReg1Data[1] = (toneValA >> 8) & 0x0F; // main tune
			updateReg1Flags |= 0x0003;

			if (!((nextCommand & 0x80) >> 7)) {
				// retrigger enveloppe
				updateReg1Data[13] = 9;
				updateReg1Flags |= 0x2000;
			}

			// get arpegiator values
			arpegioCountA = 0;
			arpegioStateA = 0;
			arpegioValueA  = 0;
			arpegioChordA = arpeg[head];
			arpegioSpeedA = (arpegioChordA >> 12) & 0xF;

			// increment head
			if (head >= eventSize) {
				head = 0;
			} else {
				head++;
			}
			
		} else {
			// need tone change
			bool toneChange = false;

			// portamento playing ?
			if (arpegioSpeedA > 0) {
				arpegioCountA++;
				if (arpegioCountA == arpegioSpeedA) {
					arpegioCountA = 0;
					arpegioStateA++;
					if (arpegioStateA == 4) {
						arpegioStateA = -1;
						arpegioValueA = 0;
					} else {
						arpegioValueA = (arpegioChordA >> (arpegioStateA * 4)) & 0x0F;
						if (arpegioValueA == 0) {
							arpegioStateA = -1;
							arpegioValueA = 0;
						}
					}
					toneValA = midiToPitch[noteBaseA + arpegioValueA];
					toneChange = true;
				}
			} else {
				toneValA = midiToPitch[noteBaseA];
			}

			// TODO: bend engaged ?
			if (bendDuratA > 0) {
				bendDuratA -= 1;
				bendValueA -= bendIncrmA;
				toneValA += bendValueA;
				toneChange = true;
			}

			if (toneChange) {
				updateReg1Data[0]  = toneValA & 0xFF;        // fine tune
				updateReg1Data[1]  = (toneValA >> 8) & 0x0F; // main tune
				updateReg1Flags |= 0x0003;
			}

				
		}
	} else {
		//------------------------------------------
		// ANALOG INPUTS

		// TODO: read one analog or digital in each loop

		portamento = analogRead(A0);
		// portamento = analogRead(A1);
		// portamento = analogRead(A2);
		// portamento = analogRead(A3);


		//------------------------------------------
		// DIGITAL INPUTS

		// TODO


		//------------------------------------------
		// TEMPO DISPLAY
		byte beat = seqClock & 0xFF;
		if (beat == 0x00 || beat  == 0x40 || beat  == 0x80 || beat  == 0xC0) {
			count = 0;
			digitalWrite(8, HIGH);
		}

		count++;
		if (count > 20) {
			digitalWrite(8, LOW); 
		}

	}
}

