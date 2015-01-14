#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "util.hpp"
#include <free.hpp>
#include "yellow.hpp"
#include "functions.h"
#include "tracker.hpp"

using namespace cv;
using namespace std;

//Returns the distance between two floating points.
double distance(Point2f one, Point2f two)
{
    return sqrt(pow(one.x - two.x, 2) + pow(one.y - two.y, 2));
}

//Returns the distance between the camera and Point center
double Calculate_Real_Distance(Mat img, Point2f center) {
    Scalar intensity = img.at<uchar>(center);
    return 0.1236 * tan(intensity[0]*4 / 2842.5 + 1.1863)*100;
}

//Determines the center of the contour passed to it
Point2f Calculate_Center(vector<Point> contour)
{
    Moments mu;
    mu = moments(contour, false);
    return Point2f(mu.m10/mu.m00, mu.m01/mu.m00);
}

//Preforms blur, erode and dilate on src
//And populates the result in dst
void Process_Image(Mat src, Mat dst)
{
    //Blur the image to smooth it
    blur(src, src, Size(3,3), Point(-1,-1), BORDER_CONSTANT);

    //Eliminate Noise
    erode(src, src ,1, Point(0,0), 1, BORDER_CONSTANT,morphologyDefaultBorderValue());
    dilate(src, dst ,1,Point(0,0),1, BORDER_CONSTANT,morphologyDefaultBorderValue());
}

//Calibrates the image such that the constant
//environment is removed, leaving only the new
//objects.
void Calibrate_Image(Mat src, Mat depth, Mat dst)
{
    convertScaleAbs(src, depth, 0.25, 0);

    Mat calibrate, calibrated, thresholded, thresholded2;
    calibrate = imread("Calibrate.png", CV_LOAD_IMAGE_GRAYSCALE);

    ///Delete the floor and static targets
    calibrated = calibrate - depth-1;
    threshold(calibrated, thresholded, 1, 255, CV_THRESH_BINARY_INV);
    dst = depth - thresholded;
}

//Converts degrees to radians
double cvt2rad(double degree)
{
    return degree * CV_PI / 180;
}

//Determines if the object is a yellow tote, gray tote, or green bin
void Determine_Game_Piece(Point2f center, Game_Piece& unknown_game_piece)
{
    char str[50];
    //Mat img = kinectRGB(0);
    Mat img = imread("color.jpeg", CV_LOAD_IMAGE_COLOR);
    cvtColor(img, img, CV_BGR2RGB);
    Vec3b color = img.at<Vec3b>(center);
    //Scalar color = Vec3b2Scalar(color_at_center);

    //imwrite("color.jpeg", img);
    sprintf(str, "r  = %d", color[0]);
    putText(img, str,Point(15, 35), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_WHITE,1,8,false);
    sprintf(str, "g  = %d", color[1]);
    putText(img, str,Point(15, 55), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_WHITE,1,8,false);
    sprintf(str, "b  = %d", color[2]);
    putText(img, str,Point(15, 75), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_WHITE,1,8,false);

    //Yellow Tote
    if(color[0] <= yellow_tote[0] + color_tolerance &&
            color[1] <= yellow_tote[1] + color_tolerance &&
            color[2] <= yellow_tote[2] + color_tolerance &&
            color[0] >= yellow_tote[0] - color_tolerance &&
            color[1] >= yellow_tote[1] - color_tolerance &&
            color[2] >= yellow_tote[2] - color_tolerance)
    {

        unknown_game_piece.set_piece_type(2);
    }
    //Gray Tote
    else if(color[0] <= gray_tote[0] + color_tolerance &&
            color[1] <= gray_tote[1] + color_tolerance &&
            color[2] <= gray_tote[2] + color_tolerance &&
            color[0] >= gray_tote[0] - color_tolerance &&
            color[1] >= gray_tote[1] - color_tolerance &&
            color[2] >= gray_tote[2] - color_tolerance)
    {
        unknown_game_piece.set_piece_type(1);

    }
    //Green Bin
    else if(color[0] <= green_bin[0] + color_tolerance &&
            color[1] <= green_bin[1] + color_tolerance &&
            color[2] <= green_bin[2] + color_tolerance &&
            color[0] >= green_bin[0] - color_tolerance &&
            color[1] >= green_bin[1] - color_tolerance &&
            color[2] >= green_bin[2] - color_tolerance)
    {
        unknown_game_piece.set_piece_type(3);
    }

    imshow("RGB", img);
    return;
}

