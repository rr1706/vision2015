#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <thread>
#include <iostream>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <input.hpp>
#include "tracker.hpp"
#include "util.hpp"
#include "functions.hpp"

using namespace cv;
using namespace std;

enum RobotError {
    RE_SUCCESS = 0,
    RE_SIGNAL = 1,
    RE_CAM_RETR = 2,
    RE_CAM_EMPTY = 3,
    RE_LOGIC = 4,
    RE_UDP = 5,
    RE_LINUX = 6
};

int frame_status, signal_status, frame_id, iter;
clock_t last_write;

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
        return RE_CAM_RETR;
    }
    if (rgb.rows < 1 || rgb.cols < 1 || depth.cols < 1 || depth.rows < 1) {
        fprintf(stderr, "empty image\n");
        return RE_CAM_EMPTY;
    }
    // our robot has the camera mounted upside down
    cv::flip(rgb, rgb, -1);
    cv::flip(depth, depth, -1);
    profile_end("kinect");
    profile_start("writer");
    // one image per 4 seconds
    if ((clock() - last_write) > (CLOCKS_PER_SEC * 4)) {
        last_write = clock();
        imwrite("color/robot_" + std::to_string(frame_id) + ".jpg", rgb);
        imwrite("depth/robot_" + std::to_string(frame_id) + ".jpg", depth);
        frame_id ++;
    }
    profile_end("writer");
    profile_start("track");
    vector<Game_Piece> game_pieces;
    try {
        game_pieces = tracker.find_pieces(depth, rgb, drawing);
    } catch (cv::Exception& ex) {
        fprintf(stderr, "vision tracker logic error: %s\n", ex.what());
        return RE_LOGIC;
    }
    try {
        send_udp(game_pieces);
    } catch (std::runtime_error& ex) {
        fprintf(stderr, "sending udp: %s\n", ex.what());
        return RE_UDP;
    }
    profile_end("track");
    profile_end("frame");
    profile_print();
    iter ++;
    return RE_SUCCESS;
}

int robot_loop()
{
    Input input;
    // processes images until told to stop
    while (!frame_status && !signal_status) {
        frame_status = robot_frame(input);
        fflush(stdout);
        if (SHOW_IMAGES) {
            cv::waitKey(1);
        }
    }
    // camera is closed here
    input.cap->release();
    return frame_status | signal_status;
}
void robot_signal(int signum)
{
    fprintf(stderr, "Program received signal %s.\n", strsignal(signum));
    if (signum == SIGINT || signum == SIGTERM) {
        signal_status = 1;
    } else {
        exit(RE_LINUX);
    }
}

int robot_main(int argc, char *argv[])
{
    int status;
    frame_status = signal_status = frame_id = iter = 0;
    last_write = clock();
    signal(SIGINT, robot_signal);
    signal(SIGTERM, robot_signal);
    signal(SIGABRT, robot_signal);
    signal(SIGSEGV, robot_signal);
    fclose(stdin);
    freopen("robot.log", "a", stdout);
    freopen("robot.err", "a", stderr);
    mkdir("depth", 0775);
    mkdir("color", 0775);
    read_config();
    if (argc >= 2) {
        sscanf(argv[1], "%d", &frame_id);
    }
    status = robot_loop();
    fprintf(stderr, "Program finished, status %d\n", status);
    exit(status);
}
