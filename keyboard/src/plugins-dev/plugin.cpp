//plugin template
#include "pluginInterface.h"
#include <iostream>

using namespace std;

extern "C" int onLoad(vector<key*>& keys) {
	cout << "Plugin 1 Loaded" << endl;
	return 0;
}

extern "C" int onPress(vector<key*>& keys) {
	cout << "Plugin 1 press event" << endl;
	return 0;
}

