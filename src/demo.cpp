#include <opencv2/highgui/highgui.hpp>
#include <chrono>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <input.hpp>
#include "yellow.hpp"
#include "functions.hpp"
#include "tracker.hpp"
#include "util.hpp"

using namespace cv;
using namespace std;

bool endprogram = false;

int robot();

int imdir() {
    int i = 0, raw, key;
    ColorTracker tracker;
    Mat color, draw;
    while (true) {
        printf("%d\n", i);
        color = imread("../images/yellow box/im_" + std::to_string(i) + ".jpg");
        tracker.find_totes(color, color, draw);
        imshow("Input", color);
        raw = cv::waitKey(0) & 0xFFFF;
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

int irtest() {
    Mat ir, draw;
    namedWindow("Drawing", CV_WINDOW_NORMAL);
    IRTracker tracker;
    Input input;
    VideoWriter writer("ir.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640, 480), true);
    while (true) {
        profile_start("frame");
        input.getBGR(ir);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey();
        tracker.find_totes(ir, ir, draw);
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
    namedWindow("RGB", CV_WINDOW_NORMAL);
    resizeWindow("Drawing", 640, 480);
    resizeWindow("RGB", 640, 480);
    moveWindow("Drawing", 640, 20);
    moveWindow("RGB", 0, 20);
    Input input;
    DepthTracker tracker;
//    VideoWriter writer("depth.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640, 480), true);
//    VideoWriter writerrgb("rgb.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640, 480), true);
    while (true) {
        if (endprogram)
            break;
        profile_start("frame");
        profile_start("kinect");
        input.getBGR(rgb);
        input.getDepth(depth);
        cv::flip(rgb, rgb, -1);
        cv::flip(depth, depth, -1);
        profile_end("kinect");
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey(0);
        profile_start("track");
        vector<Game_Piece> game_pieces = tracker.find_totes(depth, rgb, drawing);
        send_udp(game_pieces);
        profile_end("track");
//        profile_start("write");
//        writerrgb << rgb;
//        writer << drawing;
//        profile_end("write");
        profile_end("frame");
        profile_print();
        fflush(stdout);
    }
//    writer.release();
//    writerrgb.release();
    cv::destroyAllWindows();
    return 0;
}

int depthimdir() {
    int raw, key;
    namedWindow("Drawing", CV_WINDOW_NORMAL);
    namedWindow("RGB", CV_WINDOW_NORMAL);
    resizeWindow("Drawing", 640, 480);
    resizeWindow("RGB", 640, 480);
    moveWindow("Drawing", 640, 20);
    moveWindow("RGB", 0, 20);
    DepthTracker tracker;
    Mat drawing, depth, color;
    string base_folder = "/home/connor/robotics/2015/matches/2015-02-19 20:00:22/";
    DIR *dir;
    struct dirent *dp;
    dir = opendir((base_folder + "/color/").c_str());
    while ((dp = readdir(dir)) != NULL) {
        string fn = dp->d_name;
        if (fn == "." || fn == "..")
            continue;
        printf("\n>>>>> %s\n", fn.c_str());
        depth = imread(base_folder + "/depth/" + fn, CV_LOAD_IMAGE_GRAYSCALE);
        color = imread(base_folder + "/color/" + fn);
        if (color.rows < 1 || depth.rows < 1)
            continue;
        imshow("RGB", color);

        tracker.find_totes(depth, color, drawing);
        raw = cv::waitKey(0) & 0xFFFF;
        key = raw & 0xFF;
        if ((raw & 0xFF00) == 0xFF00) {
            if (key == 0x53) {
                continue;
            }
        } else if (key == 27) {
            break;
        }
        fflush(stdout);
    }
    closedir(dir);
    cv::destroyAllWindows();
    return 0;
}

int colorimdir() {
    int raw, key;
    namedWindow("Drawing", CV_WINDOW_NORMAL);
    namedWindow("RGB", CV_WINDOW_NORMAL);
    resizeWindow("Drawing", 640, 480);
    resizeWindow("RGB", 640, 480);
    moveWindow("Drawing", 640, 20);
    moveWindow("RGB", 0, 20);
    ColorTracker tracker;
    Mat drawing, depth, color;
    string base_folder = "imdir/";
    DIR *dir;
    struct dirent *dp;
    dir = opendir((base_folder + "/color/").c_str());
    while ((dp = readdir(dir)) != NULL) {
        string fn = dp->d_name;
        if (fn == "." || fn == "..")
            continue;
        printf("\n>>>>> %s\n", fn.c_str());
        depth = imread(base_folder + "/depth/" + fn, CV_LOAD_IMAGE_GRAYSCALE);
        color = imread(base_folder + "/color/" + fn);
        if (color.rows < 1 || depth.rows < 1)
            continue;
        imshow("RGB", color);

        tracker.find_totes(depth, color, drawing);
        imshow("Drawing", drawing);

        raw = cv::waitKey(0) & 0xFFFF;
        key = raw & 0xFF;
        if ((raw & 0xFF00) == 0xFF00) {
            if (key == 0x53) {
                continue;
            }
        } else if (key == 27) {
            break;
        }
        fflush(stdout);
    }
    closedir(dir);
    cv::destroyAllWindows();
    return 0;
}


int color()
{
    Input input;
    ColorTracker tracker;
    Mat color, draw;
    while (true) {
        input.getBGR(color);
        auto totes = tracker.find_totes(color, color, draw);
        send_udp(totes);
        int key = waitKey(1) & 0xFF;
        if (key == 27)
            break;
        if (key == ' ')
            waitKey(0);
    }
    return 0;
}

int color_oneimage()
{
    ColorTracker tracker;
    Mat image, depth, draw;

    image = cv::imread("/home/connor/robotics/2015/images/multi tote color.jpg");
    tracker.find_totes(depth, image, draw);
    cv::waitKey(0);
    return 0;
}

int basictimer()
{
    pdebug("Starting in basic kinect color capture mode..\n");
    Mat color;
    Input input;
    while (true) {
        profile_start("frame");
        input.getBGR(color);
        imshow("Image", color);
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

int recordimdir()
{
    Mat depth, color;
    Input input;
    bool cameraMode = true;
    int i = 0;
    mkdir("../record", 0755);
    mkdir("../record/depth", 0755);
    mkdir("../record/rgb", 0755);
    namedWindow("Depth");
    namedWindow("RGB");
    while (true) {
        puts("Frame");
        input.getDepth(depth);
        input.getBGR(color);
        imshow("Depth", depth);
        imshow("RGB", color);
        imwrite("../record/depth/img_" + to_string(i) + ".jpg", depth);
        imwrite("../record/rgb/img_" + to_string(i++) + ".jpg", color);
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

int pit_ir_demo()
{
    inputSource = KINECT;
    Input input;
    IRTracker tracker;
    Mat depth, ir, draw;
    int keypress;
    while (keypress != 27) {
        input.getIR(ir);
        tracker.find_totes(depth, ir, draw);
        imshow("Drawing", draw);
        keypress = waitKey(1) & 0xFF;
    }
    return 0;
}

int pit_depth_demo()
{
    inputSource = KINECT;
    Input input;
    DepthTracker tracker;
    Mat depth, color, draw;
    int keypress;
    while (keypress != 27) {
        input.getDepth(depth);
        tracker.find_totes(depth, color, draw);
        imshow("Drawing", draw);
        keypress = waitKey(1) & 0xFF;
    }
    return 0;
}

int pit_color_demo()
{
    inputSource = KINECT;
    Input input;
    ColorTracker tracker;
    Mat depth, color, draw;
    int keypress;
    while (keypress != 27) {
        input.getBGR(color);
        input.getDepth(depth);
        tracker.find_totes(depth, color, draw);
        imshow("Drawing", draw);
        keypress = waitKey(1) & 0xFF;
    }
    return 0;
}

int pit_demo(int argc, char *argv[])
{
    if (argc > 1) {
        if ("depth" == string(argv[1])) {
            return pit_depth_demo();
        } else if ("color" == string(argv[1])) {
            return pit_color_demo();
        }
    }
    return pit_ir_demo();
}

void handle_signal(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
        endprogram = true;
//        exit(5);
}

int robot_main(int argc, char *argv[]);

int main(int argc, char *argv[]) {
//    return colorimdir();
    return robot_main(argc, argv);
}
