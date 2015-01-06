#ifndef LIBFREENECT_CV_HPP
#define LIBFREENECT_CV_HPP

#include <opencv2/imgproc/imgproc.hpp>

cv::Mat kinectDepth(int index);
cv::Mat kinectRGB(int index);
cv::Mat kinectIR(int index);

#endif
