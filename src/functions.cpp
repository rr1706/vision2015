#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdint.h>
#include <ctime>
#include <map>
#include <chrono>
#include <hsv.hpp>
#include <input.hpp>
#include "util.hpp"
#include "yellow.hpp"
#include "functions.h"
#include "tracker.hpp"
#include "udpsender.hpp"
#include "solutionlog.hpp"

using namespace cv;
using namespace std;

double distance(Point2f one, Point2f two)
{
    return sqrt(pow(one.x - two.x, 2) + pow(one.y - two.y, 2));
}

double distance1D(double one, double two)
{
    return abs(one - two);
}

double Calculate_Real_Distance(Mat &img, Point2f center) {
    Scalar intensity = img.at<uchar>(center);
    if (inputSource == KINECT) {
        return 0.1236 * tan(intensity[0]*4 / 2842.5 + 1.1863)*100;
    } else if (inputSource == XTION) {
        return (1.6852 * intensity[0] + 50.521) * 2.54;
    }
}

Point2f Calculate_Center(vector<Point> contour)
{
    Moments mu;
    mu = moments(contour, false);
    return Point2f(mu.m10/mu.m00, mu.m01/mu.m00);
}

double cvt2rad(double degree)
{
    return degree * CV_PI / 180;
}

double cvt2degree(double radian)
{
    return radian*180/CV_PI;
}

Scalar vec2scalar(Vec3b input)
{
    uint8_t r = input[0];
    uint8_t g = input[1];
    uint8_t b = input[2];
    return Scalar(r, g, b);
}

Vec3b scalar2vec(Scalar input)
{
    uint8_t r = input[0];
    uint8_t g = input[1];
    uint8_t b = input[2];
    return Vec3b(r, g, b);
}

void Determine_Game_Piece(Mat img, Point2f, Game_Piece& unknown_game_piece, Point top, Point bottom)
{
    Point toteCheckpoint = bottom - Point(0, 20);
    if (toteCheckpoint.y < 0)
        toteCheckpoint.y = 0;
    Point binCheckpoint = top + Point(0, 20);
    if (binCheckpoint.y >= img.rows)
        binCheckpoint.y = img.rows - 1;

    Scalar colour = vec2scalar(img.at<Vec3b>(toteCheckpoint));
    Scalar rgb(colour[2], colour[1], colour[0]);
    Scalar hsv = rgb2hsv(rgb);
    pdebug("B: %f G: %f R: %f\nR: %f G: %f B: %f\nH: %f S: %f V: %f\n", colour[0], colour[1], colour[2], rgb[0], rgb[1], rgb[2], hsv[0], hsv[1], hsv[2]);

    if (check_point(vec2scalar(img.at<Vec3b>(toteCheckpoint)), yellow_tote_min_hsv, yellow_tote_max_hsv)) {
        unknown_game_piece.set_piece_type(OBJECT_YELLOW_TOTE);
    } else if (check_point(vec2scalar(img.at<Vec3b>(toteCheckpoint)), grey_tote_min_hsv, grey_tote_max_hsv)) {
        unknown_game_piece.set_piece_type(OBJECT_GREY_TOTE);
    } else if (check_point(vec2scalar(img.at<Vec3b>(toteCheckpoint)), green_bin_min_hsv, green_bin_max_hsv)) {
        unknown_game_piece.set_piece_type(OBJECT_GREEN_BIN);
    } else {
        unknown_game_piece.set_piece_type(OBJECT_UNKNOWN);
    }

    if(unknown_game_piece.get_piece_type() == OBJECT_GREY_TOTE && green_bin_top(img, binCheckpoint))
    {
        unknown_game_piece.set_green_bin(true);
    }
    if(unknown_game_piece.get_piece_type() == OBJECT_GREEN_BIN && tote_on_bottom(img, toteCheckpoint))
    {
        unknown_game_piece.set_piece_type(OBJECT_GREY_TOTE);
    }

    cvtColor(img, img, CV_BGR2HSV);
    Vec3b color = img.at<Vec3b>(toteCheckpoint);
    Vec3b top_color = img.at<Vec3b>(binCheckpoint);

    print_color(img, vec2scalar(color), toteCheckpoint);
    print_color(img, vec2scalar(top_color), binCheckpoint);
    cvtColor(img, img, CV_HSV2BGR);

    imshow("RGB", img);
}

