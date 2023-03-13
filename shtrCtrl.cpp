#include "shtrCtrl.h"

serialib shtrPort; // Serial connection to the uC that controls the shutter and camera trigger

// Blocking function, if timeput_ms = 0, that returns with a character when available.
// Otherwise the number would be negative if no character is received within timeout.
int getcShtr(unsigned int timeout_ms)
{
    char c;
    int retCode = 0;
    if (timeout_ms)
        retCode = shtrPort.readChar(&c,timeout_ms);
    else
        while ((retCode = shtrPort.readChar(&c, 0)) < 1);
    if (retCode == 1)
        return c;
    else
        return -2;
}

// Blocking function, if timeput_ms = 0, that reads a string when available.
// The timeout_ms value is used for each character.
bool getsShtr(char *s, unsigned int timeout_ms)
{
    uint8_t count = 0;
    int c;

    while (count < MAX_CHARS)
    {
        c = getcShtr(timeout_ms);
        if (c == 8 || c == 127)
        {
            if (count == 0)
                continue;
            count--;
        }
        else if (c == 10 || c == 13) // Putty generally sends '\r' on Enter key
        {
            s[count] = '\0'; // Add Null character
            c = getcShtr(1000);            // Flush out the expected '\n' character if received within 1ms
            break;
        }
        else if (c > 31 && c < 127)
            s[count++] = c;
        else if (!timeout_ms)
            continue;
        else
            return 0;
    }
    return 1;
}

void flshShtrCommBuffer()
{
    shtrPort.flushReceiver();
}

bool initShtr()
{
    // Look for the ports
    std::string devName;
    bool correctToken = false;
    char token[MAX_CHARS];

    for (int i = 1; i < 99; i++)
    {
        // Prepare the port name (Windows)
        devName = "\\\\.\\COM" + std::to_string(i);

        // try to connect to the device
        if (shtrPort.openDevice(devName.c_str(), 115200) == 1)
        {
            // Set DTR and unset RTS
            shtrPort.DTR(true);
            shtrPort.RTS(false);

            // Read string from the port
            shtrPort.flushReceiver();
            for (size_t j = 0; j < 5; ++j)
            {
                int nBytes = getsShtr(token,200);
                correctToken = !strcmp(token, recToken);
                if (correctToken)
                    break;
            }

            if (correctToken)
            {
                shtrPort.writeString(sndToken);
                break;
            }
            else
                shtrPort.closeDevice();
        }
    }
    if (correctToken)
        std::cout << "Shutter controller detected on " << devName << "." << std::endl;
    else
        std::cout << "No shutter controller was detected." << std::endl;

    return correctToken;
}

void sndShtrCmd(std::ostream &out, std::string& s)
{
    // Flush the input buffer
    shtrPort.flushReceiver();

    // Append CR/LF combo to the string
    s = s + "\r\n";

    // Send the command
    putsShtr(s.c_str());

    // Receive the response
    char response[MAX_CHARS];
    while (getsShtr(response))
    {
        if (!strcmp(response,endRspToken))
            break;
        else
            out << std::string(response) << std::endl;
    }
}

std::string sndCmdRecRspShtr(std::string& s)
{
    // Flush the input buffer
    shtrPort.flushReceiver();

    // Append CR/LF combo to the string
    s = s + "\r\n";

    // Send the command
    putsShtr(s.c_str());

    // Receive the response and append it to a string
    char response[MAX_CHARS];
    std::string rsp;
    while (getsShtr(response))
    {
        if (!strcmp(response, endRspToken))
            break;
        else
            rsp += response;
    }

    return rsp;
}

void deinitShtr(std::ostream &out)
{
    sndShtrCmd(out, std::string("++reboot"));
    shtrPort.closeDevice();
}