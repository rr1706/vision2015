#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <algorithm>
#include "yellow.hpp"
#include "util.hpp"

using namespace std;
using namespace cv;

//Class Game Piece functions
Game_Piece::Game_Piece() : x_rot(0), distance(-1), rotation(0),
    totes_high(-1), piece_type(0), green_bin_top(false), center(-1, -1)
{
}

//Mutalators
void Game_Piece::set_xrot(float rotation)
{
    x_rot = rotation;
}

void Game_Piece::set_distance(float d)
{
    distance = d;
}

void Game_Piece::set_rotation(float rot)
{
    rotation = rot;
}

void Game_Piece::set_totes_high(int stacks)
{
    totes_high = stacks;
}

void Game_Piece::set_piece_type(int type)
{
    piece_type = type;
}

void Game_Piece::set_green_bin(bool on_top)
{
    green_bin_top = on_top;
}

void Game_Piece::set_center(Point2f center)
{
    this->center = center;
}

//Accessors
float Game_Piece::get_distance()
{
    return distance;
}

float Game_Piece::get_xrot()
{
    return x_rot;
}

float Game_Piece::get_rotation()
{
    return rotation;
}

int Game_Piece::get_totes_high()
{
    return totes_high;
}

int Game_Piece::get_piece_type()
{
    return piece_type;
}

bool Game_Piece::get_green_bin()
{
    return green_bin_top;
}

std::string Game_Piece::get_piece_type_str()
{
    switch (get_piece_type()) {
    case OBJECT_GREY_TOTE:
        return "Grey Tote";
    case OBJECT_YELLOW_TOTE:
        return "Yellow Tote";
    case OBJECT_GREEN_BIN:
        return "Green Bin";
    default:
        return "Unknown";
    }
}

cv::Point2f Game_Piece::get_center()
{
    return this->center;
}

void Display_Game_Piece(Game_Piece object, Mat img, Point origin)
{
    origin = origin - Point(100, 50);
    char str[50];
    sprintf(str, "Ratchet Rockers 1706");
    putText(img, str,Point(20, 20), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);

    sprintf(str, "XRot  = %.2f", object.get_xrot());
    putText(img, str,Point(origin.x+15, origin.y+20), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);

    sprintf(str, "Distance = %.2f",object.get_distance());
    putText(img, str,Point(origin.x+15, origin.y+40), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);

    sprintf(str, "Rotation = %.2f",object.get_rotation());
    putText(img, str,Point(origin.x+15, origin.y+60), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);

    sprintf(str, "Type = %s", object.get_piece_type_str().c_str());
    putText(img, str,Point(origin.x+15, origin.y+80), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);

    sprintf(str, "Totes high = %d",object.get_totes_high());
    putText(img, str,Point(origin.x+15, origin.y+100), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);

    sprintf(str, "Green bin top? = %d",object.get_green_bin());
    putText(img, str,Point(origin.x+15, origin.y+120), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);
}

//Class Yellow Tote functions
YellowTote::YellowTote(Side side) :  area_ratio(-1), facing_Side(side), xrot(-99), stack_height(-1), center(cv::Point2f(-999,-999))
{
}

//mutalators
void YellowTote::set_side(Side s)
{
    facing_Side = s;
}

void YellowTote::set_ratio(double ratio)
{
    area_ratio = ratio;
}

void YellowTote::set_xrot(double rotation)
{
    xrot = rotation;
}

void YellowTote::set_stacked(int stacked)
{
    stack_height = stacked;
}

void YellowTote::set_center(Point2f c)
{
    center.x = c.x;
    center.y = c.y;
}

void YellowTote::set_offset(double offset)
{
    this->offset = offset;
}

void YellowTote::set_distance(double distance)
{
    this->distance = distance;
}

//accessors
Side YellowTote::get_side()
{
    return facing_Side;
}

double YellowTote::get_ratio()
{
    return area_ratio;
}

double YellowTote::get_xrot()
{
    return xrot;
}

int YellowTote::get_stacked()
{
    return stack_height;
}

cv::Point2f YellowTote::get_center()
{
    return center;
}

float YellowTote::get_center_x()
{
    return center.x;
}

float YellowTote::get_center_y()
{
    return center.y;
}

double YellowTote::get_offset()
{
    return offset;
}

double YellowTote::get_distance()
{
    return distance;
}

bool operator==(YellowTote& one, YellowTote& two)
{
    return one.get_center_x() == two.get_center_x() && one.get_center_y() == two.get_center_y();
}

void Display_YellowTote(YellowTote tote, Mat img, Point origin)
{
    char str[50];

    sprintf(str, "Ratio = %.2f",tote.get_ratio());
    putText(img, str,Point(origin.x+15, origin.y+35), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);

    sprintf(str, "xrot = %.2f",tote.get_xrot());
    putText(img, str,Point(origin.x+15, origin.y+55), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);

    sprintf(str, "Stacked = %d",tote.get_stacked());
    putText(img, str,Point(origin.x+15, origin.y+75), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_BLUE,1,8,false);
}

YellowTote YellowTote::operator=(YellowTote tote)
{
    center = tote.get_center();
    area_ratio = tote.get_ratio();
    facing_Side = tote.get_side();
    xrot = tote.get_xrot();
    stack_height = tote.get_stacked();
    return *this;
}

YellowTote::YellowTote()
{
    this->area_ratio = 0;
    this->center = Point2f(0, 0);
    this->facing_Side = UNK_SIDE;
    this->stack_height = 0;
    this->xrot = 180;

}

bool operator !=(YellowTote& one, YellowTote& two)
{
    return !(one == two);
}
