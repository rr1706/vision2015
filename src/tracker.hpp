#ifndef TRACKER_HPP
#define TRACKER_HPP

#include <opencv2/core/core.hpp>
#include <vector>
#include "yellow.hpp"

#define Image_Width  640
#define Image_Height 480

//Tolerances are subject to change

static cv::Mat kern = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3), cv::Point(-1, -1));

const float recylce_bin_ratio = 1.38;
const float single_tote_long_ratio = .50;
const float single_tote_short_ratio = .9;
const float double_tote_long_ratio = 1;
const float double_tote_short_ratio = 1.52;
const float triple_tote_long_ratio= 1.5;
const float triple_tote_short_ratio= 2.24;
const float ratio_tolerance = .05;

//Subject to change
const int max_area = 1000000;
const int min_area = 100;
const cv::Scalar yellow_tote(10,2,90);
const cv::Scalar gray_tote(39,37,42);
const cv::Scalar green_bin(30,40,30);
const int color_tolerance = 10;

const int single_stack_height = 28;  //cm
const int double_stack_height = 58;  //cm
const int triple_stack_height = 88;  //cm
const int quad_stack_height = 112;   //cm
const int penta_stack_height = 142;  //cm
const int hexa_stack_height = 172;   //cm
const int bin_height = 65; //cm
const int height_tolerance = 8;  //cm

const double short_side_distance = 15;
const double long_side_distance = 30;

class ColorTracker {
public:
    std::vector<YellowTote> find_totes(cv::Mat img);
};

class DepthTracker {
public:
    std::vector<Game_Piece> find_pieces(cv::Mat image, int key);
};

class IRTracker {
public:
    std::vector<YellowTote> find_totes(cv::Mat img);
};

#endif // TRACKER_HPP
