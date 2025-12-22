#pragma once

#include <iostream>
#include <cctype>
#include "keycodes.h"
#include <unordered_map>
#include <algorithm>
#include <cstdint>

#define UNDEFINEDOUTPUT -1
#define HIDOUTPUT 0
#define UNICODEOUTPUT 1
#define PROFILESWAPOUTPUT 2

#define numberOfModifiers 5

class key{
	public:
		//press
		bool pressed = false;
		bool lastPressed = false;
		
		//UI
		int z = 0;	//layer level (0 is lowest, goes up to 4)
		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;
		int r = 0;
		int g = 0;
		int b = 0;
		int a = 0;
		bool isRect;		//true drawn with rect, false, draw circle
		std::string text;	//if set to image type, text is image path
		int font;
		bool useImage = false;
		bool affectedByCapsLock = false;
		int shader = -1;		//associated default shader
		int pressShader = -1;	//assocaited render shader

		
		//left ctrl, left shift, left alt, left UI, right ctrl, right shift, right alt, right UI
		std::string modText[numberOfModifiers];		//text change for when pressed with modifier keys
		int modShader[numberOfModifiers];	//shader change for when pressed with modifier keys
		int modOutputType[numberOfModifiers];
		std::string modOutputValue[numberOfModifiers];	//output values for when pressed with modifier keys
		
		//plugin event
		int pluginEvent = -1;
		
		//output
		int outputType = -1;	//no event defined
		std::string outputValue;
		
		unsigned int HIDcode = -1;
		uint8_t unicodeCode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		int unicodeLength;
		
		key(int z, int x, int y, int w, int h, int r, int g, int b, int a, std::string text, int font, bool isRect,
			bool useImage, bool affectedByCapsLock, int shader, int pressShader, int outputType, std::string outputValue, 
			std::string modText[numberOfModifiers], int modShader[numberOfModifiers], 
			int modOutputType[numberOfModifiers], std::string modOutputValue[numberOfModifiers], int pluginEvent){
			this->z = z;
			this->x = x;
			this->y = y;
			this->w = w;
			this->h = h;
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
			this->text = text;
			this->font = font;
			this->isRect = isRect;
			this->useImage = useImage;
			this->affectedByCapsLock = affectedByCapsLock;
			this->shader = shader;
			this->pressShader = pressShader;
			this->outputType = outputType;
			this->outputValue = outputValue;
			for (int i = 0; i < numberOfModifiers; i++){
				this->modText[i] = modText[i];
				this->modShader[i] = modShader[i];
				this->modOutputType[i] = modOutputType[i];
				this->modOutputValue[i] = modOutputValue[i];
			}
			this->pluginEvent = pluginEvent;
			setOutput();
			return;
		}
		
	
	void setOutput(){	
		if (outputType == HIDOUTPUT)
			this->HIDcode = getHIDcode(this->outputValue);
		else if (outputType == UNICODEOUTPUT)
			setupUnicodeOutput();
		return;
	}
		
	private:
		
		unsigned int setupUnicodeOutput(){
			unicodeLength = 0;
			for (int i = 0; i < outputValue.length() && i < 8; i++){
				unicodeCode[i] = hexCharToInt(outputValue[i]);
				unicodeLength++;
			}
			return unicodeLength;
		}
		
		uint8_t hexCharToInt(char c){
			c = std::toupper(c);
			if (c >= '0' && c <= '9')
				return static_cast<uint8_t>(c - '0');
			else if (c >= 'A' && c <= 'F')
				return (static_cast<uint8_t>(c - 'A') + 10);
			else
				return 0;
			
		}
	
		unsigned int getHIDcode(std::string& outputValue){
			std::string result = outputValue;
			std::transform(result.begin(), result.end(), result.begin(),
					[](unsigned char c) { return std::toupper(c); });
			auto foundKeyCode = stringToKC.find(result);
			if (foundKeyCode != stringToKC.end()){
				return foundKeyCode->second;	//return the keycode value
			}
			else
				return 0;
		}
		