int find_number_of_totes(Mat img, Game_Piece& tote, Point2f center, Point2f height)
{
    double diff = abs(center.y - height.y);
    double yrot = diff*fov.y/(img.rows);
    yrot = cvt2rad(yrot);
    Scalar intensity = img.at<uchar>(height);
    float distance_to_top = 0.1236 * tan(intensity[0]*4 / 2842.5 + 1.1863)*100;
    float real_height =  2*sin(yrot)*distance_to_top;
    printf("real height = %.2fcm\n", real_height);
    //Check heights
    if(tote.get_green_bin() == false) //no green bin on top
    {
        if(real_height <= single_stack_height + height_tolerance && real_height >= single_stack_height - height_tolerance)
        {
            return 1;
        }
        else if(real_height <= double_stack_height + height_tolerance && real_height >= double_stack_height - height_tolerance)
        {
            return 2;
        }
        else if(real_height <= triple_stack_height + height_tolerance && real_height >= triple_stack_height - height_tolerance)
        {
            return 3;
        }
        else if(real_height <= quad_stack_height + height_tolerance && real_height >= quad_stack_height - height_tolerance)
        {
            return 4;
        }
        else if(real_height <= penta_stack_height + height_tolerance && real_height >= penta_stack_height - height_tolerance)
        {
            return 5;
        }
        else if(real_height <= hexa_stack_height + height_tolerance && real_height >= hexa_stack_height - height_tolerance)
        {
            return 6;
        }
    }
    else //green bin on top
    {
        if(real_height <= single_stack_height + bin_height +height_tolerance &&
                real_height >= single_stack_height + bin_height - height_tolerance)
        {
            return 1;
        }
        else if(real_height <= double_stack_height + bin_height + height_tolerance &&
                real_height >= double_stack_height + bin_height - height_tolerance)
        {
            return 2;
        }
        else if(real_height <= triple_stack_height + bin_height + height_tolerance &&
                real_height >= triple_stack_height + bin_height - height_tolerance)
        {
            return 3;
        }
        else if(real_height <= quad_stack_height + bin_height + height_tolerance &&
                real_height >= quad_stack_height + bin_height - height_tolerance)
        {
            return 4;
        }
        else if(real_height <= penta_stack_height + bin_height + height_tolerance &&
                real_height >= penta_stack_height + bin_height - height_tolerance)
        {
            return 5;
        }
        else if(real_height <= hexa_stack_height + bin_height + height_tolerance &&
                real_height >= hexa_stack_height + bin_height - height_tolerance)
        {
            return 6;
        }
    }
    return -1;
}

double Calculate_Xrot(Mat& img, Point2f center)
{
    //remap the center from top left to center of top
    center.x = (center.x - img.cols/2);

    // Calculate angle to center of box
    double Xrot = center.x/(img.cols/2)*fov.x/2;
    center.x = (center.x + img.cols/2);

    return Xrot;
}

void Calculate_side(SingleL L, Point2f center, Mat img)
{
    if (center.x - L.center.x > 0)
    {
        // closer to left side
        putText(img, "R", Point2f(center.x + 10, center.y - 10), CV_FONT_HERSHEY_SIMPLEX, .75, COLOR_BLUE, 2);
        L.side = RIGHT_SIDE;
    } else
    {
        putText(img, "L", Point2f(center.x - 15, center.y - 10), CV_FONT_HERSHEY_SIMPLEX, .75, COLOR_BLUE, 2);
        L.side = LEFT_SIDE;
    }
    circle(img, center, 3, COLOR_BLUE, 2);
    return;
}

