
class RGBSEG : public Component {

	int PIN_BLANK = 27;
	int PIN_LATCH = 5;
	int PIN_D0 = 16;
	int PIN_D1 = 17;

	char COLORS[5] = "CCGG";
	bool RANDOMCOLORS = false;

	//

	void setup() override { INIT(); }
	void loop() override { LOOPFX(); }

	uint8_t SEGBITS[16];
	char DTSTR[9]; time_t DT; time_t DTNEXT=0; 
	esphome::time::ESPTime DTNOW;	
	SPISettings *SPISET = nullptr;
	char COLORLIST[8] = "RYGCBPW";

	void INIT () {
		delayMicroseconds(999);
		ESP_LOGD("RGBSEG","SETUP.INIT");
		HighFrequencyLoopRequester FASTLOOP; FASTLOOP.start(); // ELSE 60 hz LIMIT
		INIT_SEGBITS();
		INIT_PINS();
		INIT_SPI();
	}

	void INIT_PINS () {
		pinMode(PIN_BLANK, OUTPUT);
		pinMode(PIN_LATCH, OUTPUT);
		pinMode(PIN_D0, OUTPUT);
		pinMode(PIN_D1, OUTPUT);

		DVOUT(0);
		LATCH_OPEN(); LATCH_DONE();
		BLANK0(); BLANK1();
	}

	void INIT_SPI () {
		SPISET = new SPISettings(1000000, 0, SPI_MODE0);
		SPI.begin();
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

	void BLANK0 () { digitalWrite(PIN_BLANK,LOW); }
	void BLANK1 () { digitalWrite(PIN_BLANK,HIGH); }

	//

	void DVOUT (int dv) {
		if (dv == 0) { digitalWrite(PIN_D0, LOW);  digitalWrite(PIN_D1, LOW);  }
		if (dv == 1) { digitalWrite(PIN_D0, LOW);  digitalWrite(PIN_D1, HIGH); }
		if (dv == 2) { digitalWrite(PIN_D0, HIGH); digitalWrite(PIN_D1, LOW);  }
		if (dv == 3) { digitalWrite(PIN_D0, HIGH); digitalWrite(PIN_D1, HIGH); }
	}

	void LATCH_OPEN () { digitalWrite(PIN_LATCH, LOW);  }
	void LATCH_DONE () { digitalWrite(PIN_LATCH, HIGH); }

	//

	void LOOPFX () {
		DTNOW = id(DTHA).now(); DT = DTNOW.timestamp; 
		strftime(DTSTR, sizeof(DTSTR), "%I%M", localtime(&DT));

		if (RANDOMCOLORS && DT>DTNEXT) {
			DTNEXT = DT+1;
			for (int i=0;i<=3;i++) { COLORS[i] = COLORLIST[random(0,7)]; }
		}
		
		for (int i=0;i<=3;i++) {
			long c = strtoul(String(DTSTR[i]).c_str(),NULL,10);
			DVOUT(i);
			SPI.beginTransaction(*SPISET);
			char color = COLORS[i];
			if (color=='R') { SPI.transfer(0); SPI.transfer(0); SPI.transfer(SEGBITS[c]); }
			if (color=='Y') { SPI.transfer(0); SPI.transfer(SEGBITS[c]); SPI.transfer(SEGBITS[c]); }
			if (color=='G') { SPI.transfer(0); SPI.transfer(SEGBITS[c]); SPI.transfer(0); }
			if (color=='C') { SPI.transfer(SEGBITS[c]); SPI.transfer(SEGBITS[c]); SPI.transfer(0); }
			if (color=='B') { SPI.transfer(SEGBITS[c]); SPI.transfer(0); SPI.transfer(0); }
			if (color=='P') { SPI.transfer(SEGBITS[c]); SPI.transfer(0); SPI.transfer(SEGBITS[c]); }
			if (color=='W') { SPI.transfer(SEGBITS[c]); SPI.transfer(SEGBITS[c]); SPI.transfer(SEGBITS[c]); }
			SPI.endTransaction();
			delayMicroseconds(999);
		}
	}

};

/*

	SPITX COLOR ORDER = BGR
	ALL COLOR CODES = RYGCBPW

	R = RED
	Y = YELLOW = R+G
	G = GREEN
	C = CYAN   = G+B
	B = BLUE
	P = PURPLE = R+B
	W = WHITE  = R+G+B		  

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

*/
