/*These following 3 functions call for the corresponding image
from the Microsoft Kinect v1 and return it as a cv::Mat*/

#include <stdexcept>
#ifdef __linux__
#include "libfreenect.h"
#include "libfreenect_sync.h"
#endif
#ifdef __APPLE__
#include <libfreenect/libfreenect_sync.h>
#endif
#include "free.hpp"

#if defined(__linux__) || defined(__APPLE__)
cv::Mat kinectDepth(int index)
{
    static char *data = 0;
    unsigned int timestamp;
    if (freenect_sync_get_depth((void**)&data, &timestamp, index, FREENECT_DEPTH_11BIT)) {
        throw std::runtime_error("failed to get kinect depth");
    }
    cv::Mat img(cv::Size(640, 480), CV_16UC1, data);
    return img;
}

cv::Mat kinectRGB(int index)
{
    static char *data = 0;
    unsigned int timestamp;
    if (freenect_sync_get_video((void**)&data, &timestamp, index, FREENECT_VIDEO_RGB)) {
        throw std::runtime_error("failed to get kinect depth");
    }
    cv::Mat img(cv::Size(640, 480), CV_8UC3, data);
    return img;
}

cv::Mat kinectIR(int index)
{
    static char *data = 0;
    unsigned int timestamp;
    if (freenect_sync_get_video((void**)&data, &timestamp, index, FREENECT_VIDEO_IR_8BIT)) {
        throw std::runtime_error("failed to get kinect depth");
    }
    cv::Mat img(cv::Size(640, 480), CV_8UC1, data);
    return img;
}
#endif

