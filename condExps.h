#pragma once

#define NOMINMAX
#include <iostream>
#include <ctime>
#include <thread>
#include "camProp.h"
#include "shtrCtrl.h"
#include "camRec.h"

extern std::atomic<bool> expsUnderProgress; // Whether the experiments are currently being conducted or not

void startConductingExps(std::ostream &out, size_t numExps, HDCAM hdcam);
void stopConductingExps(std::ostream &out);
void expsStatus(std::ostream &out);