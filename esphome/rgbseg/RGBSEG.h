//

class RGBSEG : public Component {

	bool RANDOMCOLORS = true;

	int PIN_BLANK = 27;
	int PIN_LATCH = 5;
	int PIN_D0 = 16;
	int PIN_D1 = 17;

	//

	void setup() override { INIT(); }
	void loop() override { LOOPFX(); }

	//

	SPISettings *SPISET = nullptr;
	HighFrequencyLoopRequester ESP_FASTLOOP;

	esphome::time::ESPTime DTNOW; time_t DT; time_t DTNEXT=0; char DTSTR[9]; 

	uint8_t SEGBITS[16];
	int CLOCK_HOLD = 500;
	int CLOCK_BLANK = 0;
	int LUX = 9;

	char COLORLIST[8] = "RYGCBPW";
	char RANDOLIST[7] = "RYGCBP"; 
	char COLORSINIT[5] = "CCGG";

	//

	void INIT () {		
		ESP_LOGD("RGBSEG","SETUP_INIT");
		
		INIT_BOOTDELAY();
		INIT_FASTLOOP();
		
		INIT_SPI();
		INIT_PINS();

		INIT_RGB();
		INIT_SEGBITS();

		ESP_LOGD("RGBSEG","SETUP_DONE");
	}

	void LOOPFX () {
		DO_DT();
		DO_RANDO();
		DO_LUX(); if (LUX>0) { DO_SHOWDIGITS(); }
	}

	//

	void INIT_BOOTDELAY () { delayMicroseconds(1000*1000); }
	void INIT_FASTLOOP () { ESP_FASTLOOP.start(); }
	void INIT_SPI () { SPISET = new SPISettings(1000000, 0, SPI_MODE0); SPI.begin(); }	

	void INIT_RGB () { id(clock_colors).state = COLORSINIT; }

	void INIT_PINS () {
		pinMode(PIN_BLANK, OUTPUT);
		pinMode(PIN_LATCH, OUTPUT);
		pinMode(PIN_D0, OUTPUT);
		pinMode(PIN_D1, OUTPUT);

		DIGIT_SELECT(0);
		LATCH_OPEN(); LATCH_DONE();
		BLANK_ALL(); BLANK_NONE(); BLANK_ALL();
	}

	void INIT_SEGBITS () {
		SEGBITS[0]  = 0b11111100; // 0
		SEGBITS[1]  = 0b01100000; // 1
		SEGBITS[2]  = 0b11011010; // 2
		SEGBITS[3]  = 0b11110010; // 3
		SEGBITS[4]  = 0b01100110; // 4
		SEGBITS[5]  = 0b10110110; // 5
		SEGBITS[6]  = 0b00111110; // 6
		SEGBITS[7]  = 0b11100000; // 7
		SEGBITS[8]  = 0b11111110; // 8
		SEGBITS[9]  = 0b11100110; // 9
		SEGBITS[10] = 0b11101110; // A
		SEGBITS[11] = 0b00111110; // b
		SEGBITS[12] = 0b10011100; // C
		SEGBITS[13] = 0b01111010; // d
		SEGBITS[14] = 0b10011110; // E
		SEGBITS[15] = 0b10001110; // F
	}

	//

	void DO_DT () {
		DTNOW = id(DTHA).now(); DT = DTNOW.timestamp; 
		strftime(DTSTR, sizeof(DTSTR), "%I%M", localtime(&DT));		
	}

	void DO_LUX () {
		LUX = id(CLOCK_LUX); if (LUX==0) { BLANK_ALL(); return; }
		else if (LUX==1) { CLOCK_HOLD=100; CLOCK_BLANK=6000; }
		else if (LUX==2) { CLOCK_HOLD=250; CLOCK_BLANK=4500; }
		else if (LUX==3) { CLOCK_HOLD=250; CLOCK_BLANK=3000; }
		else if (LUX==4) { CLOCK_HOLD=333; CLOCK_BLANK=1500; }
		else if (LUX==5) { CLOCK_HOLD=500; CLOCK_BLANK=1000; }
		else if (LUX==6) { CLOCK_HOLD=666; CLOCK_BLANK=750; }
		else if (LUX==7) { CLOCK_HOLD=750; CLOCK_BLANK=666; }
		else if (LUX==8) { CLOCK_HOLD=750; CLOCK_BLANK=250; }
		else if (LUX==9) { CLOCK_HOLD=1000; CLOCK_BLANK=0; }
	}

	void DO_RANDO () {
		if (RANDOMCOLORS && DT>DTNEXT) {
			DTNEXT = DT + 1;
			int r=0; int lastr=0; for (int i=0;i<=3;i++) { while (r==lastr) { r=random(0,6); }; lastr=r; id(clock_colors).state[i] = RANDOLIST[r]; }
			ESP_LOGD("RGBSEG","RANDO_COLORS: %s",id(clock_colors).state.c_str());
		}
	}

	void DO_SHOWDIGITS () {
		BLANK_NONE();
		for (int i=0;i<=3;i++) {
			long c = strtoul(String(DTSTR[i]).c_str(),NULL,10); // CONVERT 0-F CHAR -> 0-16 LONG
			char color = id(clock_colors).state[i];

			if (id(CLOCK_BLANK)>9) { BLANK_ALL(); delayMicroseconds(id(CLOCK_BLANK)); BLANK_NONE(); };
			DIGIT_SET(i,c,color);
			if (id(CLOCK_HOLD)>9) { delayMicroseconds(id(CLOCK_HOLD)); }
		}
		BLANK_ALL();
	}

	//

	void BLANK_ALL ()  { digitalWrite(PIN_BLANK,LOW); };
	void BLANK_NONE () { digitalWrite(PIN_BLANK,HIGH); };

