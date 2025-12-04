#include "serialKeyboard.h"

using namespace std;

int serialInterface::initSerial(){
	//initialize wiringOP
	wiringPiSetup(); 

	//initialize serial
	fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1){
		perror("Unable to open serial port");
		return -1;
	}

	struct termios options;
	tcgetattr(fd, &options);
	
	cfsetispeed(&options, B230400);
	cfsetospeed(&options, B230400);
	
	options.c_cflag &= ~PARENB;	//no parity bit
	options.c_cflag &= ~CSTOPB;	//1 stop bit
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag |= CREAD | CLOCAL;	//enable reciever, ignore modem control lines

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	//raw input
	options.c_iflag &= ~(IXON | IXOFF | IXANY);	//no flow control
	options.c_oflag &= ~OPOST;	//raw output
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &options);
	
	/*/
	
	unsigned char msg[3] = { 0xFF, 0x00, 0x06 };	//should be C down
	write(fd, msg, 3);
	
	std::this_thread::sleep_for(std::chrono::seconds(2));
	
	msg[0] = 254;	//key up
	write(fd, msg, 3);
	
	/**/
	
	
	/*/
	unsigned char unicodeMsg[6] = { 253, 0x02, 0x06, 0x02, 0x0D, 251 };
	write(fd, unicodeMsg, 6);
	std::this_thread::sleep_for(std::chrono::seconds(2));
	unicodeMsg[0] = 252;
	write(fd, unicodeMsg, 6);	
	/**/
	
	return 0;
}

void serialInterface::endSerial(){
	close(fd);
}
		
bool serialInterface::pressKey(uint16_t keyCode){
	cout << "Serial Key: " << keyCode << endl;
	keyDownMsg[1] = keyCode >> 8;
	keyDownMsg[2] = keyCode & 0xFF;
	return write(fd, keyDownMsg, sizeof(keyDownMsg));
}

bool serialInterface::releaseKey(uint16_t keyCode){
	keyUpMsg[1] = keyCode >> 8;
	keyUpMsg[2] = keyCode & 0xFF;
	return write(fd, keyUpMsg, sizeof(keyDownMsg));
}

bool serialInterface::pressUnicode(uint8_t* unicodeCode, int size){
	int i;
	for (i = 0; i < size; i++){
		unicodeDownMsg[i+1] = unicodeCode[i];
	}
	unicodeDownMsg[i+1] = unicodeStringEnd;
	return write(fd, unicodeDownMsg, size+2);
}

bool serialInterface::releaseUnicode(uint8_t* unicodeCode, int size){
	int i;
	for (i = 0; i < size; i++){
		unicodeUpMsg[i+1] = unicodeCode[i];
	}
	unicodeUpMsg[i+1] = unicodeStringEnd;
	return write(fd, unicodeUpMsg, size+2);
}