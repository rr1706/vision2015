#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <thread>
#include <iostream>
#include <signal.h>
#include <input.hpp>
#include "tracker.hpp"
#include "util.hpp"
#include "functions.h"

using namespace cv;
using namespace std;

static int status;

int robot_frame(Input& input)
{
    static DepthTracker tracker;
    Mat rgb, depth, drawing;
    profile_start("frame");
    profile_start("kinect");
    try {
        input.getBGR(rgb);
        input.getDepth(depth);
    } catch (cv::Exception& ex) {
        fprintf(stderr, "failed to get camera input: %s\n", ex.what());
        return 3;
    }
    if (rgb.rows < 1 || rgb.cols < 1 || depth.cols < 1 || depth.rows < 1) {
        fprintf(stderr, "empty image\n");
        return 4;
    }
    // our robot has the camera mounted upside down
    cv::flip(rgb, rgb, -1);
    cv::flip(depth, depth, -1);
    profile_end("kinect");
    profile_start("track");
    vector<Game_Piece> game_pieces;
    try {
        game_pieces = tracker.find_pieces(depth, rgb, drawing);
    } catch (cv::Exception& ex) {
        fprintf(stderr, "vision tracker logic error: %s\n", ex.what());
        return 5;
    }
    try {
        send_udp(game_pieces);
    } catch (std::runtime_error& ex) {
        fprintf(stderr, "sending udp: %s\n", ex.what());
        return 2;
    }
    profile_end("track");
    profile_end("frame");
    profile_print();
    return 0;
}

int robot_loop()
{
    Input input;
    // processes images until told to stop
    while (!status) {
        status = robot_frame(input);
        fflush(stdout);
        if (SHOW_IMAGES) {
            cv::waitKey(1);
        }
    }
    // camera is closed here
    input.cap->release();
    return status;
}
void robot_signal(int signum)
{
    if (signum == SIGINT || signum == SIGTERM) {
        status = 1;
    }
}


int robot_main()
{
    status = 0;
    signal(SIGINT, robot_signal);
    signal(SIGTERM, robot_signal);
    fclose(stdin);
    freopen("robot.log", "a", stdout);
    freopen("robot.err", "a", stderr);
    read_config();
    robot_loop();
    fprintf(stderr, "Program finished, status %d\n", status);
    exit(status);
}