//Determines amount of totes stacked
void find_number_of_totes(Mat img, Game_Piece& tote, Point2f center, Point2f height)
{
    double diff = abs(center.y - height.y);
    double yrot = diff*fov.y/(Image_Height);
    yrot = cvt2rad(yrot);
    Scalar intensity = img.at<uchar>(height);
    float distance_to_top = 0.1236 * tan(intensity[0]*4 / 2842.5 + 1.1863)*100;
    float real_height =  2*sin(yrot)*distance_to_top;
    printf("real height = %.2f\n", real_height);
    if(real_height <= single_stack_height + height_tolerance && real_height >= single_stack_height - height_tolerance)
    {
        tote.set_totes_high(99);
        printf("%d\n", tote.get_totes_high());
    }
    else if(real_height <= double_stack_height + height_tolerance && real_height >= double_stack_height - height_tolerance)
    {
        tote.set_totes_high(20);
        printf("%d\n", tote.get_totes_high());
        printf("test passed\n");
    }
    else if(real_height <= triple_stack_height + height_tolerance && real_height >= triple_stack_height - height_tolerance)
    {
        tote.set_totes_high(3);
        printf("%d\n", tote.get_totes_high());
    }
    //todo:4, 5 and 6 totes high

    return;
}

//Captures current frame for the new calibration image
void Get_Calibration_Image(Mat img, int key)
{
    if (key == 's') //s key pressed
    {
        printf("Calibrating...");
        imwrite("./Calibrate.png", img);
        printf("Done\n");
    }
    return;
}

//Calculates degrees between the point passed
//and the center of the image
double Calculate_Xrot(Point2f center)
{
    //remap the center from top left to center of top
    center.x = (center.x - Image_Width/2);

    // Calculate angle to center of box
    double Xrot = center.x/(Image_Width/2)*fov.x/2;

    return Xrot;
}

//Determines if the L is a left or right L
void Calculate_side(SingleL L, Point2f center, Mat img)
{
    if (center.x - L.center.x > 0)
    {
        // closer to left side
        putText(img, "R", center, CV_FONT_HERSHEY_SIMPLEX, 2, COLOR_BLUE, 2);
        L.side = RIGHT_SIDE;
    } else
    {
        putText(img, "L", center, CV_FONT_HERSHEY_SIMPLEX, 2, COLOR_BLUE, 2);
        L.side = LEFT_SIDE;
    }
    circle(img, center, 3, COLOR_BLUE, 2);
    return;
}

