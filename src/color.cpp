#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <algorithm>
#include "functions.h"
#include "yellow.hpp"
#include "util.hpp"
#include "tracker.hpp"

using namespace std;
using namespace cv;

vector<YellowTote> ColorTracker::find_totes(Mat img)
{
    vector<YellowTote> totes;

    Mat hsv, binary, draw;
    draw = img.clone();

    //convert the img from color to hsv
    cvtColor(img, hsv, CV_BGR2HSV);

    //process the image
    //todo: use connor's multithresh to threshold
    inRange(hsv, Scalar(0, 158, 137), Scalar(30, 255, 255), binary);
    erode(binary, binary, kern, Point(-1,-1), 1);
    //dilate(binary, binary, kern, Point(-1,-1), 1);

    DEBUG_SHOW("binary image", binary);

    vector<vector<Point> > contour;
    vector<vector<Point> > logo;
    vector<vector<Point> > box;
    vector<Vec4i> hierarchy;
    findContours(binary, contour, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    for (size_t i = 0; i < contour.size(); i++)
    {
        //If the contour is too small, throw it out
        //play around with this value
        if (contourArea(contour[i]) < 150)
        {
            continue;
        }

        //store totes in the vvP box and logos in vvP logo
        if(contourArea(contour[i]) > 5000) //tote
        {
            box.push_back(contour[i]);
        }
        else // logo
        {
            logo.push_back(contour[i]);
        }
    }

    Match_logo_totes(draw, box, logo, totes);

    for(size_t i = 0; i < totes.size(); i ++)
    {
        Display_YellowTote(totes[i], draw, totes[i].get_center());
    }

    fflush(stdout);
    DEBUG_SHOW("Processed", draw);
    return totes;
}
