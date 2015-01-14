#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <opencv2/core/core.hpp>
#include "yellow.hpp"

double distance(cv::Point2f one, cv::Point2f two);

double Calculate_Real_Distance(cv::Mat img, cv::Point2f center);

cv::Point2f Calculate_Center(std::vector<cv::Point> contour);

void Process_Image(cv::Mat src, cv::Mat dst);

void Calibrate_Image(cv::Mat src, cv::Mat depth, cv::Mat dst);

cv::Scalar Vec3b2Scalar(cv::Vec3b input);

double cvt2rad(double degree);

void Get_Calibration_Image(cv::Mat img, int key);

double Calculate_Xrot(cv::Point2f center);

void Determine_Game_Piece(cv::Point2f center, Game_Piece& unknown_game_piece);

void find_number_of_totes(cv::Mat img, Game_Piece& tote, cv::Point2f center, cv::Point2f height);

std::vector<YellowTote> pairTotes(std::vector<SingleL> singles);

void Calculate_side(SingleL L, cv::Point2f center, cv::Mat img);

void determine_stacked(std::vector<YellowTote>& detected_totes, std::vector<YellowTote>&  stacked_totes, std::vector<YellowTote>& unstacked_totes, cv::Mat img);

#endif // FUNCTIONS_H