	std::unordered_map<std::string, int> stringToKC = {
		{"FN", 		KC_FN},
	  	{"NULL", 	KC_NO},
		{"TRANSPARENT",	KC_TRANSPARENT},
		{"A", 		KC_A},
		{"B", 		KC_B},
		{"C", 		KC_C},
		{"D",		KC_D},
		{"E",		KC_E},
		{"F", 		KC_F},
		{"G", 		KC_G},
		{"H", 		KC_H},
		{"I", 		KC_I},
		{"J", 		KC_J},
		{"K", 		KC_K},
		{"L", 		KC_L},
		{"M",		KC_M},
		{"N", 		KC_N},
		{"O",		KC_O},
		{"P", 		KC_P},
		{"Q", 		KC_Q},
		{"R", 		KC_R},
		{"S", 		KC_S},
		{"T", 		KC_T},
		{"U", 		KC_U},
		{"V", 		KC_V},
		{"W", 		KC_W},
		{"X", 		KC_X},
		{"Y", 		KC_Y},
		{"Z", 		KC_Z},
		{"1", 		KC_1},
		{"2", 		KC_2},
		{"3", 		KC_3},
		{"4", 		KC_4},
		{"5", 		KC_5},
		{"6", 		KC_6},
		{"7", 		KC_7},
		{"8", 		KC_8},
		{"9", 		KC_9},
		{"0", 		KC_0},
		{"ENTER", 	KC_ENTER},
		{"ESCAPE",  KC_ESCAPE},
		{"ESCPACE", KC_ESCAPE}, // typo kept for compatibility
		{"BACKSPACE", KC_BACKSPACE},
		{"TAB", 	KC_TAB},
		{"SPACE", 	KC_SPACE},
		{"MINUS", 	KC_MINUS},
		{"EQUAL", 	KC_EQUAL},
		{"LEFT_BRACKET", KC_LEFT_BRACKET},
		{"RIGHT_BRACKET", KC_RIGHT_BRACKET},
		{"BACKSLASH", KC_BACKSLASH},
		{"NONUS_HASH", KC_NONUS_HASH},
		{"SEMICOLON", KC_SEMICOLON},
		{"QUOTE", 	KC_QUOTE},
		{"GRAVE", 	KC_GRAVE},
		{"COMMA", 	KC_COMMA},
		{"DOT", 	KC_DOT},
		{"SLASH", 	KC_SLASH},
		{"CAPS_LOCK", KC_CAPS_LOCK},
		{"F1", 		KC_F1},
		{"F2", 		KC_F2},
		{"F3", 		KC_F3},
		{"F4", 		KC_F4},
		{"F5", 		KC_F5},
		{"F6", 		KC_F6},
		{"F7", 		KC_F7},
		{"F8", 		KC_F8},
		{"F9", 		KC_F9},
		{"F10", 	KC_F10},
		{"F11", 	KC_F11},
		{"F12", 	KC_F12},
		{"PRINT_SCREEN", KC_PRINT_SCREEN},
		{"SCROLL_LOCK", KC_SCROLL_LOCK},
		{"PAUSE", KC_PAUSE},
		{"INSERT", KC_INSERT},
		{"HOME", KC_HOME},
		{"PAGE_UP", KC_PAGE_UP},
		{"DELETE", KC_DELETE},
		{"END", KC_END},
		{"PAGE_DOWN", KC_PAGE_DOWN},
		{"PACE_DOWN", KC_PAGE_DOWN}, // typo kept for compatibility
		{"RIGHT", KC_RIGHT},
		{"LEFT", KC_LEFT},
		{"DOWN", KC_DOWN},
		{"UP", KC_UP},
		{"NUM_LOCK", KC_NUM_LOCK},
		{"KP_SLASH", KC_KP_SLASH},
		{"KP_ASTERISK", KC_KP_ASTERISK},
		{"KP_MINUS", KC_KP_MINUS},
		{"KP_PLUS", KC_KP_PLUS},
		{"KP_ENTER", KC_KP_ENTER},
		{"KP_1", KC_KP_1},
		{"KP_2", KC_KP_2},
		{"KP_3", KC_KP_3},
		{"KP_4", KC_KP_4},
		{"KP_5", KC_KP_5},
		{"KP_6", KC_KP_6},
		{"KP_7", KC_KP_7},
		{"KP_8", KC_KP_8},
		{"KP_9", KC_KP_9},
		{"KP_0", KC_KP_0},
		{"KP_DOT", KC_KP_DOT},
		{"NONUS_BACKSLASH", KC_NONUS_BACKSLASH},
		{"KC_APPLICATION", KC_APPLICATION},
		{"KB_POWER", KC_KB_POWER},
		{"KP_EQUAL", KC_KP_EQUAL},
		{"F13", KC_F13},
		{"F14", KC_F14},
		{"F15", KC_F15},
		{"F16", KC_F16},
		{"F17", KC_F17},
		{"F18", KC_F18},
		{"F19", KC_F19},
		{"F20", KC_F20},
		{"F21", KC_F21},
		{"F22", KC_F22},
		{"F23", KC_F23},
		{"F24", KC_F24},
		{"EXECUTE", KC_EXECUTE},
		{"HELP", KC_HELP},
		{"MENU", KC_MENU},
		{"SELECT", KC_SELECT},
		{"STOP", KC_STOP},
		{"AGAIN", KC_AGAIN},
		{"UNDO", KC_UNDO},
		{"CUT", KC_CUT},
		{"COPY", KC_COPY},
		{"PASTE", KC_PASTE},
		{"FIND", KC_FIND},
		{"KB_MUTE", KC_KB_MUTE},
		{"KB_VOLUME_UP", KC_KB_VOLUME_UP},
		{"KB_VOLUME_DOWN", KC_KB_VOLUME_DOWN},
		{"LOCKING_CAPS_LOCK", KC_LOCKING_CAPS_LOCK},
		{"LOCKING_NUM_LOCK", KC_LOCKING_NUM_LOCK},
		{"LOCKING_SCROLL_LOCK", KC_LOCKING_SCROLL_LOCK},
		{"KP_COMMA", KC_KP_COMMA},
		{"KP_EQUAL", KC_KP_EQUAL_AS400},
		{"INTERNATIONAL_1", KC_INTERNATIONAL_1},
		{"INTERNATIONAL_2", KC_INTERNATIONAL_2},
		{"INTERNATIONAL_3", KC_INTERNATIONAL_3},
		{"INTERNATIONAL_4", KC_INTERNATIONAL_4},
		{"INTERNATIONAL_5", KC_INTERNATIONAL_5},
		{"INTERNATIONAL_6", KC_INTERNATIONAL_6},
		{"INTERNATIONAL_7", KC_INTERNATIONAL_7},
		{"INTERNATIONAL_8", KC_INTERNATIONAL_8},
		{"INTERNATIONAL_9", KC_INTERNATIONAL_9},
		{"LANGUAGE_1", KC_LANGUAGE_1},
		{"LANGUAGE_2", KC_LANGUAGE_2},
		{"LANGUAGE_3", KC_LANGUAGE_3},
		{"LANGUAGE_4", KC_LANGUAGE_4},
		{"LANGUAGE_5", KC_LANGUAGE_5},
		{"LANGUAGE_6", KC_LANGUAGE_6},
		{"LANGUAGE_7", KC_LANGUAGE_7},
		{"LANGUAGE_8", KC_LANGUAGE_8},
		{"LANGUAGE_9", KC_LANGUAGE_9},
		{"ALTERNATE_ERASE", KC_ALTERNATE_ERASE},
		{"SYSTEM_REQUEST", KC_SYSTEM_REQUEST},
		{"CANCEL", KC_CANCEL},
		{"CLEAR", KC_CLEAR},
		{"PRIOR", KC_PRIOR},
		{"RETURN", KC_RETURN},
		{"SEPARATOR", KC_SEPARATOR},
		{"OUT", KC_OUT},
		{"OPER", KC_OPER},
		{"CLEAR_AGAIN", KC_CLEAR_AGAIN},
		{"CRSEL", KC_CRSEL},
		{"EXSEL", KC_EXSEL},
		{"SYSTEM_POWER", KC_SYSTEM_POWER},
		{"SYSTEM_SLEEP", KC_SYSTEM_SLEEP},
		{"SYSTEM_WAKE", KC_SYSTEM_WAKE},
		{"AUDIO_MUTE", KC_AUDIO_MUTE},
		{"AUDIO_VOL_UP", KC_AUDIO_VOL_UP},
		{"AUDIO_VOL_DOWN", KC_AUDIO_VOL_DOWN},
		{"MEDIA_NEXT_TRACK", KC_MEDIA_NEXT_TRACK},
		{"MEDIA_PREV_TRACK", KC_MEDIA_PREV_TRACK},
		{"MEDIA_STOP", KC_MEDIA_STOP},
		{"MEDIA_PLAY_PAUSE", KC_MEDIA_PLAY_PAUSE},
		{"MEDIA_SELECT", KC_MEDIA_SELECT},
		{"MEDIA_EJECT", KC_MEDIA_EJECT},
		{"MAIL", KC_MAIL},
		{"CALCULATOR", KC_CALCULATOR},
		{"MY_COMPUTER", KC_MY_COMPUTER},
		{"WWW_SEARCH", KC_WWW_SEARCH},
		{"WWW_HOME", KC_WWW_HOME},
		{"WWW_BACK", KC_WWW_BACK},
		{"WWW_FORWARD", KC_WWW_FORWARD},
		{"WWW_STOP", KC_WWW_STOP},
		{"WWW_REFRESH", KC_WWW_REFRESH},
		{"WWW_FAVORITES", KC_WWW_FAVORITES},
		{"MEDIA_FAST_FORWARD", KC_MEDIA_FAST_FORWARD},
		{"MEDIA_REWIND", KC_MEDIA_REWIND},
		{"BRIGHTNESS_UP", KC_BRIGHTNESS_UP},
		{"BRIGHTNESS_DOWN", KC_BRIGHTNESS_DOWN},
		{"CONTROL_PANEL", KC_CONTROL_PANEL},
		{"ASSISTANT", KC_ASSISTANT},
		{"MISSION_CONTROL", KC_MISSION_CONTROL},
		{"LAUNCHPAD", KC_LAUNCHPAD},
		{"MOUSE_CURSOR_UP", QK_MOUSE_CURSOR_UP},
		{"MOUSE_CURSOR_DOWN", QK_MOUSE_CURSOR_DOWN},
		{"MOUSE_CURSOR_LEFT", QK_MOUSE_CURSOR_LEFT},
		{"MOUSE_CURSOR_RIGHT", QK_MOUSE_CURSOR_RIGHT},
		{"MOUSE_BUTTON_1", QK_MOUSE_BUTTON_1},
		{"MOUSE_BUTTON_2", QK_MOUSE_BUTTON_2},
		{"MOUSE_BUTTON_3", QK_MOUSE_BUTTON_3},
		{"MOUSE_BUTTON_4", QK_MOUSE_BUTTON_4},
		{"MOUSE_BUTTON_5", QK_MOUSE_BUTTON_5},
		{"MOUSE_BUTTON_6", QK_MOUSE_BUTTON_6},
		{"MOUSE_BUTTON_7", QK_MOUSE_BUTTON_7},
		{"MOUSE_BUTTON_8", QK_MOUSE_BUTTON_8},
		{"MOUSE_WHEEL_UP", QK_MOUSE_WHEEL_UP},
		{"MOUSE_WHEEL_DOWN", QK_MOUSE_WHEEL_DOWN},
		{"MOUSE_WHEEL_LEFT", QK_MOUSE_WHEEL_LEFT},
		{"MOUSE_WHEEL_RIGHT", QK_MOUSE_WHEEL_RIGHT},
		{"MOUSE_ACCELERATION_0", QK_MOUSE_ACCELERATION_0},
		{"MOUSE_ACCELERATION_1", QK_MOUSE_ACCELERATION_1},
		{"MOUSE_ACCELERATION_2", QK_MOUSE_ACCELERATION_2},
		{"LEFT_CTRL", KC_LEFT_CTRL},
		{"LEFT_SHIFT", KC_LEFT_SHIFT},
		{"LEFT_ALT", KC_LEFT_ALT},
		{"LEFT_GUI", KC_LEFT_GUI},
		{"RIGHT_CTRL", KC_RIGHT_CTRL},
		{"RIGHT_SHIFT", KC_RIGHT_SHIFT},
		{"RIGHT_ALT", KC_RIGHT_ALT},
		{"RIGHT_GUI", KC_RIGHT_GUI}
	};
};