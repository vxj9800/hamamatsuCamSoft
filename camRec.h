#pragma once

#define NOMINMAX
#include "dcamMisc/console4.h"

#include <iostream>
#include <filesystem>
#include <fstream>

struct camRecInfo
{
	std::filesystem::path filePath;
	unsigned int nFrames = 0;
};

// Variable to indicate whether startCamRecording is currently executing
extern std::atomic<bool> camRcrdng;

// Variable to indicate whether the camera is ready to capture frames
extern std::atomic<bool> camRdy2Capt;

void startCamRecording(std::ostream& out, HDCAM hdcam, camRecInfo recInfo);
void waitFinishCamRcrdng(std::ostream &out);