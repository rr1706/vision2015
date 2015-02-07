#ifndef TRACKER_HPP
#define TRACKER_HPP

#include <opencv2/core/core.hpp>
#include <vector>
#include "yellow.hpp"

#define Image_Width  640
#define Image_Height 480

//Tolerances are subject to change

static cv::Mat kern = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3), cv::Point(-1, -1));

const float camera_height = 50; //in
const float camera_angle = 15; //degrees
const float tote_center_height = 5; //in
const float adjacent = camera_height - tote_center_height; //in

//Subject to change
const int max_area = 1000000;
const int min_area = 100;

const cv::Scalar yellow_tote_min_hsv(  5, 158,  40);
const cv::Scalar yellow_tote_max_hsv( 35, 255, 255);

const cv::Scalar grey_tote_min_hsv( 92,  24,   0);
const cv::Scalar grey_tote_max_hsv(200, 159,  92);
const cv::Scalar grey_tote_min_rgb(  0,   0,   0);
const cv::Scalar grey_tote_max_rgb(  0,   0,   0);

const cv::Scalar green_bin_min_hsv(  0,   0,  39);
const cv::Scalar green_bin_max_hsv(255, 205,  75);
const cv::Scalar green_bin_min_rgb(  0,   0,   0);
const cv::Scalar green_bin_max_rgb(  0,   0,   0);

const int single_stack_height = 31.5;  //cm
const int double_stack_height = 61;  //cm
const int triple_stack_height = 91;  //cm
const int quad_stack_height = 121;   //cm
const int penta_stack_height = 149;  //cm
const int hexa_stack_height = 181;   //cm
//measure!
const int bin_height = 65; //cm
const int height_tolerance = 12;  //cm

const double short_side_distance = 15;
const double long_side_distance = 30;
// changes the area around which a edge of a contour is checked for close points containing distance
const int searchbuffer = 15;

class ColorTracker {
public:
    std::vector<Game_Piece> find_totes(cv::Mat img);
};

class DepthTracker {
public:
    std::vector<Game_Piece> find_pieces(cv::Mat image, cv::Mat rgb, cv::Mat &output);
};

class IRTracker {
public:
    std::vector<YellowTote> find_totes(cv::Mat img, cv::Mat &draw);
};

#endif // TRACKER_HPP
