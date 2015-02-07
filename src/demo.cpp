#include <opencv2/highgui/highgui.hpp>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <free.hpp>
#include "yellow.hpp"
#include "functions.h"
#include "tracker.hpp"
#include "util.hpp"

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
    IRTracker tracker;
    VideoWriter writer("ir.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640, 480), true);
    while (true) {
        profile_start("frame");
        img = kinectIR(0);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey();
        tracker.find_totes(img, draw);
        writer << draw;
        profile_end("frame");
        profile_print();
        fflush(stdout);
    }
    writer.release();
    cv::destroyAllWindows();
    return 0;
}

int depthvideo() {
    pdebug("Starting depth video saver application..\n");
    Mat depth, rgb, drawing;
    namedWindow("Drawing", CV_WINDOW_NORMAL);
    DepthTracker tracker;
    VideoWriter writer("depth.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640, 480), true);
    VideoWriter writerrgb("rgb.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640, 480), true);
    while (true) {
        profile_start("frame");
        profile_start("kinect");
        depth = kinectDepth(0);
        rgb = kinectRGB(0);
        profile_end("kinect");
        convertScaleAbs(depth, depth, 0.25, 0);
        cvtColor(rgb, rgb, CV_RGB2BGR);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey(0);
        profile_start("track");
        vector<Game_Piece> game_pieces = tracker.find_pieces(depth, rgb, drawing);
        profile_end("track");
        try {
            send_udp(game_pieces);
        } catch (std::runtime_error error) {
            fprintf(stderr, "%s\n", error.what());
        }
        profile_start("write");
        writerrgb << rgb;
        writer << drawing;
        profile_end("write");
        profile_end("frame");
        profile_print();
        fflush(stdout);
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

        tracker.find_pieces(depth, rgb, drawing);
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
        fflush(stdout);
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
    return 0;
}

int basictimer()
{
    pdebug("Starting in basic kinect color capture mode..\n");
    Mat img;
    while (true) {
        profile_start("frame");
        img = kinectRGB(0);
        imshow("Image", img);
        int key = waitKey(1);
        if ((key & 0xFF) == 27) {
            break;
        }
        profile_end("frame");
        profile_print();
        fflush(stdout);
    }
    destroyAllWindows();
    return 0;
}

int record()
{
    Mat depth, rgb;
    bool cameraMode = true;
    int i = 0;
    mkdir("../record", 0755);
    mkdir("../record/depth", 0755);
    mkdir("../record/rgb", 0755);
    namedWindow("Depth");
    namedWindow("RGB");
    namedWindow("IR");
    while (true) {
        puts("Frame");
        depth = kinectDepth(0);
        rgb = kinectRGB(0);
        convertScaleAbs(depth, depth, 0.25, 0);
        cvtColor(rgb, rgb, CV_RGB2BGR);
        imshow("Depth", depth);
        imshow("RGB", rgb);
        imwrite("../record/depth/img_" + to_string(i) + ".jpg", depth);
        imwrite("../record/rgb/img_" + to_string(i++) + ".jpg", rgb);
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
    return depthvideo();
}
