#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <opencv2/core/core.hpp>
#include <vector>
#include "yellow.hpp"


typedef std::vector<cv::Point> Contour;

//Returns the distance between two floating points.
double distance(cv::Point2f one, cv::Point2f two);

//Returns the distance between the camera and Point center
double Calculate_Real_Distance(cv::Mat img, cv::Point2f center);

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
double Calculate_Xrot(cv::Point2f center);

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

//Matches logos to boxes and stores them in totes.
std::vector<Game_Piece> Match_logo_totes(cv::Mat img, std::vector<std::vector<cv::Point> > box, std::vector<std::vector<cv::Point> > logo);

float calculate_distance(cv::Point2f center);

void Laplacian( cv::Mat& src, cv::Mat& dst);

double contour_average_distance(cv::Mat image, Contour contour);

double contour_stddev(cv::Mat image, Contour contour);

void seperate_Contours(cv::Mat img, std::vector<cv::Point> contours, std::vector<cv::Point> close, std::vector<cv::Point> back);

void send_udp(std::vector<Game_Piece> pieces);

#endif // FUNCTIONS_H

