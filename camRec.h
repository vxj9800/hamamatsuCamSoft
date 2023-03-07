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

void startCamRecording(std::ostream& out, HDCAM hdcam, camRecInfo recInfo);