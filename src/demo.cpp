#include <opencv2/highgui/highgui.hpp>
#include <free.hpp>
#include "timer.h"
#include "yellow.hpp"
#include "functions.h"
#include "tracker.hpp"
#include "util.hpp"
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace cv;
using namespace std;

int imdir() {
    int i = 0;
    ColorTracker tracker;
    while (true) {
        printf("%d\n", i);
        Mat color = imread("../images/yellow box/im_" + std::to_string(i) + ".jpg");
        std::vector<Game_Piece> totes = tracker.find_totes(color);
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
    Mat img, draw;
    namedWindow("Drawing", CV_WINDOW_NORMAL);
    DECLARE_TIMING(Timer);
    START_TIMING(Timer);
    double frame_time_ms;
    IRTracker tracker;
    VideoWriter writer("ir.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640, 480), true);
    while (true) {
        img = kinectIR(0);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey();
        vector<YellowTote> totes_= tracker.find_totes(img, draw);
        writer << draw;
        STOP_TIMING(Timer);
        frame_time_ms = GET_TIMING(Timer);
        if (frame_time_ms > 0) {
            printf("Current FPS = %.1f\n", 1000/frame_time_ms);
        }
        START_TIMING(Timer);

    }
    writer.release();
    cv::destroyAllWindows();
    return 0;
}

int depth() {
    Mat depth, rgb, drawing;
    namedWindow("Drawing", CV_WINDOW_NORMAL);
    //namedWindow("Calibrated", CV_WINDOW_AUTOSIZE);
    DECLARE_TIMING(Timer);
    START_TIMING(Timer);
    double frame_time_ms;
    DepthTracker tracker;
    while (true) {
        ///Acquire image
        depth = kinectDepth(0);
        rgb = kinectRGB(0);
        convertScaleAbs(depth, depth, 0.25, 0);
        cvtColor(rgb, rgb, CV_RGB2BGR);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey(0);
        vector<Game_Piece> game_pieces = tracker.find_pieces(depth, rgb, drawing);
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

int depthvideo() {
    Mat depth, rgb, drawing;
    namedWindow("Drawing", CV_WINDOW_NORMAL);
    DECLARE_TIMING(Timer);
    START_TIMING(Timer);
    double frame_time_ms;
    DepthTracker tracker;
    VideoWriter writer("depth.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640, 480), true);
    VideoWriter writerrgb("rgb.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640, 480), true);
    while (true) {
        ///Acquire image
        depth = kinectDepth(0);
        rgb = kinectRGB(0);
        convertScaleAbs(depth, depth, 0.25, 0);
        cvtColor(rgb, rgb, CV_RGB2BGR);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey(0);
        writerrgb << rgb;
        vector<Game_Piece> game_pieces = tracker.find_pieces(depth, rgb, drawing);
        send_udp(game_pieces);
        writer << drawing;
        STOP_TIMING(Timer);
        frame_time_ms = GET_TIMING(Timer);
        if (frame_time_ms > 0) {
            printf("Current FPS = %.1f\n", 1000/frame_time_ms);
        }
        START_TIMING(Timer);

    }
    writer.release();
    writerrgb.release();
    cv::destroyAllWindows();
    return 0;
}

int depthimdir() {
    int i = 0;
    int key = 0;
    namedWindow("Calibrated");
    namedWindow("Drawing");
    namedWindow("RGB");
    moveWindow("RGB", 0, 20);
    moveWindow("Drawing", 640, 20);
    DepthTracker tracker;
    Mat drawing;
    while (true) {
        printf("<< ../images/green bin/*/img_%d.jpg\n", i);
        Mat depth = imread("../images/green bin/depth/img_" + std::to_string(i) + ".jpg", CV_LOAD_IMAGE_GRAYSCALE);
        Mat rgb = imread("../images/green bin/rgb/img_" + std::to_string(i) + ".jpg");
//        Mat ir = imread("../images/green bin/ir/img_" + std::to_string(i) + ".jpg");

        vector<Game_Piece> game_pieces = tracker.find_pieces(depth, rgb, drawing);
        int raw = cv::waitKey(0) & 0xFFFF;
        key = raw & 0xFF;
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

int color()
{
    ColorTracker tracker;
    Mat img;
    while (true) {
        img = kinectRGB(0);
        cvtColor(img, img, CV_RGB2BGR);
        auto totes = tracker.find_totes(img);
        send_udp(totes);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey(0);

    }
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

int record()
{
    Mat depth, rgb, ir;
    bool cameraMode = true;
    int i = 0;
    mkdir("../record", 0755);
    mkdir("../record/depth", 0755);
    mkdir("../record/rgb", 0755);
    mkdir("../record/ir", 0755);
    namedWindow("Depth");
    namedWindow("RGB");
    namedWindow("IR");
    while (true) {
        puts("Frame");
        depth = kinectDepth(0);
        rgb = kinectRGB(0);
//        ir = kinectIR(0);
        convertScaleAbs(depth, depth, 0.25, 0);
        cvtColor(rgb, rgb, CV_RGB2BGR);
        imshow("Depth", depth);
        imshow("RGB", rgb);
//        imshow("IR", ir);
        imwrite("../record/depth/img_" + to_string(i) + ".jpg", depth);
        imwrite("../record/rgb/img_" + to_string(i++) + ".jpg", rgb);
//        imwrite("../record/ir/img_" + to_string(i++) + ".jpg", ir);
        int key;
        if (cameraMode) {
            key = waitKey(0);
        } else {
            key = waitKey(1);
        }
        switch (key & 0xFF) {
        case 'c':
            cameraMode = !cameraMode;
            break;
        case ' ':
            break;
        case 27:
            puts("Quitting.");
            return 0;
        }
    }
    cv::destroyAllWindows();
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
    return color();
}
