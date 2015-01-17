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

double distance(Point2f one, Point2f two)
{
    return sqrt(pow(one.x - two.x, 2) + pow(one.y - two.y, 2));
}

double Calculate_Real_Distance(Mat img, Point2f center) {
    Scalar intensity = img.at<uchar>(center);
    return 0.1236 * tan(intensity[0]*4 / 2842.5 + 1.1863)*100;
}

Point2f Calculate_Center(vector<Point> contour)
{
    Moments mu;
    mu = moments(contour, false);
    return Point2f(mu.m10/mu.m00, mu.m01/mu.m00);
}

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

double cvt2rad(double degree)
{
    return degree * CV_PI / 180;
}

double cvt2degree(double radian)
{
    return radian*180/CV_PI;
}

void Determine_Game_Piece(Point2f center, Game_Piece& unknown_game_piece, Point top, Point bottom)
{
    char str[50];
    Mat img = kinectRGB(0);
    cvtColor(img, img, CV_BGR2RGB);
    Vec3b color = img.at<Vec3b>(center);
    circle(img, center, 2, COLOR_RED, 1, 8, 0);

    circle(img, top, 2, COLOR_WHITE, 1, 8, 0);

    Vec3b top_color = img.at<Vec3b>(top);

    sprintf(str, "r  = %d", color[0]);
    putText(img, str,Point(15, 35), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_RED,1,8,false);
    sprintf(str, "g  = %d", color[1]);
    putText(img, str,Point(15, 55), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_RED,1,8,false);
    sprintf(str, "b  = %d", color[2]);
    putText(img, str,Point(15, 75), CV_FONT_HERSHEY_COMPLEX_SMALL, 0.75, COLOR_RED,1,8,false);

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

    if(unknown_game_piece.get_piece_type() == 1 && green_bin_top(img, top + Point(0, 40)))
    {
        unknown_game_piece.set_green_bin(true);
    }
    if(unknown_game_piece.get_piece_type() == 3 && tote_on_bottom(img, bottom - Point(0, 40)))
    {
        unknown_game_piece.set_piece_type(1);
    }

    imshow("RGB", img);
    return;
}

