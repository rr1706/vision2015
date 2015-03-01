#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <opencv2/core/core.hpp>
#include <vector>
#include "yellow.hpp"


typedef std::vector<cv::Point> Contour;

//Returns the distance between two floating points.
double distance(cv::Point2f one, cv::Point2f two);

//Returns the distance between the camera and Point center
double Calculate_Real_Distance(cv::Mat& img, cv::Point2f center);

//Determines the center of the contour passed to it
cv::Point2f Calculate_Center(std::vector<cv::Point> contour);

//Converts degrees to radians
double cvt2rad(double degree);

//Converts radians to degrees
double cvt2degree(double radian);

// Convert a OpenCV 3-byte vector to a scalar (for drawing functions)
cv::Scalar vec2scalar(cv::Vec3b input);

// Convert a Scalar to a 3-byte vector that can be accessed as integers instead of doubles
cv::Vec3b scalar2vec(cv::Scalar input);

//Determines if the object is a yellow tote, gray tote, or green bin
void Determine_Game_Piece(cv::Mat rgb, cv::Point2f center, Game_Piece& unknown_game_piece, cv::Point top, cv::Point bottom);

//Calculates degrees between the point passed
//and the center of the image
double Calculate_Xrot(cv::Mat &img, cv::Point2f center);

//Determines amount of totes stacked
int find_number_of_totes(cv::Mat img, Game_Piece& tote, cv::Point2f center, cv::Point2f height);

//Matches SingleLs, returns vector of YellowTotes
std::vector<YellowTote> pairTotes(std::vector<SingleL> singles);

//Determines if the L is a left or right L
void Calculate_side(SingleL L, cv::Point2f center, cv::Mat img);

//Returns the degree of offset the tote (stack) is to the camera
double find_orientation(cv::Mat img, cv::Point2f left, cv::Point2f closest, cv::Point2f right);

//Returns the furthest left point in a contour
cv::Point get_min_x(cv::Mat img, cv::Rect boundrect, Contour contour);

//Returns the furthest right point in a contour
cv::Point get_max_x(cv::Mat img, cv::Rect boundrect, Contour contour);

//Returns the highest pixel in a contour
cv::Point get_min_y(cv::Mat img, cv::Rect boundrect, Contour contour);

//Returns the lowest pixel in a contour
cv::Point get_max_y(cv::Mat img, cv::Rect boundrect, Contour contour);

//Returns the closest point in a contour
cv::Point get_closest_point(cv::Mat img, std::vector<cv::Point> contour);

//Checks to see if a tote has a green bin on top
bool green_bin_top(cv::Mat img, cv::Point2f top);

//Checks to see if a tote has a tote under it
bool tote_on_bottom(cv::Mat img, cv::Point2f bottom);

//Populates stacked_totes and unstacked_totes with the approapriate detected_totes
void determine_stacked(std::vector<YellowTote> detected_totes, std::vector<std::vector<YellowTote> >& stacked_totes, std::vector<YellowTote>& unstacked_totes, cv::Mat img);

//Print the color data in 3-channel format on a box drawn of the color
void print_color(cv::Mat &img, cv::Scalar color, cv::Point2i location);

//Run inRange for two color schemes and OR them together and return the resulting binary image
cv::Mat multiple_threshold(cv::Mat img, cv::Scalar hsv_min, cv::Scalar hsv_max,
                           cv::Scalar rgb_min, cv::Scalar rgb_max);

// check a single point in the image. faster than multithresh for
// simple point checks because it converts the hsv to rgb and checks
// what it looks like in the BGR color scheme without conversion
bool check_point(cv::Scalar color, cv::Scalar hsv_min, cv::Scalar hsv_max);

//Matches logos to boxes and stores them in totes.
std::vector<Game_Piece> Match_logo_totes(cv::Mat& img, std::vector<Contour> box, std::vector<Contour> logo);

float calculate_distance(cv::Mat& img, cv::Point2f center);

// hunter code
void Laplacian( cv::Mat& src, cv::Mat& dst);

// calculates the average distance to each point along the edge
double contour_average_distance(cv::Mat &image, Contour &contour);

// calculate the standard deviation of the points along the edge
// of the contour using the distance to each point in the supplied
// depth image
double contour_stddev(cv::Mat& image, Contour &contour);

// use standard deviation to remove overlayed objects
void separate_contours(cv::Mat img, std::vector<Contour>& contours);

// send a command to the RoboRIO on UDP, using the largest found piece
// throws exception if it cannot send the message (no network, can't resolve mDNS)
void send_udp(std::vector<Game_Piece> pieces);

// enable profiling for a specific identifier.
void profile_start(std::string id);

// ends profiling for a specific identifier and sets the value in the profiles
// timing storage to be the difference between the current time and the start
// time
void profile_end(std::string id);

// make a nicely printed output on stdout with all registered timings
void profile_print();

void read_config();
void write_config();

void remove_black_borders(cv::Mat &depth);
#endif // FUNCTIONS_H

