#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "functions.h"
#include "util.hpp"
#include "tracker.hpp"

using namespace cv;
using namespace std;

vector<YellowTote> IRTracker::find_totes(Mat img)
{
    std::vector<SingleL> singles;
    Mat draw;
    cvtColor(img, draw, CV_GRAY2BGR);

    threshold(img, img, 250, 255, CV_THRESH_BINARY);
    dilate(img, img, kern, Point(-1,-1), 3);
    erode(img, img, kern, Point(-1,-1), 1);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(img, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    for (size_t i = 0; i < contours.size(); i++)
    {
        vector<Point> contour = contours[i];

        if (contourArea(contour) > max_area || contourArea(contour) < min_area)
        {
            continue;
        }

        Rect bound = boundingRect(contour);
        rectangle(draw, bound, Scalar(78, 45, 68));

        Point2f massCenter = Calculate_Center(contour);
        Point2f boxCenter = Point2f(bound.x + (bound.width / 2.0), bound.y + (bound.height / 2.0));

        //line(draw, Point2f(boxCenter, 0), Point2f(boxCenter, 480), Scalar(0, 255, 0));
        SingleL single_l;
        single_l.center = massCenter;
        single_l.bound = bound;
        single_l.area = contourArea(contours[i]);

        Calculate_side(single_l, boxCenter, draw);

        singles.push_back(single_l);
    }
    std::vector<YellowTote> detected_totes = pairTotes(singles);

    //split the detected totes into ones that look stacked and ones that aren't
    vector<YellowTote> stacked_totes;
    vector<YellowTote> unstacked_totes;

    //Determine if a tote is stacked or not
    determine_stacked(detected_totes, stacked_totes, unstacked_totes, draw);

    //Figure out xrot and populate it
    if(stacked_totes.size() != 0)
    {
        for(unsigned int i = 0; i < stacked_totes.size(); i++)
        {
            stacked_totes[i].set_xrot(Calculate_Xrot(stacked_totes[i].get_center()));
        }
    }
    for(unsigned int i = 0; i < unstacked_totes.size(); i ++)
    {
        unstacked_totes[i].set_xrot(Calculate_Xrot(unstacked_totes[i].get_center()));
    }

    imshow("Drawing", draw);
    imwrite("Final.jpeg", draw);

    //send game_pieces that do not have the default values to savannah

    return detected_totes;
    //maybe this function should be a void and do all the udp stuff inside the function.
}
