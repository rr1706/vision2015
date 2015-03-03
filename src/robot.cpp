#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <thread>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
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
        try {
            input.getBGR(grgb);
            input.getDepth(gdepth);
        } catch (cv::Exception& ex) {
            fprintf(stderr, "failed to capture image: %s", ex.what());
            abort();
        }
        if (grgb.empty() || gdepth.empty()) {
            fprintf(stderr, "rgb/depth images are empty");
            abort();
        }
        grabbed_image = true;
        img_lock.unlock();
        usleep(30000);
    }
    input.cap->release();
}

static ColorTracker tracker;
static vector<Game_Piece> game_pieces;
static chrono::high_resolution_clock::time_point frame_start, frame_end;
static Mat colorMat, depthMat, drawing;

int robot_frame()
{
    using namespace std::chrono;
    frame_start = high_resolution_clock::now();
    profile_start("frame");
    profile_start("camera");
    profile_start("lock_wait");
    img_lock.lock();
    profile_end("lock_wait");
    // copy images to let the capture thread continue
    grgb.copyTo(colorMat);
    gdepth.copyTo(depthMat);
    img_lock.unlock();
    // our robot has the camera mounted upside down
    cv::flip(colorMat, colorMat, -1);
    cv::flip(depthMat, depthMat, -1);
    profile_end("camera");
    profile_start("writer");
    // one image per 4 seconds
    if ((clock() - last_write) > (CLOCKS_PER_SEC * 4)) {
        last_write = clock();
        imwrite("color/robot_" + std::to_string(frame_id) + ".jpg", colorMat);
        imwrite("depth/robot_" + std::to_string(frame_id) + ".jpg", depthMat);
        frame_id ++;
    }
    profile_end("writer");
    profile_start("track");
    try {
        game_pieces = tracker.find_totes(depthMat, colorMat, drawing);
    } catch (cv::Exception& ex) {
        fprintf(stderr, "vision tracker logic error: %s\n", ex.what());
        return RE_LOGIC;
    }
    profile_end("track");
    profile_start("udp");
    try {
        send_udp(game_pieces);
    } catch (std::runtime_error& ex) {
        fprintf(stderr, "sending udp: %s\n", ex.what());
        return RE_UDP;
    }
    profile_end("udp");
    if (SHOW_IMAGES) {
        profile_start("imshow");
        char fps[16];
        sprintf(fps, "FPS: %.2f", 1 / fps_avg);
        putText(drawing, fps, Point(550, 20), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE);
        pyrDown(drawing, drawing, Size(drawing.cols / 2, drawing.rows / 2));
        imshow("Drawing", drawing);
//        imshow("RGB", colorMat);
        profile_end("imshow");
    }
    profile_end("frame");
    profile_print();
    frame_end = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(frame_end - frame_start);
    fps_avg = fps_avg * 0.9 + time_span.count() * 0.1;
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
//        namedWindow("RGB", CV_WINDOW_NORMAL);
        resizeWindow("Drawing", 640, 480);
//        resizeWindow("RGB", 640, 480);
        moveWindow("Drawing", 0, 20);
//        moveWindow("RGB", 640, 20);
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
        profile_start("flush");
        fflush(stdout);
        profile_end("flush");
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