void determine_stacked( vector<YellowTote> detected_totes, vector< vector<YellowTote> >& stacked_totes, vector<YellowTote>& unstacked_totes, Mat img)
{
    /*
     * stacked totes description:
     * first vector is a list of the tote stacks detected
     * the second interior vector is all the totes in a single stack
     */
    for (size_t tote_i = 0; tote_i < detected_totes.size(); tote_i++)
    {
        int level = 1;
        vector<YellowTote> stack;
        for (size_t other_i =  tote_i + 1; other_i < detected_totes.size(); other_i++)
        {
            if (abs(detected_totes[tote_i].get_center_x() - detected_totes[other_i].get_center_x()) < 20)
            {
                if (detected_totes[tote_i].get_stacked() == -1)
                {
                    detected_totes[tote_i].set_stacked(level);
                    stack.push_back(detected_totes[tote_i]);
                    level += 1;
                }
                if (detected_totes[other_i].get_stacked() == -1)
                {
                    detected_totes[other_i].set_stacked(level);
                    stack.push_back(detected_totes[other_i]);
                    level++;
                }
            }
        }
        if (!stack.empty()) {
            stacked_totes.push_back(stack);
        }
    }
    for (unsigned int i = 0; i < detected_totes.size(); i ++)
    {
        if(detected_totes[i].get_stacked() == -1)
        {
            circle(img, Point(detected_totes[i].get_center_x(), detected_totes[i].get_center_y()), 3, COLOR_BLUE);
            detected_totes[i].set_stacked(1);
            unstacked_totes.push_back(detected_totes[i]);
        }
    }

    //draw a line down the middle of the stack if one exists
    for(unsigned int i = 0; i < stacked_totes.size(); i++)
    {
        for(unsigned int j = 0; j < stacked_totes[i].size()-1; j++)
        {
            circle(img, stacked_totes[i][j].get_center(), 3, COLOR_BLUE);
            line(img, stacked_totes[i][j].get_center(), stacked_totes[i][j+1].get_center(), COLOR_BLUE, 2);
        }
    }
}

const int movement = 10;

inline Point moved_point(Point pt, Point center)
{
    int offsetX, offsetY;
    if (pt.x < center.x) {
        offsetX = 1;
    } else {
        offsetX = -1;
    }
    if (pt.y < center.y) {
        offsetY = 1;
    } else {
        offsetY = -1;
    }
    return Point2i(pt.x + (offsetX * movement), pt.y + (offsetY * movement));
}

double contour_average_distance(Mat& image, Contour& contour)
{
    Moments moment = moments(contour, false);
    Point2f center = Point2f(moment.m10/moment.m00, moment.m01/moment.m00);
    double average = 0, distance;
    for (Point pt : contour) {
        distance = Calculate_Real_Distance(image, moved_point(pt, center));
        average += distance;
    }
    return average / contour.size();
}

double contour_stddev(Mat &image, Contour& contour)
{
    double dist, diff_mean;
    double ave_dist = contour_average_distance(image, contour);
    double variance = 0;
    Moments moment = moments(contour, false);
    Point2f center = Point2f(moment.m10/moment.m00, moment.m01/moment.m00);
    for (Point pt : contour) {
        dist = Calculate_Real_Distance(image, moved_point(pt, center));
        diff_mean = ave_dist - dist;
        variance += pow(diff_mean, 2);
    }
    variance = variance / contour.size();
    return sqrt(variance);
}

//calculate average distance of the pixels above the average distance
//calclate the standard deviation.
//save every point that is within two s.ds of this average to a vector point closer
//calculate the average distance of the pixels below the average distance
//calculate the standard deviation of this value
//save every point that is within two s.ds of this average to a vector point back
void separate_contours(Mat img, vector<Contour>& contours)
{
    double ave, stddev, dist;
    vector<Contour> newContours;
    for (Contour contour : contours) {
        ave = contour_average_distance(img, contour);
        stddev = contour_stddev(img, contour);
        Moments moment = moments(contour, false);
        Point2f center = Point2f(moment.m10/moment.m00, moment.m01/moment.m00);
        Contour newContour;
        for (Point pt : contour) {
            dist = Calculate_Real_Distance(img, moved_point(pt, center));
            if (abs(dist - ave) < stddev) {
                newContour.push_back(pt);
            }
        }
        if (!newContour.empty() && contourArea(newContour) > 2500 && contourArea(newContour) < 500000) {
            newContours.push_back(newContour);
        }
    }
    contours.swap(newContours);
}

