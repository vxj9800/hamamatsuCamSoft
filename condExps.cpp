#include "condExps.h"

// Define globals for this file only
size_t nExps = 0; // Number of experiments to be conducted
size_t expLen_us = 0; // Length of one experiment in us

// Define globals that are 'extern' in the header file
std::atomic<bool> expsUnderProgress = false; // Whether the experiments are currently being conducted or not

void startConductingExps(std::ostream &out, size_t numExps, HDCAM hdcam)
{
    // State that some experiments are being conducted
    expsUnderProgress = true;

    // Get some info from the shutter controller
    size_t expLen_us = std::stoull(sndCmdRecRspShtr(std::string("++expLen"))); // Length of one experiment in us
    size_t camTrigLen_us = std::stoull(sndCmdRecRspShtr(std::string("++camTrigLen"))); // Length of time the camera should be recording in us

    // Define the number of experiments
    nExps = numExps;

    // Get some camera info
    size_t exposureTime_ns = (size_t)(get_camPropInfo(hdcam, DCAM_IDPROP_EXPOSURETIME).currentVal * 1e9);
    size_t nFrames = camTrigLen_us * 1000 / exposureTime_ns;

    // Get camera property values so that we can reset it after conducting experiments
    double trigSource = get_camPropInfo(hdcam, DCAM_IDPROP_TRIGGERSOURCE).currentVal;
    double trigMode = get_camPropInfo(hdcam, DCAM_IDPROP_TRIGGER_MODE).currentVal;
    double trigPolarity = get_camPropInfo(hdcam, DCAM_IDPROP_TRIGGERPOLARITY).currentVal;

    // Name a folder with current date and time
    char fldrName[80];
    time_t t = std::time(nullptr);
    struct tm timeInfo;
    localtime_s(&timeInfo, &t);
    std::strftime(fldrName, sizeof(fldrName), "%Y%m%d%H%M%S", &timeInfo);
    std::filesystem::path folderPath = std::string(fldrName);

    // If the folder exists then delete it, probably won't ever happen.
    if (std::filesystem::is_directory(folderPath))
        std::filesystem::remove_all(folderPath);

    // Create a folder
    std::filesystem::create_directories(folderPath);

    // Start conducting experiments
    while (nExps && expsUnderProgress)
    {
        // Set up the recording stuff
        std::filesystem::path filePath = (folderPath / (std::to_string(numExps - nExps) + std::string(".bin"))).make_preferred();
        if (std::filesystem::is_directory(filePath.parent_path())) // Make sure if the folder was created
        {
            camRecInfo recInfo;
            recInfo.nFrames = (unsigned int) nFrames;
            recInfo.filePath = filePath.parent_path() /= filePath.filename();
            // Set camera trigger source to external
            setCamPropValue(hdcam, out, getCamPropsIdxByID(DCAM_IDPROP_TRIGGERSOURCE), DCAMPROP_TRIGGERSOURCE__EXTERNAL);
            setCamPropValue(hdcam, out, getCamPropsIdxByID(DCAM_IDPROP_TRIGGER_MODE), DCAMPROP_TRIGGER_MODE__START);
            setCamPropValue(hdcam, out, getCamPropsIdxByID(DCAM_IDPROP_TRIGGERPOLARITY), DCAMPROP_TRIGGERPOLARITY__POSITIVE);
            // Initiate the camera capture
            std::thread camRecThread(startCamRecording, std::ref(out), hdcam, recInfo);
            while (!camRcrdng); // Wait for the thread to actually start executing
            camRecThread.detach();
        }
        else
            out << "Failed to create the folder." << std::endl;

        // Wait till camera is ready to capture or an error ocurred
        while (camRcrdng && !camRdy2Capt);

        // Make the shutter controller execute one experiment
        if (camRcrdng)
            sndShtrCmd(out, std::string("++start"));
        else
            out << "Seems like the camera could not be set up properly for Exp No.: " << nExps << std::endl;

        // If an experiment is started then code should not reach here before it is completed.
        // Wait for the camera to finish recording, transferring images to SSD/HDD should be remaining at this point.
        waitFinishCamRcrdng(out);

        // Reduce nExps by one only if it is grather than 0
        nExps += nExps ? -1 : 0;
    }

    // Reset Camera properties to the original values
    setCamPropValue(hdcam, out, getCamPropsIdxByID(DCAM_IDPROP_TRIGGERSOURCE), trigSource);
    setCamPropValue(hdcam, out, getCamPropsIdxByID(DCAM_IDPROP_TRIGGER_MODE), trigMode);
    setCamPropValue(hdcam, out, getCamPropsIdxByID(DCAM_IDPROP_TRIGGERPOLARITY), trigPolarity);

    // State that the experiments are done
    expsUnderProgress = false;

    // Make sure that the number of experiments is zero
    nExps = 0;
}

void stopConductingExps(std::ostream &out)
{
    nExps = 0;
    if (expsUnderProgress)
        out << "The system will stop after finishing the ongoing experiment." << std::endl;
    while (expsUnderProgress);
}

void expsStatus(std::ostream &out)
{
    if (expsUnderProgress)
    {
        out << "Number of experements remaining to be conducted:    " << nExps << std::endl;
        out << "Time required to finish remaining experiments:      " << expLen_us * nExps / 1000 << " ms at least" << std::endl;
    }
    else
        out << "No experiments are under process." << std::endl;
}