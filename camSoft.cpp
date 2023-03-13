// camSoft.cpp : Defines the entry point for the application.
//

#include "camSoft.h"

HDCAM hdcam = NULL;

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
	if (liveCapOn)
		out << "Live feed must be turned off before recording." << std::endl;
	else if (expsUnderProgress)
		out << "The ongoing experiments must be stopped before recording is started." << std::endl;
	else if (camRcrdng)
		out << "The camera is already recording something." << std::endl;
	else
	{
		if (filePath.extension() == ".bin")
		{
			camRecInfo recInfo;
			if (filePath.parent_path() == "")
			{
				recInfo.nFrames = nFrames;
				recInfo.filePath = filePath.filename();
				std::thread camRecThread(startCamRecording, std::ref(out), hdcam, recInfo);
				camRecThread.detach();
			}
			else if (std::filesystem::is_directory(filePath.parent_path()) || std::filesystem::create_directories(filePath.parent_path()))
			{
				recInfo.nFrames = nFrames;
				recInfo.filePath = filePath.parent_path() /= filePath.filename();
				std::thread camRecThread(startCamRecording, std::ref(out), hdcam, recInfo);
				camRecThread.detach();
			}
			else
				out << "Failed to create the path specified." << std::endl;
		}
		else
			out << "Recording file name must contain \".bin\" extension." << std::endl;
	}
}

void otSoftShtrCtrl(std::ostream& out, std::string cmd)
{
	sndShtrCmd(out, cmd);
}

void camPropsUpdate(std::ostream& out)
{
	fillCamProps(hdcam);
}

void camPropList(std::ostream& out)
{
	fillCamProps(hdcam);
	printCamPropsArray(out);
}

void camPropInfoByName(std::ostream& out, std::string propName)
{
	fillCamProps(hdcam);
	int propListIdx = getCamPropsIdxByName(propName);
	if (propListIdx >= 0)
		printCamPropInfo(out, propListIdx);
	else
		out << "Property with name \"" << propName << "\" doesn't exist." << std::endl;
}

void camPropInfoByID(std::ostream& out, int32 propID)
{
	fillCamProps(hdcam);
	int propListIdx = getCamPropsIdxByID(propID);
	if (propListIdx >= 0)
		printCamPropInfo(out, propListIdx);
	else
		out << "Property with ID \"" << propID << "\" doesn't exist." << std::endl;
}

void camPropSetByName(std::ostream& out, std::string propName, double val)
{
	if (liveCapOn || camRcrdng || expsUnderProgress)
		out << "Camera properties cannot be set while live feed is on, camera is recording or experiments are being conducted." << std::endl;
	else
	{
		fillCamProps(hdcam);
		int propListIdx = getCamPropsIdxByName(propName);
		if (propListIdx >= 0)
			setCamPropValue(hdcam, out, propListIdx, val);
		else
			out << "Property with name \"" << propName << "\" doesn't exist." << std::endl;
	}
}

void camPropSetByID(std::ostream& out, int32 propID, double val)
{
	if (liveCapOn || camRcrdng || expsUnderProgress)
		out << "Camera properties cannot be set while live feed is on, camera is recording or experiments are being conducted." << std::endl;
	else
	{
		fillCamProps(hdcam);
		int propListIdx = getCamPropsIdxByID(propID);
		if (propListIdx >= 0)
			setCamPropValue(hdcam, out, propListIdx, val);
		else
			out << "Property with ID \"" << propID << "\" doesn't exist." << std::endl;
	}
}

