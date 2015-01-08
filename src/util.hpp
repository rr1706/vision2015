#ifndef RR15_UTIL_HPP
#define RR15_UTIL_HPP

#include <opencv2/highgui/highgui.hpp>

#ifdef DEBUG
#define DEBUG_SHOW imshow
#else
void DEBUG_SHOW(std::string windowName, cv::Mat image){}
#endif

#define COLOR_RED cv::Scalar(0, 0, 255)
#define COLOR_BLUE cv::Scalar(255, 0, 0)
#define COLOR_GREEN cv::Scalar(0, 255, 0)
#define COLOR_BLACK cv::Scalar(0, 0, 0)
#define COLOR_WHITE cv::Scalar(255, 255, 255)


#endif // defined(RR15_UTIL_HPP)
