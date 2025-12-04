//keyboard.cpp

#include "keyboard.h"

using namespace std;

#define CSp0 17
#define CSp1 20
#define CSp2 22
#define CSp3 25
#define SPIchannel 0
#define SPIspeed 500000



keyboard::keyboard(){
	keyboardWindow = new window();
	int OK = keyboardWindow->initialize(screenWidth, screenHeight);
	//int OK = keyboardWindow->initialize(1920, 1080, false);
	if (OK != 0){
		printf("Error initializing window\n");
		return;
	}
	
	return;
}

int keyboard::run(){
	//unzip any new profiles that have arrived:
	int res = preextractProfiles();
	if (res == -1){	//error in extraction
		return -1;
	}	
	//check if startup file is found
	string profileToLoad = initialProfile;
	ifstream file("startup");
	if (!file.is_open()){
		cout << "startup not found. Using default profile: " << initialProfile << endl;
	}
	else{
		getline(file, profileToLoad);
	}
	//load profiles:
	loadProfile(profileToLoad.c_str());
	//frame counter text:
	debugText.push_back(keyboardWindow->writeText("000", 100, 100, 2.0f, 0, 0, 0, 255));
	
	
	serial = new serialInterface();
	if (serial->initSerial() == -1){
		cout << "Serial could not be initialized, exiting program" << endl;
		return 0;
	}
	
	bool keyboardActive = true;
	bool* on = &keyboardActive;
		
	#ifdef useBluetooth
		bluetoothConn = new bluetooth();	//init bluetooth connection:
	#endif
	
	//clean this up later when bluetooth and halleffect comes back:
	/*
	//run bluetooth interaction:
	//thread bluetoothThread([this, on](){ bluetoothConn->begin(on, false); });	//true for testing, false for bluetooth on
	//run hall effect thread:
	thread hallEffectThread([this, on](){ runHallEffectSensors(on); });
	*/

	//keyboard monitor thread:
	thread monitorThread([this, on](){ keyboardMonitor(on); });

	//run graphics thread:
	keyboardWindow->runWindow(keys);	//graphics calls must run in main thread
	
	//once returning from this thread, the rest of the program should end
	keyboardActive = false;	
	monitorThread.join();
	//hallEffectThread.join();	//wait for hall effect polling to finish
	//bluetoothThread.join();
	

	return 0;	//ends program
}

