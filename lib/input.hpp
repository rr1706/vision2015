#ifndef INPUT_HPP
#define INPUT_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <memory>

enum InputType {
    KINECT,
    XTION
};

extern InputType inputSource;

class Input
{
private:
    int camId;
public:
    std::shared_ptr<cv::VideoCapture> cap;
    Input(int camId = 0);
    void getBGR(cv::Mat &out);
    void getIR(cv::Mat &out);
    void getDepth(cv::Mat &out);
};

#endif // INPUT_HPP
