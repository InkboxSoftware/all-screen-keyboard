//keyboard.h
#include <stdio.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <GLES3/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <atomic>
#include <dlfcn.h>

#include "../config.h"
#include "key.h"
#include "readHallEffect.h"
#include "graphics.h"
#include "json.hpp"
#include "serialKeyboard.h"

#ifdef useBluetooth
	#include "bluetooth.h"
#endif

#define CIRCLESEGMENTS 100

#define MAXKEYS 112

class keyboard{

public:
	keyboard();
	int run();

private:
	std::vector<key*> keys;

	void keyboardMonitor(bool* on);	//runs keyboard and output to serial

	//load initial profile:
	int preextractProfiles();
	int loadProfile(const char* profileFolderName, bool mainThread=true);
	int swapProfile(const char* newProfileFolderName);
	
	int textureKey(int keyID, int layoutType, std::string layoutString, int modifiers, std::string profilePath="", int customShader=-1);
	int rectKey(int keyID);
	int circleKey(int keyID, int radius, int segments);
	
	int assignKeyboardFont(const char* fontname);
	int assignTextColor(int r, int g, int b, int a);
	int setKeyboardBGImg(const char* src);
	int setKeyboardBackgroundColor(int r, int g, int b, int a);

	int freeRect(int z, int x, int y, int w, int h, int r, int g, int b, int a, int customShader=-1);
	int freeCircle(int z, int x, int y, int radius, int segments, int r, int g, int b, int a, int customShader=-1);
	int freeTexture(int z, int x, int y, int w, int h, std::string layoutText, bool isImage, int r, int g, int b, int a, std::string profilePath="", int font=0, int customShader=-1);
	int freeDebugText(int x, int y, const char* toWrite, float size, int r, int g, int b, int a);
	//you could add custom shader numbers as an optional parameter
	
	int loadVideo(const char* filename, int x, int y, int w, int h);
	
	//loadShaders
	int loadCustomShader(const char* vertexShader, const char* fragmentShader);
	
	void runHallEffectSensors(bool* on);

	//helper functions:
	bool parseRGBAfromHex(const std::string& RGBA, int* r, int* g, int* b, int *a);
	
	//for functions that need to be run in main thread:
	bool inMainThread = true;	//starts in main thread

	struct color {
		int r;
		int g;
		int b;
		int a;
	};
	
	int modifierBit[5] = { 0x11, 0x22, 0x44, 0x88, 0x100 };

	//variables
	window* keyboardWindow;
	serialInterface* serial;
	color textColor;
	std::vector<std::string> profileFonts;
	
	//key variables
	#define avgSize 10
	int keyVals[MAXKEYS][avgSize];
	int keySignal[MAXKEYS];
	
	//profile variables
	nlohmann::json profileJSON;
	std::string path;
	bool wantToRerender = false;
	
	//modifier key tracking
	std::vector<int> leftShiftKeys;
	std::vector<int> leftCtrlKeys;
	std::vector<int> leftAltKeys;
	std::vector<int> leftUIKeys;
	std::vector<int> rightShiftKeys;
	std::vector<int> rightCtrlKeys;
	std::vector<int> rightAltKeys;
	std::vector<int> rightUIKeys;
	std::vector<int> FnKeys;
	std::vector<int> capsKeys;
	
	//plugins
	struct customPlugin {
		void* handle;
		int (*onLoad)(std::vector<key*>);
		int (*onPress)(std::vector<key*>);
	};
	std::vector<customPlugin> plugins;
	bool loadPlugin(std::string pluginPath, std::string pluginName);
	
	//debugging text:
	bool debugTextOn = false;
	std::vector<text*> debugText;
	
	int getOutputType(std::string outputType){
		if (outputType == "HID")
			return HIDOUTPUT;
		else if (outputType == "Unicode")
			return UNICODEOUTPUT;
		else if (outputType == "ProfSwap")
			return PROFILESWAPOUTPUT;
		return -1;
	}
};