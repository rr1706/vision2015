#include <opencv2/highgui/highgui.hpp>
#include <free.hpp>
#include "timer.h"
#include "yellow.hpp"
#include "functions.h"
#include "tracker.hpp"
#include "util.hpp"
#include <signal.h>

using namespace cv;
using namespace std;

int imdir() {
    int i = 0;
    ColorTracker tracker;
    while (true) {
        printf("%d\n", i);
        Mat color = imread("../images/yellow box/im_" + std::to_string(i) + ".jpg");
        std::vector<YellowTote> totes = tracker.find_totes(color);
        imshow("Input", color);
        int raw = cv::waitKey(0) & 0xFFFF;
        printf("KEY: %X\n", raw);
        int key = raw & 0xFF;
        if ((raw & 0xFF00) == 0xFF00) {
            if (key == 0x51) {
                i--;
            } else if (key == 0x53) {
                i++;
            }
        } else if (key == 27) {
            break;
        }
    }
    cv::destroyAllWindows();
    return 0;
}

int irtest() {
    Mat img;
    namedWindow("Drawing", CV_WINDOW_AUTOSIZE);
    //namedWindow("Calibrated", CV_WINDOW_AUTOSIZE);
    DECLARE_TIMING(Timer);
    START_TIMING(Timer);
    double frame_time_ms;
    IRTracker tracker;
    while (true) {
        ///Acquire image
        //img = kinectDepth(0);
        img = imread("../images/ir.png", CV_LOAD_IMAGE_GRAYSCALE);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        //vector<Game_Piece> game_pieces = find_with_depth(img, key);
        //vector<YellowTote> totes = find_yellow_color(img);
        vector<YellowTote> totes_= tracker.find_totes(img);
        STOP_TIMING(Timer);
        frame_time_ms = GET_TIMING(Timer);
        if (frame_time_ms > 0) {
            //printf("Current FPS = %.1f\n", 1000/frame_time_ms);
        }
        START_TIMING(Timer);

    }
    cv::destroyAllWindows();
    return 0;
}

int depth() {
    Mat img;
    namedWindow("Drawing", CV_WINDOW_AUTOSIZE);
    //namedWindow("Calibrated", CV_WINDOW_AUTOSIZE);
    DECLARE_TIMING(Timer);
    START_TIMING(Timer);
    double frame_time_ms;
    DepthTracker tracker;
    while (true) {
        ///Acquire image
        img = kinectDepth(0);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey(0);
        vector<Game_Piece> game_pieces = tracker.find_pieces(img, key);
//        imshow("Image", img);
        //vector<YellowTote> totes = find_yellow_color(img);
        //vector<YellowTote> totes_= tracker.find_totes(img);
        STOP_TIMING(Timer);
        frame_time_ms = GET_TIMING(Timer);
        if (frame_time_ms > 0) {
            printf("Current FPS = %.1f\n", 1000/frame_time_ms);
        }
        START_TIMING(Timer);

    }
    cv::destroyAllWindows();
    return 0;
}

int basictimer()
{
    pdebug("Starting in basic kinect color capture mode..\n");
    Mat img;
    DECLARE_TIMING(Timer);
    START_TIMING(Timer);
    double frame_time_ms;
    while (true) {
        img = kinectRGB(0);
        imshow("Image", img);
        int key = waitKey(1);
        if ((key & 0xFF) == 27) {
            break;
        }
        STOP_TIMING(Timer);
        frame_time_ms = GET_TIMING(Timer);
        if (frame_time_ms > 0) {
            pdebug("Current FPS = %.1f\n", 1000/frame_time_ms);
        }
        START_TIMING(Timer);
        fflush(stdout);
    }
    destroyAllWindows();
    return 0;
}

void handle_signal(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
        exit(5);
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    return depth();
}
