// camSoft.cpp : Defines the entry point for the application.
//

#include "camSoft.h"

HDCAM hdcam = NULL;
std::vector<camPropInfo> camProps; // Array to store camera properties
size_t nCamProps = 0; // Number of properties that the camera has

bool initCam()
{
	std::cout << "Looking for Hamamatsu C11440-22C camera." << std::endl;
	hdcam = dcamcon_init_open(); // Unitialize DCAM-API and open device
	if (hdcam != NULL)
		std::cout << "Camera initialization sucessful." << std::endl;
	else
		dcamapi_uninit(); // Uninitialize DCAM-API
	return (hdcam != NULL);
}

void otSoftCamRec(std::ostream& out, unsigned int nFrames, std::filesystem::path filePath)
{
	filePath.make_preferred();
	if (!liveCapOn)
		if (filePath.extension() == ".bin")
		{
			if (filePath.parent_path() == "")
			{
				camRecInfo recInfo;
				recInfo.nFrames = nFrames;
				recInfo.filePath = filePath.filename();
				startCamRecording(out, hdcam, recInfo);
			}
			else if (std::filesystem::is_directory(filePath.parent_path()) || std::filesystem::create_directories(filePath.parent_path()))
			{
				camRecInfo recInfo;
				recInfo.nFrames = nFrames;
				recInfo.filePath = filePath.parent_path() /= filePath.filename();
				startCamRecording(out, hdcam, recInfo);
			}
			else
				out << "Failed to create the path specified." << std::endl;
		}
		else
			out << "Recording file name must contain \".bin\" extension." << std::endl;
	else
		out << "Live feed must be turned off before recording." << std::endl;
}

void camPropsUpdate(std::ostream& out)
{
	fillCamProps(hdcam, camProps, nCamProps);
}

void camPropList(std::ostream& out)
{
	printCamPropsArray(out, camProps, nCamProps);
}

void camPropInfoByName(std::ostream& out, std::string propName)
{
	int propListIdx = getCamPropsIdxByName(propName, camProps, nCamProps);
	if (propListIdx >= 0)
		printCamPropInfo(out, propListIdx, camProps, nCamProps);
	else
		out << "Property with name \"" << propName << "\" doesn't exist." << std::endl;
}

void camPropInfoByID(std::ostream& out, int32 propID)
{
	int propListIdx = getCamPropsIdxByID(propID, camProps, nCamProps);
	if (propListIdx >= 0)
		printCamPropInfo(out, propListIdx, camProps, nCamProps);
	else
		out << "Property with ID \"" << propID << "\" doesn't exist." << std::endl;
}

void camPropSetByName(std::ostream& out, std::string propName, double val)
{
	if (liveCapOn)
		out << "Camera properties cannot be set while camera is recording or capturing." << std::endl;
	else
	{
		int propListIdx = getCamPropsIdxByName(propName, camProps, nCamProps);
		if (propListIdx >= 0)
			setCamPropValue(hdcam, out, propListIdx, val, camProps, nCamProps);
		else
			out << "Property with name \"" << propName << "\" doesn't exist." << std::endl;
	}
}

void camPropSetByID(std::ostream& out, int32 propID, double val)
{
	if (liveCapOn)
		out << "Camera properties cannot be set while camera is recording or capturing." << std::endl;
	else
	{
		int propListIdx = getCamPropsIdxByID(propID, camProps, nCamProps);
		if (propListIdx >= 0)
			setCamPropValue(hdcam, out, propListIdx, val, camProps, nCamProps);
		else
			out << "Property with ID \"" << propID << "\" doesn't exist." << std::endl;
	}
}

void liveCapStart(std::ostream& out)
{
	if (liveCapOn)
		out << "Live feed from camera is already on." << std::endl;
	else
	{
		int propListIdx = getCamPropsIdxByID(DCAM_IDPROP_IMAGE_WIDTH, camProps, nCamProps);
		liveCapImgWidth = (GLsizei)camProps[propListIdx].currentVal;
		propListIdx = getCamPropsIdxByID(DCAM_IDPROP_IMAGE_HEIGHT, camProps, nCamProps);
		liveCapImgHeight = (GLsizei)camProps[propListIdx].currentVal;
		liveCapOn = true;
		std::thread liveCapThread(startCamCap, std::ref(out), hdcam);
		liveCapThread.detach();
	}
}

