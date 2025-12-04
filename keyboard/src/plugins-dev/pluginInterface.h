#pragma once
#include "../key.h"
#include <vector>

extern "C" {
	int onLoad(std::vector<key*>& keys);
	int onPress(std::vector<key*>& keys);
}