//someone make a document with a step by step process of this math
//and put it somewhere online so we can link to it in a comment.
double find_orientation(Mat img, Point2f left, Point2f closest, Point2f right)
{
    double theta_6 = -99;

    double d1 = Calculate_Real_Distance(img, closest);
    double d2 = Calculate_Real_Distance(img, right);
    double theta = (right.x - closest.x) *fov.x / (img.cols);
    double c = sqrt(pow(d1, 2) + pow(d2, 2) - (2 * d1 * d2 * cos(cvt2rad(theta))));
    printf("c = %.2f\n", c);

    if (c < 10) {
        theta_6 = 0;
    } else if (c > 100) {
        theta_6 = 90;
    } else if (c > 40) {
        double d_2 = Calculate_Real_Distance(img, closest);
        double d_3 = Calculate_Real_Distance(img, left);

        printf("d2 = %.2f\n", d_2);
        printf("d3 = %.2f\n", d_3);

        double theta_2 = ((left.x - closest.x) * fov.x / (img.cols));
        printf("theta2 = %.2f\n", theta_2);

        double d_6 = cos(cvt2rad(theta_2))*d_2;
        printf("d6 = %.2f\n", d_6);

        double d_7 = tan(cvt2rad(theta_2))*d_2;
        printf("d7 = %.2f\n", d_7);

        double d_8 = d_3 - d_6;
        printf("d8 = %.2f\n", d_8);

        theta_6 = cvt2degree(atan(d_8/d_7));
    } else {
        double d_2 = Calculate_Real_Distance(img, closest);
        double d_3 = Calculate_Real_Distance(img, right);

        printf("d2 = %.2f\n", d_2);
        printf("d3 = %.2f\n", d_3);

        double theta_2 = ((right.x - closest.x) * fov.x / (img.cols));
        printf("theta2 = %.2f\n", theta_2);

        double d_6 = cos(cvt2rad(theta_2))*d_2;
        printf("d6 = %.2f\n", d_6);

        double d_7 = tan(cvt2rad(theta_2))*d_2;
        printf("d7 = %.2f\n", d_7);

        double d_8 = d_3 - d_6;
        printf("d8 = %.2f\n", d_8);

        theta_6 = cvt2degree(atan(d_8/d_7));
    }

    return theta_6;

}

Point get_min_x(Mat img, Rect boundrect, Contour contour)
{
    int x;
    int y = boundrect.y + boundrect.height / 2;
    Scalar sclr;
    for (x = boundrect.x; x < boundrect.x + 40; x++) {
        if (x > img.cols)
            continue;
        sclr = img.at<uchar>(Point(x, y));
        if (sclr[0] < 255 && cv::pointPolygonTest(contour, Point(x, y), false) >= 0) {
            break;
        }
    }
    return Point(x, y);
}

Point get_max_x(Mat img, Rect boundrect, Contour contour)
{
    int x;
    int y = boundrect.y + boundrect.height / 2;
    Scalar sclr;
    for (x = boundrect.x + boundrect.width; x > boundrect.x + boundrect.width - 40; x--) {
        if (x < 0)
            continue;
        sclr = img.at<uchar>(Point(x, y));
        if (sclr[0] < 255 && cv::pointPolygonTest(contour, Point(x, y), false) >= 0) {
            break;
        }
    }
    return Point(x, y);
}

Point get_min_y(Mat img, Rect boundrect, Contour contour)
{
    int x = boundrect.x + (boundrect.width / 2);
    int y;
    Scalar sclr;
    for (y = boundrect.y; y < boundrect.y + 40; y++) {
        if (y > img.rows)
            continue;
        sclr = img.at<uchar>(Point(x, y));
        if (sclr[0] < 255 && cv::pointPolygonTest(contour, Point(x, y), false) >= 0) {
            break;
        }
    }
    return Point(x, y);
}

Point get_max_y(Mat img, Rect boundrect, Contour contour)
{
    int x = boundrect.x + (boundrect.width / 2);
    int y;
    Scalar sclr;
    for (y = boundrect.y + boundrect.height; y > boundrect.y + boundrect.height - 40; y--) {
        if (y < 0)
            continue;
        sclr = img.at<uchar>(Point(x, y));
        if (sclr[0] < 255 && cv::pointPolygonTest(contour, Point(x, y), false) >= 0) {
            break;
        }
    }
    return Point(x, y);
}

Point get_closest_point(Mat img, vector<Point> contour)
{
    Point closest_point = Point(-1,-1);
    double closest = 255;
    for(unsigned int i = 0; i < contour.size(); i++)
    {

        double intensity = 255;
        int x = contour[i].x;
        int y;
        for (y = contour[i].y; y < contour[i].y + searchbuffer; y++) {
            if (x < 0 || y < 0 || x > img.cols || y > img.rows)
                continue;
            Scalar sclr = img.at<uchar>(Point(x, y));
            if (sclr[0] < 255) {
                intensity = sclr[0];
                break;
            }
        }
        if(intensity < closest)
        {
            closest = intensity;
            closest_point = Point(x, y);
        }
    }
    return closest_point;
}

bool green_bin_top(Mat img, Point2f top)
{
    return check_point(vec2scalar(img.at<Vec3b>(top)), green_bin_min_hsv, green_bin_max_hsv);
}