void liveCapStart(std::ostream& out)
{
	if (liveCapOn)
		out << "Live feed from camera is already on." << std::endl;
	else if (expsUnderProgress)
		out << "Live feed cannot be started while experiments are being conducted." << std::endl;
	else if (camRcrdng)
		out << "Live feed cannot be started while the camera is recording." << std::endl;
	else
	{
		fillCamProps(hdcam);
		int propListWidthIdx = getCamPropsIdxByID(DCAM_IDPROP_IMAGE_WIDTH);
		int propListHeightIdx = getCamPropsIdxByID(DCAM_IDPROP_IMAGE_HEIGHT);
		setCamCapImgSize(camProps[propListWidthIdx].currentVal, camProps[propListHeightIdx].currentVal);
		std::thread liveCapThread(startCamCap, std::ref(out), hdcam);
		liveCapThread.detach();
	}
}

void liveCapStop(std::ostream& out)
{
	if (!liveCapOn)
		out << "Live feed from camera is already off." << std::endl;
	else
		stopCamCap(out);
}

void liveCapLUT(std::ostream& out, int lutMin, int lutMax)
{
	if (lutMax > 65535 || lutMax <= lutMin || lutMin < 0)
		out << "Make sure that lutMin >= 0, lutMax <= 65535 and lutMax > lutMin." << std::endl;
	else
		setCamCapLUT(lutMin, lutMax);
}

void condExpsStart(std::ostream &out, size_t numExps)
{
	if (liveCapOn)
		out << "Live feed must be turned off before conducting experiments." << std::endl;
	else if (expsUnderProgress)
		out << "Experiments are already being conducted." << std::endl;
	else if (camRcrdng)
		out << "Experiments cannot be conducted while the camera is recording." << std::endl;
	else
	{
		std::thread condExpsThread(startConductingExps, std::ref(out), numExps, hdcam);
		condExpsThread.detach();
	}
}

void condExpsStop(std::ostream &out)
{
	stopConductingExps(out);
}

void condExpsStatus(std::ostream &out)
{
	expsStatus(out);
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
	std::cout << "// Date last updated: 03/09/2022                                         //" << std::endl;
	std::cout << "///////////////////////////////////////////////////////////////////////////" << std::endl << std::endl;

	if (initShtr() && initCam())
	{
		fillCamProps(hdcam); // Get a list of camera properties
		std::cout << std::endl;

		// Create a root menu of our cli
		auto otSoft = std::make_unique<cli::Menu>("otSoft", "Main menu of this application.");
		otSoft->Insert("camRec", otSoftCamRec, "Start camera recording.");
		otSoft->Insert("shtrCtrl", otSoftShtrCtrl, "Passthrough for setting up shutter controller. Type 'shtrCtrl help' for more information.");

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

		// Create a submenu for conducting experiments
		auto condExps = std::make_unique<cli::Menu>("condExps", "Menu for conducting experiments.");
		condExps->Insert("start", condExpsStart, "Start conducting n experiments.");
		condExps->Insert("stop", condExpsStop, "Stop ongoing experiments.");
		condExps->Insert("status", condExpsStatus, "Status of the ongoing experiments.");
		otSoft->Insert(std::move(condExps));

		// create the cli with the root menu
		cli::Cli cli(std::move(otSoft));

		// global exit action
		cli.ExitAction([](auto &out)
					   {
						   stopConductingExps(out);	 // Make sure no experiment is being conducted
						   stopCamCap(out);			 // Make sure the camera is not capturing
						   waitFinishCamRcrdng(out); // Wait for the camera to finish recording
						   deinitShtr(out);			 // Reboot the shutter controller
						   dcamdev_close(hdcam);	 // close DCAM handle
						   dcamapi_uninit();		 // Uninit DCAM-API
						   out << "Camera uninitialized properly." << std::endl;
						   out << "Press Enter to exit...";
					   });

		cli::LoopScheduler scheduler;
		cli::CliLocalTerminalSession localSession(cli, scheduler, std::cout, 200);

		localSession.ExitAction(
			[&scheduler](auto &out) // session exit action
			{
				scheduler.Stop();
			});

		scheduler.Run();
	}
	else
		std::cout << "Shutter or Camera initialization unsucessful. Try again later." << std::endl;
}