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

vector<Game_Piece> ColorTracker::find_totes(Mat img)
{
    Mat hsv, binary, draw;
    draw = img.clone();

    //todo: do connor's multithresh process here, doesn't have to be before the code release.

    //convert the img from color to hsv
    cvtColor(img, hsv, CV_BGR2HSV);

    //process the image
    //todo: use connor's multithresh to threshold
    inRange(hsv, Scalar(0, 158, 137), Scalar(30, 255, 255), binary);
    erode(binary, binary, kern, Point(-1,-1), 1);
    //dilate(binary, binary, kern, Point(-1,-1), 1);

    DEBUG_SHOW("binary image", binary);

    vector<vector<Point> > contour;
    vector<Vec4i> hierarchy;
    vector<vector<Point> > logo;
    vector<vector<Point> > box;
    findContours(binary, contour, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

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

    printf("box size = %zu\n", box.size());
    printf("logo size = %zu\n", logo.size());

    vector<Game_Piece> totes = Match_logo_totes(draw, box, logo);


    for(size_t i = 0; i < totes.size(); i ++)
    {
        totes[i].set_distance(calculate_distance(totes[i].get_center()));
        Display_Game_Piece(totes[i], draw, totes[i].get_center());
    }

    fflush(stdout);
    DEBUG_SHOW("Processed", draw);
    return totes;
}
