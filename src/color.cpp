#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <algorithm>
#include "yellow.hpp"
#include "util.hpp"
#include "tracker.hpp"

using namespace std;
using namespace cv;

vector<YellowTote> ColorTracker::find_totes(Mat img)
{
    std::vector<YellowTote> totes;
    Mat hsv, binary, draw;
    draw = img.clone();
    cvtColor(img, hsv, CV_BGR2HSV);
    inRange(hsv, Scalar(0, 158, 137), Scalar(30, 255, 255), binary);
    erode(binary, binary, kern, Point(-1,-1), 1);
    dilate(binary, binary, kern, Point(-1,-1), 1);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    DEBUG_SHOW("binary image", binary);
    findContours(binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    vector<vector<Point> > polygons(contours.size());
    for (size_t i = 0; i < contours.size(); i++) {
        vector<Point> contour = contours[i];
        double area = contourArea(contour);
        if (area > 100000 || area < 10000) {
            continue;
        }
        drawContours(draw, contours, i, COLOR_RED);
        vector<Point> polygon;
        approxPolyDP(contour, polygon, 5, true);
        polygons[i] = polygon;
        drawContours(draw, polygons, i, COLOR_GREEN);
        Rect bound = boundingRect(polygon);
        //        RotatedRect rotated = minAreaRect(polygon);
        rectangle(draw, bound, COLOR_BLUE);
        //        Point2f vertices[4];
        //        rotated.points(vertices);
        //        for (int i = 0; i < 4; i++) {
        //            line(draw, vertices[i], vertices[(i+1)%4], COLOR_WHITE);
        //        }
        Point2f center;
        // check to see if the contour and the box have similar area
        double areaRatio = area / bound.area();
        double hwRatio = static_cast<double>(bound.height) / bound.width;
        printf("CTR AR:%.2f HW:%.2f\n", areaRatio, hwRatio);
        if (areaRatio > 0.80) {
            // if it is close to 1, then we are seeing a face probably
            // now check the height and width ratio
            if ((hwRatio > 0.6 && hwRatio < 1)) {
                // it's the square short side
                Moments moment = moments(contour, false);
                center = Point2f(moment.m10/moment.m00, moment.m01/moment.m00);
            } else {
                puts("LONG SIDE");
                // it's the long side
                continue; // todo: return xrot as 90
            }
        } else {
            // if not, then we are seeing the box on the corner
            puts("CORNER BOX");
            continue; // possible solution: split the boxes in half
        }
        Point2f centerRebased(center.x - (img.cols / 2.), center.y - (img.rows / 2.));
        // find the x rotation
        double xrot = (centerRebased.x / (img.cols / 2.)) * (fov.x / 2.);// - (fov.x / 2.);
        putText(draw, "XROT: " + to_string(xrot), center, CV_FONT_HERSHEY_PLAIN, 1, COLOR_RED, 1);
        putText(draw, "AR: " + to_string(areaRatio), center + Point2f(0, 20), CV_FONT_HERSHEY_PLAIN, 1, COLOR_RED, 1);
        putText(draw, "HW: " + to_string(hwRatio), center + Point2f(0, 40), CV_FONT_HERSHEY_PLAIN, 1, COLOR_RED, 1);

    }
    fflush(stdout);
    DEBUG_SHOW("Processed", draw);
    return totes;
}
