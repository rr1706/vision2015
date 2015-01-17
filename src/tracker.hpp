#ifndef TRACKER_HPP
#define TRACKER_HPP

#include <opencv2/core/core.hpp>
#include <vector>
#include "yellow.hpp"

#define Image_Width  640
#define Image_Height 480

//Tolerances are subject to change

static cv::Mat kern = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3), cv::Point(-1, -1));

//Subject to change
const int max_area = 1000000;
const int min_area = 100;
const cv::Scalar yellow_tote(10,5,80);
const cv::Scalar gray_tote(30,30,30);
const cv::Scalar green_bin(50,50,50);
const int color_tolerance = 80;

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
