#include "input.hpp"
#include "free.hpp"

InputType inputSource = XTION;

Input::Input(int camId) : camId(camId)
{
    if (inputSource == XTION) {
        cap = std::make_shared<cv::VideoCapture>(CV_CAP_OPENNI_ASUS);
    }
}

void Input::getBGR(cv::Mat &out)
{
    if (inputSource == KINECT) {
        cv::Mat rgb = kinectRGB(camId);
        cvtColor(rgb, out, CV_RGB2BGR);
    } else if (inputSource == XTION) {
        cap.get()->grab();
        cap.get()->retrieve(out, CV_CAP_OPENNI_BGR_IMAGE);
    }
}

void Input::getIR(cv::Mat &out)
{
    if (inputSource == KINECT) {
        kinectIR(camId).copyTo(out);
    } else if (inputSource == XTION) {
        throw std::runtime_error("IR from the ASUS Xtion is not supported at the moment.");
    }
}

void Input::getDepth(cv::Mat &out)
{
    if (inputSource == KINECT) {
        cv::Mat depth = kinectDepth(camId);
        cv::convertScaleAbs(depth, out, 0.25, 0);
    } else if (inputSource == XTION) {
        cap.get()->grab();
        cap.get()->retrieve(out, CV_CAP_OPENNI_DEPTH_MAP);
        out -= 512;
        out.convertTo(out, CV_8UC1, 0.0625);
        printf("Xtion image res: x=%d, y=%d\n", out.cols, out.rows);
        out = out(cv::Rect(22, 12, 590, 450));
        for (int x = 0; x < out.cols; x++) {
            for (int y = 0; y < out.rows; y++) {
                if (out.at<uint8_t>(y, x) == 0) {
                    out.at<uint8_t>(y, x) = 254;
                }
            }
        }
    }
}