//needs to be changed so stacked_totes is a vector vector to account for multiple stacks
//and it needs to be able to draw a line from the bottom tote to the top tote in the
//stacked totes second list
//Populates stacked_totes and unstacked_totes with the approapriate detected_totes
void determine_stacked(vector<YellowTote>& detected_totes, vector<YellowTote>&  stacked_totes, vector<YellowTote>& unstacked_totes, Mat img)
{
    for (size_t tote_i = 0; tote_i < detected_totes.size(); tote_i++)
    {
        for (size_t other_i =  tote_i + 1; other_i < detected_totes.size(); other_i++)
        {
            if (abs(detected_totes[tote_i].get_center_x() - detected_totes[other_i].get_center_x()) < 20)
            {
                if (!detected_totes[tote_i].get_stacked())
                {
                    detected_totes[tote_i].set_stacked(true);
                    stacked_totes.push_back(detected_totes[tote_i]);
                }
                if (!detected_totes[other_i].get_stacked())
                {
                    detected_totes[other_i].set_stacked(true);
                    stacked_totes.push_back(detected_totes[other_i]);
                }
            }
        }
    }
    for (unsigned int i = 0; i < detected_totes.size(); i ++)
    {
        if(!detected_totes[i].get_stacked())
        {
            unstacked_totes.push_back(detected_totes[i]);
        }
    }

    //I think I got it to draw from the bottom of the stack to the top, it is untested though -_-
    for(unsigned int i = 0; i < stacked_totes.size()-1; i ++)
    {
        circle(img, Point(stacked_totes[0].get_center_x(), stacked_totes[0].get_center_y()), 3, COLOR_BLUE);
        line(img, Point((int)stacked_totes[0].get_center_x(), (int)stacked_totes[0].get_center_y()), Point((int)stacked_totes[1].get_center_x(), (int)stacked_totes[1].get_center_y()), COLOR_BLUE, 2);
    }

    return;
}

//broken code
//vector<float> Average_Distance(Mat image, vector<vector<Point>> contours, Rect roi)
//{
//    Mat labels = Mat::zeros(image.size(), CV_8UC1);
//    vector<float> cont_avgs(contours.size(), 0.f);
//    for(size_t i = 0; i < contours.size(); i++)
//    {
//        drawContours(labels, contours, i, Scalar(i), CV_FILLED);
//        Scalar mean = cv::mean(image(roi), labels(roi) == i);
//        cont_avgs[i] = mean[0];
//    }
//}

//Matches SingleLs, returns vector of YellowTotes
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

std::vector<YellowTote> find_yellow_ir(Mat img)
{
    std::vector<SingleL> singles;
    Mat draw;
    cvtColor(img, draw, CV_GRAY2BGR);

    threshold(img, img, 250, 255, CV_THRESH_BINARY);
    dilate(img, img, kern, Point(-1,-1), 3);
    erode(img, img, kern, Point(-1,-1), 1);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(img, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    for (size_t i = 0; i < contours.size(); i++)
    {
        vector<Point> contour = contours[i];

        if (contourArea(contour) > max_area || contourArea(contour) < min_area)
        {
            continue;
        }

        Rect bound = boundingRect(contour);
        rectangle(draw, bound, Scalar(78, 45, 68));

        Point2f massCenter = Calculate_Center(contour);
        Point2f boxCenter = Point2f(bound.x + (bound.width / 2.0), bound.y + (bound.height / 2.0));

        //line(draw, Point2f(boxCenter, 0), Point2f(boxCenter, 480), Scalar(0, 255, 0));
        SingleL single_l;
        single_l.center = massCenter;
        single_l.bound = bound;
        single_l.area = contourArea(contours[i]);

        Calculate_side(single_l, boxCenter, draw);

        singles.push_back(single_l);
    }
    std::vector<YellowTote> detected_totes = pairTotes(singles);

    //split the detected totes into ones that look stacked and ones that aren't
    vector<YellowTote> stacked_totes;
    vector<YellowTote> unstacked_totes;

    //Determine if a tote is stacked or not
    determine_stacked(detected_totes, stacked_totes, unstacked_totes, draw);

    //Figure out xrot and populate it
    if(stacked_totes.size() != 0)
    {
        for(unsigned int i = 0; i < stacked_totes.size(); i++)
        {
            stacked_totes[i].set_xrot(Calculate_Xrot(stacked_totes[i].get_center()));
        }
    }
    for(unsigned int i = 0; i < unstacked_totes.size(); i ++)
    {
        unstacked_totes[i].set_xrot(Calculate_Xrot(unstacked_totes[i].get_center()));
    }

    imshow("Drawing", draw);
    imwrite("Final.jpeg", draw);

    //send game_pieces that do not have the default values to savannah

    return detected_totes;
    //maybe this function should be a void and do all the udp stuff inside the function.
}

//vector<YellowTote> find_yellow_color(Mat img)
//{

//}
