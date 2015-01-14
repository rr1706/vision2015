#ifndef TRACKER_HPP
#define TRACKER_HPP

#include <opencv2/core/core.hpp>
#include <vector>
#include "yellow.hpp"

#define Image_Width  640
#define Image_Height 480

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

const float single_stack_height = 28;  //cm
const float double_stack_height = 58;  //cm
const float triple_stack_height = 88;  //cm
const float height_tolerance = 5;  //cm

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
