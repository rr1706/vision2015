#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <algorithm>
#include "functions.hpp"
#include "yellow.hpp"
#include "util.hpp"
#include "tracker.hpp"

using namespace std;
using namespace cv;

vector<Game_Piece> ColorTracker::find_totes(cv::Mat depth, cv::Mat img, cv::Mat &draw)
{
    Mat hsv, binary;
    img.copyTo(draw);
    profile_start("filter");

    //todo: do connor's multithresh process here, doesn't have to be before the code release.

    //convert the img from color to hsv
    cvtColor(img, hsv, CV_BGR2HSV);

    //process the image
    //todo: use connor's multithresh to threshold
    inRange(hsv, yellow_tote_min_hsv, yellow_tote_max_hsv, binary);
    erode(binary, binary, kern, Point(-1,-1), 1);
    //dilate(binary, binary, kern, Point(-1,-1), 1);

    vector<vector<Point> > contour;
    vector<Vec4i> hierarchy;
    vector<vector<Point> > logo;
    vector<vector<Point> > box;
    findContours(binary, contour, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    profile_end("filter");

    for (size_t i = 0; i < contour.size(); i++)
    {
        //If the contour is too small, throw it out
        //play around with this value
        if (contourArea(contour[i]) < 100)
        {
            continue;
        }

        //store totes in the vvP box and logos in vvP logo
        if(contourArea(contour[i]) > 4000) //tote
        {
            box.push_back(contour[i]);
        }
        else // logo
        {
            logo.push_back(contour[i]);
        }
    }

    profile_start("match");
    vector<Game_Piece> totes = Match_logo_totes(draw, box, logo);
    profile_end("match");

    profile_start("draw");
    if (DRAW) {
        drawContours(draw, box, -1, COLOR_RED, 1, 8);
    }
    for(size_t i = 0; i < totes.size(); i ++)
    {
        Point2i p1 = totes[i].get_center();
        Point2i p2 = p1 + Point2i(20, 0);
        Point2i p3 = p1 + Point2i(20, 0);
        Scalar i2 = depth.at<uchar>(p2);
        Scalar i3 = depth.at<uchar>(p3);
        if (i3[0] < 255 && i3[0] > 0) {
            totes[i].set_distance(Calculate_Real_Distance(depth, p3));
        } else if (i2[0] < 255 && i2[0] > 0) {
            totes[i].set_distance(Calculate_Real_Distance(depth, p2));
        } else {
            totes[i].set_distance(Calculate_Real_Distance(depth, totes[i].get_center()));
        }
        if (DRAW) {
            Display_Game_Piece(totes[i], draw, totes[i].get_center());
        }
    }
    profile_end("draw");
    return totes;
}