bool tote_on_bottom(Mat img, Point2f bottom)
{
    return check_point(vec2scalar(img.at<Vec3b>(bottom)), grey_tote_min_hsv, grey_tote_max_hsv);
}

vector<YellowTote> pairTotes(vector<SingleL> singles)
{
    std::vector<YellowTote> detected_totes;
    for (size_t i = 0; i < singles.size(); i++)
    {
        if (singles[i].paired)
            continue;
        for (size_t j = i + 1; j < singles.size(); j++)
        {
            if (singles[j].paired)
                continue;
            int dist = distance(singles[i].center, singles[j].center);
            int xdist = abs(singles[i].center.x - singles[j].center.x);
            if (dist < singles[i].bound.height * 2 && xdist > 15)
            {
                singles[j].paired = true;
                singles[i].paired = true;
                YellowTote tote(LONG_SIDE);
                //populate the values we can in our totes based off what we
                //know from the L's they are constructed from
                if(singles[i].side == LEFT_SIDE)
                {
                    tote.set_ratio(singles[i].area/singles[j].area);
                    tote.set_center(Point2f((singles[i].center.x + singles[j].center.x)/2,(singles[i].center.y + singles[j].center.y)/2));
                }
                else
                {
                    tote.set_ratio(singles[j].area/singles[i].area);
                    tote.set_center(Point2f((singles[i].center.x + singles[j].center.x)/2,(singles[i].center.y + singles[j].center.y)/2));
                }
                detected_totes.push_back(tote);
                break;
            }
        }
    }


    return detected_totes;
}

void print_color(Mat &img, Scalar color, Point2i location)
{
    char colorStr[255];
    Rect rekt(location.x - 5, location.y - 20, 120, 30);
    Vec3b saneColor = scalar2vec(color);
    // warn: if color is RGB then it is going to draw an inverted color rectangle :)
    rectangle(img, rekt, color, CV_FILLED);
    rectangle(img, rekt, COLOR_WHITE, 2);
    sprintf(colorStr, "(%03d,%03d,%03d)", saneColor[0], saneColor[1], saneColor[2]);
    putText(img, colorStr, location, CV_FONT_HERSHEY_PLAIN, 1, COLOR_WHITE);
    circle(img, location, 2, COLOR_BLACK, 1);
}

Mat multiple_threshold(Mat img, Scalar hsv_min, Scalar hsv_max,
                       Scalar rgb_min, Scalar rgb_max)
{
    Mat hsv, rgb, resultTTT, result_hsv, result_rgb;
    cvtColor(img, hsv, CV_BGR2HSV);
    cvtColor(img, rgb, CV_BGR2RGB);
    inRange(hsv, hsv_min, hsv_max, result_hsv);
    inRange(rgb, rgb_min, rgb_max, result_rgb);
    bitwise_or(result_hsv, result_rgb, resultTTT);
    return resultTTT;
}

bool check_point(Scalar color, Scalar hsv_min, Scalar hsv_max)
{
    auto rgb = Scalar(color[2], color[1], color[0]);
    auto hsv = rgb2hsv(rgb);
    return hsv[0] >= hsv_min[0] && hsv[0] <= hsv_max[0]
            && hsv[1] >= hsv_min[1] && hsv[1] <= hsv_max[1]
            && hsv[2] >= hsv_min[2] && hsv[2] <= hsv_max[2];
    auto rgb_min = hsv2rgb(hsv_min);
    auto rgb_max = hsv2rgb(hsv_max);
    // the rgb_min/max are arranged r,g,b
    // the color is arranged b,g,r
    return color[0] > rgb_min[2] && color[0] < rgb_max[2]
            && color[1] > rgb_min[1] && color[1] < rgb_max[1]
            && color[2] > rgb_min[0] && color[2] < rgb_max[0];
}

