#include "camRec.h"

void recordFrames(std::ostream& out, HDCAM hdcam, HDCAMWAIT hwait)
{
    DCAMERR err;

    // start capture
    err = dcamcap_start(hdcam, DCAMCAP_START_SNAP);
    if (failed(err))
    {
        out << "Could not start camera recording." << std::endl;
        return;
    }
    else
    {
        // set wait param
        DCAMWAIT_START waitstart;
        memset(&waitstart, 0, sizeof(waitstart));
        waitstart.size = sizeof(waitstart);
        waitstart.eventmask = DCAMWAIT_CAPEVENT_STOPPED | DCAMWAIT_CAPEVENT_RELOADFRAME;
        waitstart.timeout = 1000;

        // Wait for capture to complete
        bool bStop = false;
        while (!bStop)
        {
            err = dcamwait_start(hwait, &waitstart);
            if (!failed(err) && (waitstart.eventhappened & DCAMWAIT_CAPEVENT_STOPPED))
                bStop = true;
            if (!failed(err) && (waitstart.eventhappened & DCAMWAIT_CAPEVENT_RELOADFRAME))
                out << "DCAMWAIT_CAPEVENT_RELOADFRAME" << std::endl;

            // get capture and transfer status
            int32 capStatus = 0;
            err = dcamcap_status(hdcam, &capStatus);
            out << "Capture Status: ";
            if (failed(err))
                out << "Can't retrieve";
            else
                switch (capStatus)
                {
                case DCAMCAP_STATUS_BUSY: out << "BUSY"; break;
                case DCAMCAP_STATUS_ERROR: out << "ERROR"; break;
                case DCAMCAP_STATUS_READY: out << "READY"; break;
                case DCAMCAP_STATUS_STABLE: out << "STABLE"; break;
                case DCAMCAP_STATUS_UNSTABLE: out << "UNSTABLE"; break;
                default: break;
                }
            out << ", ";
            DCAMCAP_TRANSFERINFO transInfo;
            memset(&transInfo, 0, sizeof(transInfo));
            transInfo.size = sizeof(transInfo);
            transInfo.iKind = DCAMCAP_TRANSFERKIND_FRAME;
            err = dcamcap_transferinfo(hdcam, &transInfo);
            if (failed(err))
                out << "Frames Captured: Unavailable, Newest Frame Index: Unavailable" << std::endl;
            else
                out << "Frames Captured: " << transInfo.nFrameCount << ", "
                << "Newest Frame Index: " << transInfo.nNewestFrameIndex << std::endl;
        }
        // stop capture
        dcamcap_stop(hdcam);
    }
}

void startCamRecording(std::ostream& out, HDCAM hdcam, camRecInfo recInfo)
{
    DCAMERR err;

    // open wait handle
    DCAMWAIT_OPEN	waitopen;
    memset(&waitopen, 0, sizeof(waitopen));
    waitopen.size = sizeof(waitopen);
    waitopen.hdcam = hdcam;

    err = dcamwait_open(&waitopen);
    if (failed(err))
    {
        out << "Could not create DCAMWAIT_OPEN object." << std::endl;
        return;
    }
    else
    {
        HDCAMWAIT hwait = waitopen.hwait;

        // Get frame size in bytes
        double bufframebytes;
        err = dcamprop_getvalue(hdcam, DCAM_IDPROP_BUFFER_FRAMEBYTES, &bufframebytes);
        if (failed(err))
            out << "Could not retrieve DCAM_IDPROP_BUFFER_FRAMEBYTES value." << std::endl;
        else
        {
            // Check if enough memory is available
            MEMORYSTATUSEX mem;
            mem.dwLength = sizeof(mem);
            if (!GlobalMemoryStatusEx(&mem))
                out << "Failed to retrieve physical memory info." << std::endl;
            else
            {
                size_t frameSize = (size_t)bufframebytes;
                int	number_of_buffer = recInfo.nFrames;
                if (mem.ullAvailPhys < frameSize * number_of_buffer)
                    out << "Not enough memory. Required: " << frameSize * number_of_buffer / 1024.0 / 1024.0 / 1024.0
                        << " GB, Available: " << mem.ullAvailPhys / 1024.0 / 1024.0 / 1024.0 << " GB" << std::endl;
                else
                {
                    // allocate buffer
                    void** pFrames = new void* [number_of_buffer];
                    char* buf = new char[frameSize * number_of_buffer];
                    memset(buf, 0, frameSize * number_of_buffer);

                    int		i;
                    for (i = 0; i < number_of_buffer; i++)
                    {
                        pFrames[i] = buf + frameSize * i;
                    }

                    DCAMBUF_ATTACH bufattach;
                    memset(&bufattach, 0, sizeof(bufattach));
                    bufattach.size = sizeof(bufattach);
                    bufattach.iKind = DCAMBUF_ATTACHKIND_FRAME;
                    bufattach.buffer = pFrames;
                    bufattach.buffercount = number_of_buffer;

                    // attach user buffer
                    err = dcambuf_attach(hdcam, &bufattach);
                    if (failed(err))
                        out << "Could not attach frame buffer." << std::endl;
                    else
                    {
                        // Start recording
                        recordFrames(out, hdcam, hwait);

                        // release buffer
                        dcambuf_release(hdcam);

                        // Save images
                        std::ofstream imgsFile(recInfo.filePath, std::ios::out | std::ios::binary);
                        if (!imgsFile)
                            out << "Could not open file: " << recInfo.filePath << std::endl;
                        else
                        {
                            double imgWidth_d = 0, imgHeight_d = 0;
                            DCAMERR errW = dcamprop_getvalue(hdcam, DCAM_IDPROP_IMAGE_WIDTH, &imgWidth_d);
                            DCAMERR errH = dcamprop_getvalue(hdcam, DCAM_IDPROP_IMAGE_HEIGHT, &imgHeight_d);
                            if (failed(errW) || failed(errH))
                                out << "Could not retrieve image width and height properties." << std::endl;
                            else
                            {
                                uint16_t imgWidth = (uint16_t)imgWidth_d, imgHeight = (uint16_t)imgHeight_d, nFrames = (uint16_t)recInfo.nFrames;
                                imgsFile.write((char*)&imgWidth, sizeof(imgWidth));
                                imgsFile.write((char*)&imgHeight, sizeof(imgHeight));
                                imgsFile.write((char*)&nFrames, sizeof(nFrames));
                                imgsFile.write(buf, frameSize * number_of_buffer);
                            }
                            imgsFile.close();
                            if (!imgsFile.good())
                                out << "Error occurred at writing time!" << std::endl;
                        }
                    }
                    // free buffer
                    delete[] buf;
                    delete[] pFrames;
                }
            }
        }
        // close wait handle
        dcamwait_close(hwait);
    }
}