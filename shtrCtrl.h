#pragma once

#define NOMINMAX
#include <iostream>
#include <string>
#include "serialib/lib/serialib.h"

#define MAX_CHARS 80

// Define tokens used for communication
#define sndToken "116119101101122101114083111102116119097114101\r\n"    // Each 3 digits converted to chars results in 'tweezerSoftware'
#define recToken "115104117116116101114067111110116114111108108101114"  // Each 3 digits converted to chars results in 'shutterController'
#define endRspToken "101110100067109100082115112"                       // Each 3 digits converted to chars results in 'endCmdRsp'

extern serialib shtrPort; // Serial connection to the uC that controls the shutter and camera trigger

#define putcShtr(c) shtrPort.writeChar(c)   // Blocking function that writes a character
#define putsShtr(s) shtrPort.writeString(s) // Blocking function that writes a string

void flshShtrCommBuffer();
int getcShtr(unsigned int timeout_ms = 0);
bool getsShtr(char *s, unsigned int timeout_ms = 0);

bool initShtr();
void sndShtrCmd(std::ostream &out, std::string& s);
std::string sndCmdRecRspShtr(std::string &s);
void deinitShtr(std::ostream &out);