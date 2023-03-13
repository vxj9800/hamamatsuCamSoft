#pragma once

#define NOMINMAX
#include "dcamMisc/console4.h"
#include <limits>
#include <iomanip>
#include <vector>

constexpr int propNameSize = 40;
constexpr int attrNameSize = 24;
constexpr int datatypeNameSize = 8;
constexpr int suppValNameSize = 24;
constexpr int unitNameSize = 16;

typedef struct _camPropInfo
{
	int32 propID = 0;
	DCAMPROP_ATTR propAttr;
	char propName[propNameSize];
	int nAttr = 0;
	char attrNames[16][attrNameSize];
	char dataType[datatypeNameSize];
	int nSuppVals = -1;
	int32 suppVals[32];
	char suppValNames[32][suppValNameSize];
	char unit[unitNameSize];
	double min = std::numeric_limits<double>::quiet_NaN();
	double max = std::numeric_limits<double>::quiet_NaN();
	double step = std::numeric_limits<double>::quiet_NaN();
	double defaultVal = std::numeric_limits<double>::quiet_NaN();
	double currentVal = std::numeric_limits<double>::quiet_NaN();
} camPropInfo;

extern std::vector<camPropInfo> camProps; // Array to store camera properties

camPropInfo get_camPropInfo(HDCAM hdcam, int32 propID);
void fillCamProps(HDCAM hdcam);
void printCamPropsArray(std::ostream& out);
void printCamPropInfo(std::ostream& out, size_t camPropsIdx);
int getCamPropsIdxByName(std::string& propName);
int getCamPropsIdxByID(int32 propID);
void setCamPropValue(HDCAM hdcam, std::ostream& out, size_t camPropsIdx, double val);