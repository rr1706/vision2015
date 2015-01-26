#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "functions.h"
#include "util.hpp"
#include "tracker.hpp"

using namespace cv;
using namespace std;

vector<YellowTote> IRTracker::find_totes(Mat img, Mat& draw)
{
    std::vector<SingleL> singles;

    char str[50];

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

        //todo:check to make sure a contour isn't an overlapping object

        Rect bound = boundingRect(contour);
        rectangle(draw, bound, Scalar(78, 45, 68));

        Point2f massCenter = Calculate_Center(contour);
        Point2f boxCenter = Point2f(bound.x + (bound.width / 2.0), bound.y + (bound.height / 2.0));

        //declare and populate a SingleL with the appropriate values
        SingleL single_l;
        single_l.center = massCenter;
        single_l.bound = bound;
        single_l.area = contourArea(contours[i]);

        Calculate_side(single_l, boxCenter, draw);

        singles.push_back(single_l);
    }
    
    std::vector<YellowTote> detected_totes = pairTotes(singles);

    //split the detected totes into ones that look stacked and ones that aren't
    vector<vector<YellowTote> > stacked_totes;
    vector<YellowTote> unstacked_totes;

    //Determine if a tote is stacked or not
    determine_stacked(detected_totes, stacked_totes, unstacked_totes, draw);

    //populate xrot for stacked_totes
    for(unsigned int i = 0; i < stacked_totes.size(); i++)
    {
        for(unsigned int j = 0; j < stacked_totes[i].size(); j++)
        {
            stacked_totes[i][j].set_xrot(Calculate_Xrot(stacked_totes[i][j].get_center()));
        }
    }

    //populate xrot for unstacked totes
    for(unsigned int i = 0; i < unstacked_totes.size(); i ++)
    {
        unstacked_totes[i].set_xrot(Calculate_Xrot(unstacked_totes[i].get_center()));
    }

    //Display stacked_totes
    for(unsigned int i = 0; i < stacked_totes.size(); i++)
    {
        for(unsigned int j = 0; j < stacked_totes[i].size(); j++)
        {
            Display_YellowTote(stacked_totes[i][j], draw, stacked_totes[i][j].get_center());
        }
    }
    //display unstacked_totes
    for(unsigned int i = 0; i < unstacked_totes.size(); i++)
    {
        Display_YellowTote(unstacked_totes[i], draw, unstacked_totes[i].get_center());
    }

    //Branding :D
    sprintf(str, "Ratchet Rockers 1706");
    putText(draw, str,Point(1100, 15), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);

    //send stacked_totes[i][max] and all of unstacked_totes to RoboRio

    imshow("Drawing", draw);
    return detected_totes;
}
