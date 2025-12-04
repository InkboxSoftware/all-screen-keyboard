#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <iostream>

#include <wiringPi.h>
#include "keycodes.h"


//constants
#define TX_PIN 15
#define bit_time 98
#define keyEventDown 255
#define keyEventUp 254
#define unicodeEventDown 253
#define unicodeEventUp 252
#define unicodeStringEnd 251

class serialInterface {
	public:
		int initSerial();
		bool pressKey(uint16_t keyCode);
		bool releaseKey(uint16_t keyCode);
		bool pressUnicode(uint8_t* unicodeCode, int size);
		bool releaseUnicode(uint8_t* unicodeCode, int size);
		void endSerial();
	
	private:
		int fd;	//file descriptor for serial0
		uint8_t keyDownMsg[3] = { keyEventDown, 0x00, 0x00 };
		uint8_t keyUpMsg[3] = { keyEventUp, 0x00, 0x00 };
		uint8_t unicodeDownMsg[10] = { unicodeEventDown, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		uint8_t unicodeUpMsg[10] = { unicodeEventUp, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		
};