void liveCapStop(std::ostream& out)
{
	if (!liveCapOn)
		out << "Live feed from camera is already off." << std::endl;
	else
	{
		liveCapOn = false;
		liveCapLutMin = 0.0f;
		liveCapLutMax = 65535.0f;
	}
}

void liveCapLUT(std::ostream& out, int lutMin, int lutMax)
{
	if (lutMax > 65535 || lutMax <= lutMin || lutMin < 0)
		out << "Make sure that lutMin >= 0, lutMax <= 65535 and lutMax > lutMin." << std::endl;
	else
	{
		liveCapLutMin = (float)lutMin;
		liveCapLutMax = (float)lutMax;
	}
}

int main(int argc, char* const argv[])
{
	// Roll the intro
	std::cout << "///////////////////////////////////////////////////////////////////////////" << std::endl;
	std::cout << "// otSoft: A Command Line Program to perform optical tweezer experiments //" << std::endl;
	std::cout << "// Developed by: Vatsal Asitkumar Joshi                                  //" << std::endl;
	std::cout << "// Hardware required: Hamamatsu C11440-22C Camera,                       //" << std::endl;
	std::cout << "//                    Raspberry Pi Pico Microcontroller,                 //" << std::endl;
	std::cout << "//                    1064nm Laser                                       //" << std::endl;
	std::cout << "// Date last updated: 12/14/2022                                         //" << std::endl;
	std::cout << "///////////////////////////////////////////////////////////////////////////" << std::endl << std::endl;

	if (initCam())
	{
		fillCamProps(hdcam, camProps, nCamProps); // Get a list of camera properties
		std::cout << std::endl;

		// Create a root menu of our cli
		auto otSoft = std::make_unique<cli::Menu>("otSoft", "Main menu of this application.");
		otSoft->Insert("camRec", otSoftCamRec, "Start camera recording.");

		// Create a submenu for camera properties
		auto camProp = std::make_unique<cli::Menu>("camProp", "Menu to access camera properties.");
		camProp->Insert("update", camPropsUpdate, "Update the list of all the properties of Hamamatsu C11440-22C camera.");
		camProp->Insert("list", camPropList, "List all the properties of Hamamatsu C11440-22C camera.");
		camProp->Insert("infoByName", camPropInfoByName, "Get info regarding a certain property by name.");
		camProp->Insert("infoByID", camPropInfoByID, "Get info regarding a certain property by ID.");
		camProp->Insert("setByName", camPropSetByName, "Set camera property value.");
		camProp->Insert("setByID", camPropSetByID, "Set camera property value.");
		otSoft->Insert(std::move(camProp));

		// Create a submenu for camera live feed
		auto liveCap = std::make_unique<cli::Menu>("liveCap", "Menu to show live feed from the camera.");
		liveCap->Insert("start", liveCapStart, "Start the camera live feed.");
		liveCap->Insert("stop", liveCapStop, "Stop the camera live feed.");
		liveCap->Insert("lut", liveCapLUT, "Update input-output mapping of the camera pixel values.");
		otSoft->Insert(std::move(liveCap));

		// create the cli with the root menu
		cli::Cli cli(std::move(otSoft));

		// global exit action
		cli.ExitAction([](auto& out) {
			liveCapOn = false; // Make sure the camera is not capturing
			dcamdev_close(hdcam); // close DCAM handle
			dcamapi_uninit(); // Uninit DCAM-API
			out << "Camera uninitialized properly." << std::endl;

		});

		cli::CliFileSession input(cli);
		input.Start();
	}
	else
		std::cout << "Camera initialization unsucessful. Try again later." << std::endl;
}