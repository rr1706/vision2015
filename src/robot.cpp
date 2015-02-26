#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <thread>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
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

atomic_int signal_status, frame_status;
atomic_bool grabbed_image;
int frame_id, iter;
clock_t last_write;
double fps_avg, last_frame;

cv::Mat grgb, gdepth;
// lock requested when the capture thread is taking pictures,
// and when the processing thread is copying the image
std::mutex img_lock;

void capture_thread()
{
    Input input;
    // the signal handler can stop the thread this way
    while (!signal_status && !frame_status) {
        img_lock.lock();
        input.getBGR(grgb);
        input.getDepth(gdepth);
        grabbed_image = true;
        img_lock.unlock();
        usleep(1000);
    }
    input.cap->release();
}


int robot_frame()
{
    static ColorTracker tracker;
    Mat rgb, depth, drawing;
    vector<Game_Piece> game_pieces;
    clock_t frame_start, frame_end;

    frame_start = clock();
    profile_start("frame");
    profile_start("camera");
    img_lock.lock();
    // copy images to let the capture thread continue
    rgb = grgb.clone();
    depth = gdepth.clone();
    img_lock.unlock();
    if (rgb.rows < 1 || rgb.cols < 1 || depth.cols < 1 || depth.rows < 1) {
        fprintf(stderr, "empty image\n");
        return RE_CAM_EMPTY;
    }
    // our robot has the camera mounted upside down
    cv::flip(rgb, rgb, -1);
    cv::flip(depth, depth, -1);
    profile_end("camera");
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
    try {
//        game_pieces = tracker.find_pieces(depth, rgb, drawing);
        game_pieces = tracker.find_totes(rgb);
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
    frame_end = clock();
    last_frame = static_cast<double>(frame_end - frame_start) / CLOCKS_PER_SEC;
    fps_avg = fps_avg * 0.9 + last_frame * 0.1;
    printf("FPS: %.2f\n", 1 / fps_avg);
    iter ++;
    return RE_SUCCESS;
}

// prevent accidental death of the driver during practice
// sends zero values to the driver station on UDP when we quit
void death()
{
    vector<Game_Piece> piece;
    try {
        send_udp(piece);
    } catch (...) {
    }
}

int robot_loop()
{
    if (SHOW_IMAGES) {
        namedWindow("Drawing", CV_WINDOW_NORMAL);
        namedWindow("RGB", CV_WINDOW_NORMAL);
        resizeWindow("Drawing", 640, 480);
        resizeWindow("RGB", 640, 480);
        moveWindow("Drawing", 640, 20);
        moveWindow("RGB", 0, 20);
    }
    grabbed_image = false;
    // start the camera thread
    // this might be unnecessary if the camera backend has its own threading/IPC
    // this prevents buffering of images, outdated results
    thread cap(capture_thread);
    // holds off on processing until we have real data for at least one image
    while (!grabbed_image) {
        usleep(20);
    }
    // processes images until told to stop
    while (!frame_status && !signal_status) {
        frame_status = robot_frame();
        fflush(stdout);
        if (SHOW_IMAGES) {
            cv::waitKey(1);
        }
    }
    cap.join();
    death();
    return frame_status | signal_status;
}
void robot_signal(int signum)
{
    fprintf(stderr, "Program received signal %s.\n", strsignal(signum));
    if (signum == SIGINT || signum == SIGTERM) {
        signal_status = 1;
    } else {
        death();
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
    if (!freopen("robot.log", "a", stdout))
        throw std::runtime_error("Failed to log output stream");
    if (!freopen("robot.err", "a", stderr))
        throw std::runtime_error("Failed to log error stream");
    mkdir("depth", 0775);
    mkdir("color", 0775);
    if (argc >= 2) {
        sscanf(argv[1], "%d", &frame_id);
    }
    status = robot_loop();
    fprintf(stderr, "Program finished, status %d\n", status);
    exit(status);
}