	void LATCH_DONE () { digitalWrite(PIN_LATCH, LOW);  }
	void LATCH_OPEN () { digitalWrite(PIN_LATCH, HIGH); }

	void DIGIT_SELECT (int digit) {
		if (digit == 0) { digitalWrite(PIN_D0, LOW);  digitalWrite(PIN_D1, LOW);  }
		if (digit == 1) { digitalWrite(PIN_D0, LOW);  digitalWrite(PIN_D1, HIGH); }
		if (digit == 2) { digitalWrite(PIN_D0, HIGH); digitalWrite(PIN_D1, LOW);  }
		if (digit == 3) { digitalWrite(PIN_D0, HIGH); digitalWrite(PIN_D1, HIGH); }
	}

	//

	void DIGIT_SET (int digit, char val, char color) {
		DIGIT_SELECT(digit);
		LATCH_OPEN();
		SPI.beginTransaction(*SPISET);
		if (false) {}
		else if (color=='R') { SPI.transfer(0);            SPI.transfer(0);            SPI.transfer(SEGBITS[val]); }
		else if (color=='Y') { SPI.transfer(0);            SPI.transfer(SEGBITS[val]); SPI.transfer(SEGBITS[val]); }
		else if (color=='G') { SPI.transfer(0);            SPI.transfer(SEGBITS[val]); SPI.transfer(0); }
		else if (color=='C') { SPI.transfer(SEGBITS[val]); SPI.transfer(SEGBITS[val]); SPI.transfer(0); }
		else if (color=='B') { SPI.transfer(SEGBITS[val]); SPI.transfer(0);            SPI.transfer(0); }
		else if (color=='P') { SPI.transfer(SEGBITS[val]); SPI.transfer(0);            SPI.transfer(SEGBITS[val]); }
		else if (color=='W') { SPI.transfer(SEGBITS[val]); SPI.transfer(SEGBITS[val]); SPI.transfer(SEGBITS[val]); }
		else                 { SPI.transfer(SEGBITS[val]); SPI.transfer(SEGBITS[val]); SPI.transfer(SEGBITS[val]); }
		SPI.endTransaction();
		LATCH_DONE();
	}

};

//

/*

	SPITX COLOR ORDER = BGR
	ALL COLOR CODES = RYGCBPW RYGP CRGB

	R = RED
	Y = YELLOW = R+G
	G = GREEN
	C = CYAN   = G+B
	B = BLUE
	P = PURPLE = R+B
	W = WHITE  = R+G+B

	//

	0 ABCDEF__
	1 _BC_____
	2 AB_DE_G_
	3 ABCD__G_
	4 _BC__FG_
	5 A_CD_FG_
	6 __CDEFG_
	7 ABC_____
	8 ABCDEFG_
	9 ABC__FG_
	A ABC_EFG_
	b __CDEFG_
	C A__DEF__
	d _BCDE_G_
	E A__DEFG_
	F A___EFG_

	0 11111100
	1 01100000
	2 11011010
	3 11110010
	4 01100110
	5 10110110
	6 00111110
	7 11100000
	8 11111110
	9 11100110
	A 11101110
	b 00111110
	C 10011100
	d 01111010
	E 10011110
	F 10001110

	SEGBITS[0]  = 0b11111100; // 0
	SEGBITS[1]  = 0b01100000; // 1
	SEGBITS[2]  = 0b11011010; // 2
	SEGBITS[3]  = 0b11110010; // 3
	SEGBITS[4]  = 0b01100110; // 4
	SEGBITS[5]  = 0b10110110; // 5
	SEGBITS[6]  = 0b00111110; // 6
	SEGBITS[7]  = 0b11100000; // 7
	SEGBITS[8]  = 0b11111110; // 8
	SEGBITS[9]  = 0b11100110; // 9
	SEGBITS[10] = 0b11101110; // A
	SEGBITS[11] = 0b00111110; // b
	SEGBITS[12] = 0b10011100; // C
	SEGBITS[13] = 0b01111010; // d
	SEGBITS[14] = 0b10011110; // E
	SEGBITS[15] = 0b10001110; // F

	//

	BRIGHTNESS CONTROLS = BLANK & HOLD

	MIN: BLANK=6000 HOLD=100
	MAX: BLANK=0    HOLD=1000

	BLANK=0    HOLD=500-4000
	BLANK=1000 HOLD=500-3000
	BLANK=2000 HOLD=250-2000
	BLANK=3000 HOLD=200-1500
	BLANK=4000 HOLD=150-1000
	BLANK=5000 HOLD=100-500
	BLANK=6000 HOLD=100

	HOLD=100  BLANK=0-6000
	HOLD=500  BLANK=0-5000
	HOLD=1000 BLANK=0-4000
	HOLD=2000 BLANK=0-3000
	HOLD=3000 BLANK=0-2000
	HOLD=4000 BLANK=0

	0 OFF: BLANK=9999 HOLD=0
	1 MIN: BLANK=6000 HOLD=100
	2 MID: BLANK=1000 HOLD=500
	3 MAX: BLANK=0    HOLD=1000

	0 OFF: BLANK=9999 HOLD=0
	1 MIN: BLANK=6000 HOLD=100
	2 P10: BLANK=4500 HOLD=250
	3 P25: BLANK=3000 HOLD=250
	4 P33: BLANK=1500 HOLD=333
	5 MID: BLANK=1000 HOLD=500
	6 P66: BLANK=750  HOLD=666
	7 P75: BLANK=666  HOLD=750
	8 P90: BLANK=250  HOLD=750
	9 MAX: BLANK=0    HOLD=1000

*/
