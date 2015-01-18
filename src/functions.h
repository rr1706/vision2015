#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <opencv2/core/core.hpp>
#include "yellow.hpp"


typedef std::vector<cv::Point> Contour;

//Returns the distance between two floating points.
double distance(cv::Point2f one, cv::Point2f two);

//Returns the distance between the camera and Point center
double Calculate_Real_Distance(cv::Mat img, cv::Point2f center);

//Determines the center of the contour passed to it
cv::Point2f Calculate_Center(std::vector<cv::Point> contour);

//Calibrates the image such that the constant
//environment is removed, leaving only the new
//objects.
void Calibrate_Image(cv::Mat src, cv::Mat depth, cv::Mat dst);

//Converts degrees to radians
double cvt2rad(double degree);

//Converts radians to degrees
double cvt2degree(double radian);

//Determines if the object is a yellow tote, gray tote, or green bin
void Determine_Game_Piece(cv::Mat rgb, cv::Point2f center, Game_Piece& unknown_game_piece, cv::Point top, cv::Point bottom);

//Captures current frame for the new calibration image
void Get_Calibration_Image(cv::Mat img, int key);

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
cv::Point get_min_x(cv::Mat img, cv::Rect boundrect);

//Returns the furthest right point in a contour
cv::Point get_max_x(cv::Mat img, cv::Rect boundrect);

//Returns the highest pixel in a contour
cv::Point get_min_y(cv::Rect boundrect);

//Returns the lowest pixel in a contour
cv::Point get_max_y(cv::Rect boundrect);

//Returns the closest point in a contour
cv::Point get_closest_point(cv::Mat img, std::vector<cv::Point> contour);

//Checks to see if a tote has a green bin on top
bool green_bin_top(cv::Mat img, cv::Point2f top);

//Checks to see if a tote has a tote under it
bool tote_on_bottom(cv::Mat img, cv::Point2f bottom);

//Populates stacked_totes and unstacked_totes with the approapriate detected_totes
void determine_stacked(std::vector<YellowTote> detected_totes, std::vector<std::vector<YellowTote> >& stacked_totes, std::vector<YellowTote>& unstacked_totes, cv::Mat img);

#endif // FUNCTIONS_H