void keyboard::keyboardMonitor(bool* on){
	while (*on){
	if (wantToRerender){
	//rerender keys
	inMainThread = false;
	keyboardWindow->setGraphicsLock(true);
	while (keyboardWindow->isRendering())
		__asm__("nop");	//wait for rendering to finish
							cout << "non-main thread clearing" << endl;
							keyboardWindow->requestClear();
							while (!keyboardWindow->asyncFunctionCompleted())
								__asm__("nop");
								
								
	string shaderPath = "profiles/scram/shaders/";
	int numberOfCustomShaders = profileJSON["shaders"].size();
	for (int i = 0; i < numberOfCustomShaders; i++){
		string vShaderPath = profileJSON["shaders"][i];
		vShaderPath = (shaderPath + vShaderPath + ".vert");
		string fShaderPath = profileJSON["shaders"][i];
		fShaderPath = (shaderPath + fShaderPath + ".frag");
		if (loadCustomShader(vShaderPath.c_str(), fShaderPath.c_str()) != 0){
			cout << "Could not get compile custom shader " << profileJSON["shaders"][i] << endl;
			numberOfCustomShaders--;
		}
	}
									//check custom shaders:
	int gShader = profileJSON["shaderGlobal"];
	if (gShader >= 0 && keyboardWindow->setCustomGlobalTextureShader(gShader) == -1){
		cout << "Global texture shader out of range: " << gShader<< endl
		<< "\tCurrent Number of shaders: " << numberOfCustomShaders << endl;
	}
	gShader = profileJSON["pressShaderGlobal"];
	if (gShader >= 0 && keyboardWindow->setCustomGlobalPressedTextureShader(gShader) == -1){
		cout << "Global texture shader out of range: " << gShader << endl
		<< "\tCurrent Number of shaders: " << numberOfCustomShaders << endl;
	}
								//set keyboard UI graphics:
								
	for (int i = 0; i < keys.size(); i++){	
		int heldTypeModifier = 0;
		if (keys[i]->useImage || keys[i]->text.length() > 0){	//texture via image or text
			for (int j = 0; j < numberOfModifiers; j++){
				if (keys[i]->modText[j].length() > 0){
					heldTypeModifier |= modifierBit[j];
				}
			}
			for (int j = 0; j < numberOfModifiers; j++){
				if (keys[i]->modText[j].length() > 0){
					int newMod = ((heldTypeModifier - modifierBit[j]) << 16) + modifierBit[j];
					textureKey(i, keys[i]->useImage, keys[i]->modText[j], newMod, path, keys[i]->modShader[j]);
				}
			}
			heldTypeModifier = ((0x1FF ^ heldTypeModifier) & 0x1FF) + FORNORMALTEXTURE;
			if (heldTypeModifier == 0x1FF + FORNORMALTEXTURE) heldTypeModifier = -1;
			textureKey(i, keys[i]->useImage, keys[i]->text, heldTypeModifier, path);
		}
		else if (keys[i]->isRect){	//is a rectangle
			rectKey(i);
		}
		else if (!keys[i]->isRect){	//is a circle
			circleKey(i, keys[i]->w / 2, CIRCLESEGMENTS);
		}
	}
	
	wantToRerender = false;
	keyboardWindow->setGraphicsLock(false);
	continue;
	}
	//new touch screen monitoring version:
	while (keyboardWindow->eventsToProcess.load(memory_order_acquire) == 0 && *on)
		__asm__("nop");		//idle until events available
	while (keyboardWindow->accessInputMUX.load(memory_order_acquire) > 0 && *on)
		__asm__("nop");		//idle until access is available
	if (!*on) break;
	keyboardWindow->accessInputMUX.fetch_add(1, memory_order_acq_rel);		//obtain access
	for (int i = 0; i < keyboardWindow->touchIOqueue.size(); i++){
		//process event
		int coreX = static_cast<int>(keyboardWindow->touchIOqueue[i].x * screenWidth);
		int coreY = static_cast<int>(keyboardWindow->touchIOqueue[i].y * screenHeight);
		int keyArraySize = keys.size();
		for (int k = 0; k < keyArraySize; k++){
			if (keys[k]->x <= coreX && keys[k]->x + keys[k]->w >= coreX &&
				keys[k]->y <= coreY && keys[k]->y + keys[k]->h >= coreY){
				keys[k]->pressed = keyboardWindow->touchIOqueue[i].press;
				//output:
				switch (keys[k]->outputType){
					case HIDOUTPUT:
						switch (keyboardWindow->touchIOqueue[i].press){
							case true:
								cout << "HID" << endl;
								serial->pressKey(keys[k]->HIDcode);
								break;
							default:
								serial->releaseKey(keys[k]->HIDcode);
								break;
						}
						break;
					case UNICODEOUTPUT:
						switch (keyboardWindow->touchIOqueue[i].press){
							case true:
								cout << "Unicode out: " << keys[k]->outputValue << " - " << keys[k]->unicodeLength << endl;
								serial->pressUnicode(keys[k]->unicodeCode, keys[k]->unicodeLength);
								break;
							default:
								serial->releaseUnicode(keys[k]->unicodeCode, keys[k]->unicodeLength);
								break;
						}
						break;
					case PROFILESWAPOUTPUT:
						switch (keyboardWindow->touchIOqueue[i].press){
							case true:
								cout << "Swap profile: " << keys[k]->outputValue << endl;
								break;
							default:
								//cout << "Swap profile up: " << keys[k]->outputValue << endl;
								swapProfile(keys[k]->outputValue.c_str());
								keyArraySize = 0;
								break;
						}
						break;
					default:
						cout << "unknown output type: " << keys[k]->outputType << endl;
						break;
				}
				//running plugin events on key up:
				if (keyboardWindow->touchIOqueue[i].press){
					if (keys[k]->pluginEvent >= 0 && keys[k]->pluginEvent < plugins.size()){
						if (plugins[keys[k]->pluginEvent].onPress){			
							cout << "running plugin: " << keys[k]->pluginEvent << endl;
							int ret = plugins[keys[k]->pluginEvent].onPress(keys);
							if (ret == 1){
								wantToRerender = true;	
							}
						}
					}
				}
				break;
			}
		}
		
		//cout << "Processed (" << coreX << ", " << coreY << "): " << keyboardWindow->touchIOqueue[i].press << endl;	
		keyboardWindow->eventsToProcess.fetch_sub(1, memory_order_acq_rel);	//decrease events
	}
	keyboardWindow->touchIOqueue.clear();
	keyboardWindow->accessInputMUX.fetch_sub(1, memory_order_acq_rel);		//release access
	}
	serial->endSerial();
}

int keyboard::swapProfile(const char* newProfileFolderName){
	keyboardWindow->setGraphicsLock(true);	//lock the graphics process to allow for async requests
	while (keyboardWindow->isRendering())
		__asm__("nop");	//wait for rendering to finish
	return loadProfile(newProfileFolderName, false);	//does not run in main thread
}