vector<Game_Piece> Match_logo_totes(Mat img, vector<vector<Point> > box, vector<vector<Point> > logo)
{
    vector<Game_Piece> totes;
    //loop through every box
    for(size_t i = 0; i < box.size(); i++)
    {
        Game_Piece t;
        t.set_piece_type(OBJECT_YELLOW_TOTE);
        Moments moment = moments(box[i], false);
        Point2f box_center = Point2f(moment.m10/moment.m00, moment.m01/moment.m00);
        circle(img, box_center, 3, COLOR_RED, 3);
        t.set_center(box_center);
        //all we see is the long side of 1 or more yellow totes.
        if(logo.size() == 0)
        {
            //tote[i].set_center(box_center);
            Point2f box_center_rb = Point2f(box_center.x - (img.cols / 2.), -(box_center.y - (img.rows / 2.)));
            t.set_xrot((box_center_rb.x / (img.cols / 2.)) * (fov.x / 2.));
            t.set_rotation(90);
        }
        //loop through every logo
        for(size_t j = 0; j < logo.size(); j++)
        {
            Moments moment = moments(logo[j], false);
            Point2f logo_center = Point2f(moment.m10/moment.m00, moment.m01/moment.m00);
            circle(img, logo_center, 3, COLOR_BLUE, 3);

            //if the two centers are close, they are a match.
            if(distance(box_center, logo_center) < 35)
            {
                t.set_center(box_center);

                //Remap the center from the top left to the center of the screen
                //for tote_center and logo_center
                Point2f box_center_rb = Point2f(box_center.x - (img.cols / 2.), -(box_center.y - (img.rows / 2.)));
                logo_center = Point2f(logo_center.x - (img.cols / 2.), -(logo_center.y - (img.rows / 2.)));

                //Calculate x rotation to tote_center and logo_center
                t.set_xrot((box_center_rb.x / (img.cols / 2.)) * (fov.x / 2.));
                t.set_rotation((box_center_rb.x - logo_center.x) / (img.cols) * (fov.x));
            }
            else //this box doesn't have a matching logo, we're looking at it's long side
            {
//                tote[i].set_center(box_center);
                Point2f box_center_rb = Point2f(box_center.x - (img.cols / 2.), -(box_center.y - (img.rows / 2.)));
                t.set_xrot((box_center_rb.x / (img.cols / 2.)) * (fov.x / 2.));
                t.set_rotation(90);
            }
        }
        totes.push_back(t);
    }
    //Determine if yellow totes are stacked
    for(size_t i = 0; i < totes.size(); i++)
    {
        int stack_height = 1;
        for(size_t j = i+1; j < totes.size(); j++)
        {
            if(abs(totes[i].get_center().x - totes[j].get_center().x) < 10)
            {
                stack_height++;
            }
        }
        totes[i].set_totes_high(stack_height);
    }
    return totes;
}

void Laplacian( Mat& src, Mat& dst)
{
    int kernel_size = 3;
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;

    Laplacian( src, dst, ddepth, kernel_size, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( dst, dst );

    threshold(dst, dst, 5, 75, CV_THRESH_BINARY);
}

float calculate_distance(Mat& img, Point2f center)
{
    float distance = -1;
    float y_rot = center.y * fov.y / img.rows;
    distance = adjacent*tan(cvt2rad(y_rot));
    return distance;
}

void send_udp(std::vector<Game_Piece> pieces)
{
    static int iter = 0;
    static UdpSender udp("roboRIO-1706.local", "http");
    static SolutionLog lg("vision_output.csv", {"iter", "clock", "xrot", "distance", "rotation", "green", "type", "height"});
    auto closest = pieces.end();
    for (auto it = pieces.begin(); it < pieces.end(); ++it) {
        if (closest == pieces.end() || it->get_distance() < closest->get_distance()) {
            closest = it;
        }
    }
    if (closest != pieces.end()) {
        udp.send(*closest);
        lg.log("iter", iter++);
        lg.log("clock", static_cast<double>(clock()) / CLOCKS_PER_SEC);
        lg.log("xrot", closest->get_xrot());
        lg.log("distance", closest->get_distance());
        lg.log("rotation", closest->get_rotation());
        lg.log("green", closest->get_green_bin());
        lg.log("type", closest->get_piece_type());
        lg.log("height", closest->get_totes_high());
        lg.flush();
    } else {
        Game_Piece fake;
        udp.send(fake);
    }
}

static std::map<std::string, clock_t> profiles;

void profile_start(string id)
{
    profiles[id] = clock();
}

void profile_end(string id)
{
    clock_t start = profiles[id];
    profiles[id] = clock() - start;
}

void profile_print()
{
    for (auto it = profiles.begin(); it != profiles.end(); ++it) {
        double duration = it->second;
        duration = duration / CLOCKS_PER_SEC;
        string blanks(20 - it->first.size(), ' ');
        printf("Profile [%s]%s %.2fs\n", it->first.c_str(), blanks.c_str(), duration);
    }
    printf("--------------------------------------------------------------------------------\n");
}
