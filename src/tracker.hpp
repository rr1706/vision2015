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

const cv::Scalar yellow_tote_min_hsv(0, 60, 90);
const cv::Scalar yellow_tote_max_hsv(31, 255, 255);

const cv::Scalar grey_tote_min_hsv(0, 19, 83);
const cv::Scalar grey_tote_max_hsv(190, 90, 168);
const cv::Scalar grey_tote_min_rgb(125, 142, 140);
const cv::Scalar grey_tote_max_rgb(149, 155, 147);

const cv::Scalar green_bin_min_hsv(0, 0, 0);
const cv::Scalar green_bin_max_hsv(180, 80, 100);
const cv::Scalar green_bin_min_rgb(38, 42, 0);
const cv::Scalar green_bin_max_rgb(59, 63, 13);

const int single_stack_height = 28;  //cm
const int double_stack_height = 58;  //cm
const int triple_stack_height = 88;  //cm
const int quad_stack_height = 112;   //cm
const int penta_stack_height = 142;  //cm
const int hexa_stack_height = 172;   //cm
//measure!
const int bin_height = 65; //cm
const int height_tolerance = 8;  //cm

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