int keyboard::loadProfile(const char* profileFolderName, bool mainThread){
	inMainThread = mainThread;	
	//determine if the profile to load exists:
	struct stat sb;
	string profiles = "profiles/";
	path = (profiles + profileFolderName);
	if (!(stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
		cout << "No profile folder with name: " << path << " found" << endl;
		return -1;
	}
	cout << "Loading keyboard profile: " << path << endl;	
	
	//determine if layout.prof is found
	string profile = "/layout.prof";
	string profPath = (path + profile);
	ifstream file(profPath.c_str());
	if (!file.good()){
		cout << "No layout.prof file found in " << path << endl;
		return -1;
	}
	cout << "Loading layout.prof..." << endl;
	
	//at this point it's okay to clear the graphics memory:
	if (mainThread)
		keyboardWindow->clearGraphicsMemory();
	else{
		cout << "non-main thread clearing" << endl;
		keyboardWindow->requestClear();
		while (!keyboardWindow->asyncFunctionCompleted())
			__asm__("nop");
	}
	
	//read file and update layout
	file >> profileJSON;
	
	keys.clear();			//clear keys vector
	profileFonts.clear();	//clear font vector<string>
	
	//load global values:
	int numberOfKeys = profileJSON["keys"].size();
	cout << "Number of Keys: " << numberOfKeys << endl;
	int globalOffsetX = profileJSON["globalOffsetX"];
	int globalOffsetY = profileJSON["globalOffsetY"];
	profileFonts = profileJSON["fonts"];	//vector of strings
	for (int i = 0; i < profileFonts.size(); i++){
		profileFonts[i] = (path + "/fonts/" + profileFonts[i]);
		cout << profileFonts[i] << endl;
	}
	
	//background:

	int r; int g; int b; int a;
	string BGcolor = profileJSON["backgroundColor"];
	if (BGcolor.length() == 8 || BGcolor.length() == 9){
		parseRGBAfromHex(profileJSON["backgroundColor"], &r, &g, &b, &a);
		setKeyboardBackgroundColor(r, g, b, a);
	}
	string imgPath = profileJSON["backgroundImgPath"];
	if (imgPath.length() > 0){
		if (!profileJSON["backgroundIsVideo"]){
			setKeyboardBGImg((path + "/images/" + imgPath).c_str());
		}
		else{
			loadVideo((path + "/images/" + imgPath).c_str(), 0, 0, screenWidth, screenHeight);
		}
	}
	
	//load shaders
	string layoutName = profileFolderName;
	string shaderPath = "profiles/" + layoutName + "/shaders/";
	int numberOfCustomShaders = profileJSON["shaders"].size();
	for (int i = 0; i < numberOfCustomShaders; i++){
		string vShaderPath = profileJSON["shaders"][i];
		vShaderPath = (shaderPath + vShaderPath + ".vert");
		string fShaderPath = profileJSON["shaders"][i];
		fShaderPath = (shaderPath + fShaderPath + ".frag");
		if (loadCustomShader(vShaderPath.c_str(), fShaderPath.c_str()) != 0){
			cout << "Could not get compile custom shader " << profileJSON["shaders"][i] << endl;
			numberOfCustomShaders--;
		}
	}
	
	//check custom shaders:
	int gShader = profileJSON["shaderGlobal"];
	if (gShader >= 0 && keyboardWindow->setCustomGlobalTextureShader(gShader) == -1){
		cout << "Global texture shader out of range: " << gShader<< endl
		<< "\tCurrent Number of shaders: " << numberOfCustomShaders << endl;
	}
	gShader = profileJSON["pressShaderGlobal"];
	if (gShader >= 0 && keyboardWindow->setCustomGlobalPressedTextureShader(gShader) == -1){
		cout << "Global texture shader out of range: " << gShader << endl
		<< "\tCurrent Number of shaders: " << numberOfCustomShaders << endl;
	}

	
	//load keys:
	for (int i = 0; i < numberOfKeys; i++){
		parseRGBAfromHex(profileJSON["keys"][i]["RGBA"], &r, &g, &b, &a);
		int x = profileJSON["keys"][i]["X"];
		int y = profileJSON["keys"][i]["Y"];
		string modText[numberOfModifiers];
		int modShader[numberOfModifiers];
		int modOutputTypes[numberOfModifiers];
		string modOutputValue[numberOfModifiers];
		if (profileJSON["keys"][i]["modText"].size() != numberOfModifiers){
			cout << "Incorrect number of text entries in \"modText\" for key " << i 
				<< " (" << profileJSON["keys"][i]["outputValue"] << ")" << endl;
			return -1;
		}
		if (profileJSON["keys"][i]["modShader"].size() != numberOfModifiers){
			cout << "Incorrect number of text entries in \"modShader\" for key " << i 
				<< " (" << profileJSON["keys"][i]["outputValue"] << ")" << endl;
			return -1;
		}
		for (int j = 0; j < numberOfModifiers; j++){
			modText[j] = profileJSON["keys"][i]["modText"][j];
			modShader[j] = profileJSON["keys"][i]["modShader"][j];
			modOutputTypes[j] = getOutputType(profileJSON["keys"][i]["modType"][j]);
			modOutputValue[j] = profileJSON["keys"][i]["modOutputValue"][j];
		}
		//checking shaders
		int kShader = profileJSON["keys"][i]["shader"];
		if (kShader >= numberOfCustomShaders){ 
			kShader = -1;
			cout << "Custom Shader for key " << i << 
				" (" << profileJSON["keys"][i]["outputValue"] << ") out of range" << endl;
		}
		int kPressShader = profileJSON["keys"][i]["pressShader"];
		if (kPressShader >= numberOfCustomShaders){ 
			kPressShader = -1;
			cout << "Custom Press Shader for key " << i << 
				" (" << profileJSON["keys"][i]["outputValue"] << ") out of range" << endl;
		}
		//push keys into the keys vector
		key* k = new key(profileJSON["keys"][i]["Z"], 
						x + globalOffsetX, y + globalOffsetY,
						profileJSON["keys"][i]["W"], profileJSON["keys"][i]["H"],
						r, g, b, a, profileJSON["keys"][i]["text"],	profileJSON["keys"][i]["font"],
						profileJSON["keys"][i]["isRect"], profileJSON["keys"][i]["useImage"], 
						profileJSON["keys"][i]["affectedByCapsLock"], 
						kShader, kPressShader, 
						getOutputType(profileJSON["keys"][i]["outputType"]), profileJSON["keys"][i]["outputValue"],
						modText, modShader, modOutputTypes, modOutputValue,
						profileJSON["keys"][i]["pluginEvent"]);
		keys.push_back(k);					
	}
	
	//this is included here as a work around for crashing when keyboards have different number of keys
	while (keys.size() < 110){	
		string modText[numberOfModifiers] = { "", "", "", "", "" };
		int modShader[numberOfModifiers] = { -1, -1, -1, -1, -1 };
		int modOutputTypes[numberOfModifiers] = { 0, 0, 0, 0, 0 };
		string modOutputValue[numberOfModifiers] = { "", "", "", "", "" };
		key* dummyKey = new key(0, 0, 0, 0, 0, 
								0, 0, 0, 0, 
							"", -1, true, false, false, -1, -1, 0, "", 
			modText, modShader, modOutputTypes, modOutputValue, -1);
		keys.push_back(dummyKey);
	}
	
	//importing plugins
	string pluginPath = "profiles/" + layoutName + "/plugins/";	//plugin path
	plugins.clear();	//clear all old plugins	
	int numberOfPlugins = profileJSON["plugins"].size();
	for (int i = 0; i < numberOfPlugins; i++){
		if (loadPlugin(pluginPath, profileJSON["plugins"][i])){
			if (plugins[plugins.size() - 1].onLoad){
				plugins[plugins.size() - 1].onLoad(keys);	//run load function
			}
		}
		else{
			cerr << "Could not load plugin: " << profileJSON["plugins"][i] << endl;
		}
	}
	
	//reset modifier key vectors
	leftShiftKeys.clear();
	leftCtrlKeys.clear();
	leftAltKeys.clear();
	leftUIKeys.clear();
	rightShiftKeys.clear();
	rightCtrlKeys.clear();
	rightAltKeys.clear();
	rightUIKeys.clear();
	FnKeys.clear();
	capsKeys.clear();
	
	//scan for modifier keys:
	for (int i = 0; i < keys.size(); i++){
		if (keys[i]->outputType == HIDOUTPUT){
			if (keys[i]->HIDcode == KC_LEFT_SHIFT){
				leftShiftKeys.push_back(i);
				//cout << "Left Shift: " << i << endl;
				continue;	
			}
			if (keys[i]->HIDcode == KC_LEFT_CTRL){
				leftCtrlKeys.push_back(i);
				//cout << "Left Ctrl: " << i << endl;
				continue;	
			}
			if (keys[i]->HIDcode == KC_LEFT_ALT){
				leftAltKeys.push_back(i);
				//cout << "Left Alt: " << i << endl;
				continue;	
			}
			if (keys[i]->HIDcode == KC_LEFT_GUI){
				leftUIKeys.push_back(i);
				//cout << "Left GUI: " << i << endl;
				continue;	
			}
			if (keys[i]->HIDcode == KC_RIGHT_SHIFT){
				rightShiftKeys.push_back(i);
				//cout << "Right Shift: " << i << endl;
				continue;	
			}
			if (keys[i]->HIDcode == KC_RIGHT_CTRL){
				rightCtrlKeys.push_back(i);
				//cout << "Right Ctrl: " << i << endl;
				continue;	
			}
			if (keys[i]->HIDcode == KC_RIGHT_ALT){
				rightAltKeys.push_back(i);
				//cout << "Right Alt: " << i << endl;
				continue;	
			}
			if (keys[i]->HIDcode == KC_RIGHT_GUI){
				rightUIKeys.push_back(i);
				//cout << "Right GUI: " << i << endl;
				continue;	
			}
			if (keys[i]->HIDcode == KC_FN){
				FnKeys.push_back(i);
				//cout << "Fn: " << i << endl;
				continue;	
			}
			if (keys[i]->HIDcode == KC_CAPS_LOCK){
				capsKeys.push_back(i);
				//cout << "Caps: " << i << endl;
			}
		}
	}	
	//give to graphics:
	keyboardWindow->setModifierKeys(leftShiftKeys, leftCtrlKeys, leftAltKeys, leftUIKeys,
									rightShiftKeys, rightCtrlKeys, rightAltKeys, rightUIKeys, FnKeys, capsKeys);
	
	//set keyboard UI graphics:
	for (int i = 0; i < keys.size(); i++){		
		int heldTypeModifier = 0;
		if (keys[i]->useImage || keys[i]->text.length() > 0){	//texture via image or text
			for (int j = 0; j < numberOfModifiers; j++){
				if (keys[i]->modText[j].length() > 0){
					heldTypeModifier |= modifierBit[j];
				}
			}
			for (int j = 0; j < numberOfModifiers; j++){
				if (keys[i]->modText[j].length() > 0){
					int newMod = ((heldTypeModifier - modifierBit[j]) << 16) + modifierBit[j];
					textureKey(i, keys[i]->useImage, keys[i]->modText[j], newMod, path, keys[i]->modShader[j]);
				}
			}
			heldTypeModifier = ((0x1FF ^ heldTypeModifier) & 0x1FF) + FORNORMALTEXTURE;
			if (heldTypeModifier == 0x1FF + FORNORMALTEXTURE) heldTypeModifier = -1;
			textureKey(i, keys[i]->useImage, keys[i]->text, heldTypeModifier, path);
		}
		else if (keys[i]->isRect){	//is a rectangle
			rectKey(i);
		}
		else if (!keys[i]->isRect){	//is a circle
			circleKey(i, keys[i]->w / 2, CIRCLESEGMENTS);
		}
	}
	
	//set up free objects
	int numberOfFreeObj = profileJSON["freeObj"].size();
	for (int i = 0; i < numberOfFreeObj; i++){
		int freeObjectType = profileJSON["freeObj"][i]["freeType"];
		parseRGBAfromHex(profileJSON["freeObj"][i]["RGBA"], &r, &g, &b, &a);
		int rad = profileJSON["freeObj"][i]["W"];
		rad /= 2;
		
		int shader = profileJSON["freeObj"][i]["shader"];
		if (shader >= numberOfCustomShaders){ 
			shader = -1;
			cout << "Custom Shader for object " << i << " out of range" << endl;
		}
		
		switch (freeObjectType){
			case 0:		//freeRect
				freeRect(profileJSON["freeObj"][i]["Z"], profileJSON["freeObj"][i]["X"], profileJSON["freeObj"][i]["Y"],
				profileJSON["freeObj"][i]["W"], profileJSON["freeObj"][i]["H"], r, g, b, a, shader);
				break;
			case 1:		//free circle
				freeCircle(profileJSON["freeObj"][i]["Z"], profileJSON["freeObj"][i]["X"], profileJSON["freeObj"][i]["Y"],
				rad, CIRCLESEGMENTS, r, g, b, a, shader);
				break;
			case 2:		//freeImage
				freeTexture(profileJSON["freeObj"][i]["Z"], profileJSON["freeObj"][i]["X"], profileJSON["freeObj"][i]["Y"], 
				profileJSON["freeObj"][i]["W"], profileJSON["freeObj"][i]["H"], profileJSON["freeObj"][i]["text"], 
				true, r, g, b, a, path, shader);
				break;
			case 3:		//freeText
				freeTexture(profileJSON["freeObj"][i]["Z"], profileJSON["freeObj"][i]["X"], profileJSON["freeObj"][i]["Y"], 
				profileJSON["freeObj"][i]["W"], profileJSON["freeObj"][i]["H"], profileJSON["freeObj"][i]["text"], 
				false, r, g, b, a, path, profileJSON["freeObj"][i]["font"], shader);
				break;
				
			default:
				cout << "Unknown object type in \"freeObj\" section of layout.prof" << endl;
		}
	}	
	
	//unlock graphics:
	keyboardWindow->setGraphicsLock(false);
	//profile loaded sucessfully
	return 0;
}


bool keyboard::loadPlugin(std::string pluginPath, std::string pluginName){
	void* handle = dlopen((pluginPath + "/" + pluginName).c_str(), RTLD_LAZY);
	if (!handle) {
		return false;
	}
	customPlugin p;
	p.handle = handle;
	p.onLoad = (int(*)(vector<key*>)) dlsym(handle, "onLoad");
	p.onPress = (int(*)(vector<key*>)) dlsym(handle, "onPress");	
	plugins.push_back(p);	//add to the plugins vector
	return true;
}

int keyboard::textureKey(int keyID, int layoutType, string layoutString, int modifiers, string profilePath, int customShader){
	if (keys[keyID]->useImage){
		profilePath += "/images/";
		//int drawImage(const char* src, int x, int y, int width, int height, int a);
		if (inMainThread)
			keyboardWindow->drawImage((profilePath + layoutString).c_str(), keys[keyID]->z, 
						keys[keyID]->x, keys[keyID]->y, keys[keyID]->w, keys[keyID]->h, 
						keys[keyID]->a, keyID, modifiers, customShader);
		else{
			keyboardWindow->requestDrawImage((profilePath + layoutString).c_str(), keys[keyID]->z, 
						keys[keyID]->x, keys[keyID]->y, keys[keyID]->w, keys[keyID]->h, 
						keys[keyID]->a, keyID, modifiers, customShader);
			while (!keyboardWindow->asyncFunctionCompleted())
				__asm__("nop");
		}
	}
	else {	//draw text
		if (profileFonts.size() == 0){
			printf("No fonts loaded for this profile\n");
			return -1;
		}
		if (profileFonts.size() <= keys[keyID]->font){
			printf("Font out of range: %d. Fonts loaded only: %d\n", keys[keyID]->font, static_cast<int>(profileFonts.size()));
			return -1;
		}
		//set font to key's font:
		assignKeyboardFont(profileFonts[keys[keyID]->font].c_str());
		//int drawText(const char* text, int x, int y, int width, int height, int r, int g, int b, int a);
		if (inMainThread)
			keyboardWindow->drawText(layoutString.c_str(), keys[keyID]->z, 
						keys[keyID]->x, keys[keyID]->y, keys[keyID]->w, keys[keyID]->h, 
						keys[keyID]->r, keys[keyID]->g, keys[keyID]->b, keys[keyID]->a,
						keyID, modifiers, customShader);
		else {
			keyboardWindow->requestDrawText(layoutString.c_str(), keys[keyID]->z, 
						keys[keyID]->x, keys[keyID]->y, keys[keyID]->w, keys[keyID]->h, 
						keys[keyID]->r, keys[keyID]->g, keys[keyID]->b, keys[keyID]->a,
						keyID, modifiers, customShader);
			while (!keyboardWindow->asyncFunctionCompleted())
				__asm__("nop");
		}
	}
	return 0;
}

int keyboard::rectKey(int keyID){
	if (inMainThread)
		return keyboardWindow->drawRect(keys[keyID]->z, keys[keyID]->x, keys[keyID]->y, 
				keys[keyID]->w, keys[keyID]->h, 
				keys[keyID]->r, keys[keyID]->g, keys[keyID]->b, keys[keyID]->a, keyID, keys[keyID]->shader);
	//else request async
	keyboardWindow->requestDrawRect(keys[keyID]->z, keys[keyID]->x, keys[keyID]->y, 
			keys[keyID]->w, keys[keyID]->h, 
			keys[keyID]->r, keys[keyID]->g, keys[keyID]->b, keys[keyID]->a, keyID, keys[keyID]->shader);
	while (!keyboardWindow->asyncFunctionCompleted())
		__asm__("nop");

	return keyboardWindow->asyncOutput();
}

int keyboard::circleKey(int keyID, int radius, int segments){
	if (inMainThread)
		return keyboardWindow->drawCircle(keys[keyID]->z, keys[keyID]->x + (keys[keyID]->w / 2), 
					keys[keyID]->y + (keys[keyID]->h / 2), radius, segments,
					keys[keyID]->r, keys[keyID]->g, keys[keyID]->b, keys[keyID]->a, keyID, keys[keyID]->shader);
	//else request async
	keyboardWindow->requestDrawCircle(keys[keyID]->z, keys[keyID]->x + (keys[keyID]->w / 2), 
					keys[keyID]->y + (keys[keyID]->h / 2), radius, segments,
					keys[keyID]->r, keys[keyID]->g, keys[keyID]->b, keys[keyID]->a, keyID, keys[keyID]->shader);
	while (!keyboardWindow->asyncFunctionCompleted())
		__asm__("nop");
	return keyboardWindow->asyncOutput();
}

int keyboard::assignKeyboardFont(const char* fontname){
	return keyboardWindow->setFont(fontname);
}

int keyboard::setKeyboardBackgroundColor(int r, int g, int b, int a){
	if (r < 256 && g < 256 && b < 256 && a < 256)
		keyboardWindow->setbackrgoundColor(r, g, b, a);
	else {
		SDL_Log("Color value over 255 detected");
		return -1;
	}
	return 0;
}

int keyboard::assignTextColor(int r, int g, int b, int a){
	if (r < 256 && g < 256 && b < 256 && a < 256)
		textColor = {r, g, b, a};
	else {
		SDL_Log("Color value over 255 detected");
		return -1;
	}
	return 0;
}

bool keyboard::parseRGBAfromHex(const string& RGBA, int* r, int* g, int* b, int* a){
	string hex = (RGBA[0] == '#') ? RGBA.substr(1) : RGBA;
	if (hex.length() != 8)
		//incorrect string length
		return false;
	
	*r = stoi(hex.substr(0, 2), nullptr, 16);
	*g = stoi(hex.substr(2, 2), nullptr, 16);
	*b = stoi(hex.substr(4, 2), nullptr, 16);
	*a = stoi(hex.substr(6, 2), nullptr, 16);
	return true;
}


int keyboard::freeRect(int z, int x, int y, int w, int h, int r, int g, int b, int a, int customShader){
	if (inMainThread)
		return keyboardWindow->drawRect(z, x, y, w, h, r, g, b, a, customShader=customShader);
		//else request async
	keyboardWindow->requestDrawRect(z, x, y, w, h, r, g, b, a, customShader=customShader);
	while (!keyboardWindow->asyncFunctionCompleted())
		__asm__("nop");
	return keyboardWindow->asyncOutput();
}

int keyboard::freeCircle(int z, int x, int y, int radius, int segments, int r, int g, int b, int a, int customShader){
	if (inMainThread)
		return keyboardWindow->drawCircle(z, x, y, radius, segments, r, g, b, a, customShader=customShader);
	//else request async
	keyboardWindow->requestDrawCircle(z, x, y, radius, segments, r, g, b, a, customShader=customShader);
	while (!keyboardWindow->asyncFunctionCompleted())
		__asm__("nop");
	return keyboardWindow->asyncOutput();
}

int keyboard::freeTexture(int z, int x, int y, int w, int h, string layoutText, bool isImage, int r, int g, int b, int a, string profilePath, int font, int customShader){
	if (isImage){
		//int drawImage(const char* src, int x, int y, int width, int height, int a);
		if (inMainThread)
			keyboardWindow->drawImage((profilePath + layoutText).c_str(), z, x, y, w, h, a, customShader=customShader);
		else{
			keyboardWindow->requestDrawImage((profilePath + layoutText).c_str(), z, x, y, w, h, a, customShader=customShader);
			while (!keyboardWindow->asyncFunctionCompleted())
				__asm__("nop");
		}
	}
	else {	//draw text
		if (profileFonts.size() == 0){
			printf("No fonts loaded for this profile\n");
			return -1;
		}
		if (profileFonts.size() <= font){
			printf("Font out of range: %d. Fonts loaded only: %d\n", font, static_cast<int>(profileFonts.size()));
			return -1;
		}
		//set font to key's font:
		assignKeyboardFont(profileFonts[font].c_str());
		//int drawText(const char* text, int x, int y, int width, int height, int r, int g, int b, int a);
		if (inMainThread)
			keyboardWindow->drawText(layoutText.c_str(), z, x, y, w, h, r, g, b, a, customShader=customShader);
		else {
			keyboardWindow->requestDrawText(layoutText.c_str(), z, x, y, w, h, r, g, b, a, customShader=customShader);
			while (!keyboardWindow->asyncFunctionCompleted())
				__asm__("nop");
		}
	}
	return 0;
}

int keyboard::freeDebugText(int x, int y, const char* toWrite, float size, int r, int g, int b, int a){
	if (keyboardWindow->writeText(toWrite, x, y, size, r, g, b, a) == nullptr)
		return -1;
	return 0;
}
	
int keyboard::setKeyboardBGImg(const char* src){
	if (inMainThread)
		return keyboardWindow->setbackgroundImage(src);
	//else request async
	keyboardWindow->requestSetbackgroundImage(src);
	while (!keyboardWindow->asyncFunctionCompleted())
		__asm__("nop");
	return keyboardWindow->asyncOutput();	
}

int keyboard::loadVideo(const char* filename, int x, int y, int w, int h){
	return keyboardWindow->loadVideo(filename, x, y, w, h);
}

int keyboard::loadCustomShader(const char* vertexShader, const char* fragmentShader){
	if (inMainThread)
		return keyboardWindow->loadCustomShaderProgram(vertexShader, fragmentShader);
	//else request async
	keyboardWindow->requestCustomShader(vertexShader, fragmentShader);
	while (!keyboardWindow->asyncFunctionCompleted())
		__asm__("nop");
	return keyboardWindow->asyncOutput();
}

int keyboard::preextractProfiles(){
	//search for .zip files in /profiles
	filesystem::path profileDir = "profiles";
	
	//if no directory of same name, extract files (extraction skipped if dir found)
	for (auto& entry : filesystem::directory_iterator(profileDir)){
		if (entry.is_regular_file() && entry.path().extension() == ".zip"){
			string zipName = entry.path().stem().string();	//no .zip portion on string
			filesystem::path extractionDir = profileDir / zipName;
			
			if (!filesystem::exists(extractionDir)){	//skips if already exists
				cout << "Extracting: " << entry.path().filename() << endl;
				
				//use system unzip command
				string command = "unzip -q \"" + entry.path().string() + "\" -d \"" + extractionDir.string() + "\"";
				int result = system(command.c_str());
				
				if (result != 0){
					cerr << "Failed to extract " << entry.path().filename() << endl;
					return -1;
				}
				else{
					cout << "Extraction of " << entry.path().filename() << " sucessful" << endl;
				}
			}
			
		}
	}
	return 0;
}

//hall effect sensors -------------------------------------------------------------------
void keyboard::runHallEffectSensors(bool* on){	
	/*
	//create and initailize new hallEffectReader class
	hallEffectReader* hallEffectKeyboard = new hallEffectReader();
	hallEffectKeyboard->initSPI(SPIchannel, SPIspeed, CSp0, CSp1, CSp2, CSp3);
	
	//initialize all to zero
	for (int i = 0; i < MAXKEYS; i++){
		for (int j = 0; j < avgSize; j++){
			keyVals[i][j] = 0;
		}
		keySignal[i] = 0;
	}
	
	int index = 0;
	int globalAverage = 0;
	
	Uint32 lastTime = SDL_GetTicks();
	Uint32 currentTime = SDL_GetTicks();
	
	while(*on){
		//poll:
		for (int i = 0; i < MAXKEYS; i++){
			keyVals[i][index] = hallEffectKeyboard->readHallEffectSensor(i);
			//cout << "Key " << i << ": " << keyVals[i][index] << endl;
			keySignal[i] = 0;
			//calculate average:
			for (int j = 0; j < avgSize; j++){
				keySignal[i] += keyVals[i][j];
			}
			keySignal[i] /= avgSize;
			if (i != 0 && debugTextOn)
			debugText[i+1]->updateText(to_string(keySignal[i]).c_str());	//0th debug is FPS, so increase by 1
		}
		//determining global average:
		globalAverage = 0;
		for (int i = 0; i < MAXKEYS; i++){
			globalAverage += keySignal[i];
		}
		globalAverage /= MAXKEYS;
		if (debugTextOn)
		debugText[1]->updateText(to_string(globalAverage).c_str());	//0th debug is FPS, so increase by 1
		
		//checking which are pressed:
		for (int i = 0; i < MAXKEYS; i++){
			if (keySignal[i] < globalAverage - 35){
				if ((i == 0 || i == 5 || i == 1 || i == 24 || i == 31 || i == 54 || i == 57 || i == 58 || i == 59 || i == 66 || i == 28 || i == 43) && keySignal[i] >= globalAverage - 80)	//these two keys have sensitivity issues
					pressedKeys[i] = false;
				else 
					pressedKeys[i] = true;
			}
			else
				pressedKeys[i] = false;
		}
		
		//waiting:
		/*
		currentTime = SDL_GetTicks();
		while (currentTime - lastTime < 1){	//
			currentTime = SDL_GetTicks();
		}
		lastTime = currentTime;
		* /
		
		//increase index:
		index++;
		if (index >= avgSize){
			index = 0;
		}
		//mark as pressed:
		//pressedKeys[i] = true;
	}
	*/
	return;
}