int find_number_of_totes(Mat img, Game_Piece& tote, Point2f center, Point2f height)
{
    double diff = abs(center.y - height.y);
    double yrot = diff*fov.y/(Image_Height);
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

double Calculate_Xrot(Point2f center)
{
    //remap the center from top left to center of top
    center.x = (center.x - Image_Width/2);

    // Calculate angle to center of box
    double Xrot = center.x/(Image_Width/2)*fov.x/2;
    center.x = (center.x + Image_Width/2);

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
    for (size_t tote_i = 0; tote_i < detected_totes.size(); tote_i++)
    {
        int level = 1;
        for (size_t other_i =  tote_i + 1; other_i < detected_totes.size(); other_i++)
        {
            if (abs(detected_totes[tote_i].get_center_x() - detected_totes[other_i].get_center_x()) < 20)
            {
                if (detected_totes[tote_i].get_stacked() == -1)
                {
                    detected_totes[tote_i].set_stacked(level);
                    stacked_totes[tote_i][level] = detected_totes[tote_i];
                    level++;
                }
                if (detected_totes[other_i].get_stacked() == -1)
                {
                    detected_totes[other_i].set_stacked(level);
                    stacked_totes[tote_i][level] = detected_totes[other_i];
                    level++;
                }
            }
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



    for(unsigned int i = 0; i < stacked_totes.size(); i++)
    {
        for(unsigned int j = 0; j < stacked_totes[i].size(); j++)
        {
            //figure out if the variable is junk
            if(stacked_totes[i][j].get_center_x() == 0)
            {
                //it is junk, get rid of it in the vector
                stacked_totes[i].erase(stacked_totes[i].begin()+j);
            }
        }
    }

    //draw a line down the middle of the stack if one exists
    for(unsigned int i = 0; i < stacked_totes.size(); i++)
    {
        for(unsigned int j = 0; j < stacked_totes[i].size()-1; j++)
        {
            //do not draw to the junk points
            if (stacked_totes[i][j].get_stacked() == 0) {
                continue;
            }
            if (stacked_totes[i][j + 1].get_stacked() == 0) {
                continue;
            }

            circle(img, stacked_totes[i][j].get_center(), 3, COLOR_BLUE);
            line(img, stacked_totes[i][j].get_center(), stacked_totes[i][j+1].get_center(), COLOR_BLUE, 2);
        }
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

//someone make a document with a step by step process of this math
//and put it somewhere online so we can link to it in a comment.
double find_orientation(Mat img, Point2f left, Point2f center, Point2f right)
{
    double theta_3 = -99;
    //is left or right side the long side?
    //Zac's function will do this for me.
    //get the distances to the 3 points
    //d_3 is short in this case
    if(left.x == right.x)
    {
        double d_2 = Calculate_Real_Distance(img, center);
        double d_3 = Calculate_Real_Distance(img, left);

        double theta_2 = (left.x - center.x)/(Image_Width/2)*fov.x*2;

        double d_6 = cos(cvt2rad(theta_2))*d_2;
        double d_7 = d_3 - d_6;

        double theta_4 = cvt2degree(acos(d_7/short_side_distance));
        double theta_5 = 180-(theta_4 + theta_2);
        theta_3 = 180-theta_5;
    }

    //d_1 is short side
    else
    {
        double d_2 = Calculate_Real_Distance(img, center);
        double d_3 = Calculate_Real_Distance(img, right);

        double theta_2 = (left.x - center.x)/(Image_Width/2)*fov.x*2;

        double d_6 = cos(cvt2rad(theta_2))*d_2;
        double d_7 = d_3 - d_6;

        double theta_4 = cvt2degree(acos(d_7/short_side_distance));
        double theta_5 = 180-(theta_4 + theta_2);
        theta_3 = 180-theta_5;
    }

    return theta_3;


}

Point get_min_x(Rect boundrect)
{
    return Point(boundrect.x, boundrect.y + boundrect.height/2);
}

Point get_max_x(Rect boundrect)
{
    return Point(boundrect.x + boundrect.width, boundrect.y + boundrect.height/2);
}

Point get_min_y(Rect boundrect)
{
    return Point(boundrect.x + (boundrect.width / 2.0), boundrect.y + 15);
}

Point get_max_y(Rect boundrect)
{
    return Point(boundrect.x + (boundrect.width / 2.0), boundrect.y + boundrect.height - 15);
}

Point get_closest_point(Mat img, vector<Point> contour)
{
    Point closest_point = Point(-1,-1);
    Scalar closest = Scalar(255);
    for(unsigned int i = 0; i < contour.size(); i++)
    {
        Scalar intensity = img.at<uchar>(contour[i]);
        if(intensity[0] < closest[0])
        {
            closest[0] = intensity[0];
            closest_point = contour[i];
        }
    }
    return closest_point;
}

bool green_bin_top(Mat img, Point2f top)
{
    bool green_bin_on_top = false;
    Vec3b color = img.at<Vec3b>(top);
    //Check to see if the top pixel is of a green bin
    if(color[0] <= green_bin[0] + color_tolerance &&
            color[1] <= green_bin[1] + color_tolerance &&
            color[2] <= green_bin[2] + color_tolerance &&
            color[0] >= green_bin[0] - color_tolerance &&
            color[1] >= green_bin[1] - color_tolerance &&
            color[2] >= green_bin[2] - color_tolerance)
    {
        green_bin_on_top = true;
    }

    return green_bin_on_top;
}

bool tote_on_bottom(Mat img, Point2f bottom)
{
    {
        bool tote_on_bottom = false;
        Vec3b color = img.at<Vec3b>(bottom);
        printf("r%d g%d b%d\r\n", color[0], color[1], color[2]);
        //Check to see if the bottom pixel is of a gray tote
        //Gray Tote
        if(color[0] <= gray_tote[0] + color_tolerance &&
                color[1] <= gray_tote[1] + color_tolerance &&
                color[2] <= gray_tote[2] + color_tolerance &&
                color[0] >= gray_tote[0] - color_tolerance &&
                color[1] >= gray_tote[1] - color_tolerance &&
                color[2] >= gray_tote[2] - color_tolerance)
        {
            tote_on_bottom = true;

        }


        return tote_on_bottom;
    }
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
