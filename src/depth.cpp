#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "functions.h"
#include "util.hpp"
#include "tracker.hpp"

using namespace cv;
using namespace std;

std::vector<Game_Piece> DepthTracker::find_pieces(Mat img, int key)
{
    Mat calibrate, calibrated, thresholded, thresholded2, dst, detected_edges, drawing;

    calibrate = imread("./Calibrate.png", CV_LOAD_IMAGE_GRAYSCALE);

    convertScaleAbs(img, img, 0.25, 0);
//    img.convertTo(img, CV_8UC1, 255.0/2048.0);

    //img = imread("raw.jpeg", CV_LOAD_IMAGE_GRAYSCALE);

    //imwrite("raw.jpeg", img);

    Get_Calibration_Image(img, key);

    ///Delete the floor and static targets
    calibrated = calibrate - img-1;
    threshold(calibrated, thresholded, 1, 255, CV_THRESH_BINARY_INV);
    thresholded2 = img - thresholded;

    //Blur the image to smooth it
    blur(thresholded2, thresholded2, Size(3,3), Point(-1,-1), BORDER_CONSTANT);

    //Eliminate Noise
    erode(thresholded2, thresholded2 ,1, Point(0,0), 1, BORDER_CONSTANT,morphologyDefaultBorderValue());
    dilate(thresholded2, dst ,1,Point(0,0),1, BORDER_CONSTANT,morphologyDefaultBorderValue());

    /// Create a matrix of the same type and size as depth_mat (for dst)
    dst.create( thresholded2.size(), thresholded2.type() );

    ///Blur the image to smooth it
    blur(thresholded2, thresholded2, Size(3,3), Point(-1,-1), BORDER_CONSTANT);

    ///Eliminate Noise
    erode(thresholded2, thresholded2 ,1, Point(0,0), 1, BORDER_CONSTANT,morphologyDefaultBorderValue());
    dilate(thresholded2, detected_edges ,1,Point(0,0),1, BORDER_CONSTANT,morphologyDefaultBorderValue());

    ///Make sure these Mat's are empty
    dst = Scalar::all(0);

    img.copyTo( dst, detected_edges);

    //Calibrate_Image(img, depth, thresholded2);

    //Get_Calibration_Image(depth, c);

    //Process_Image(thresholded2, dst);

    vector<Game_Piece> game_piece;

    cvtColor(img, drawing, CV_GRAY2RGB);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    Game_Piece unknown_game_piece;

    findContours(dst, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0) );

    Rect boundRect;

    for( unsigned int i = 0; i < contours.size(); i++ )
    {
        if(contourArea(contours[i])>2500 && contourArea(contours[i]) < 500000)
        {

            //todo: differentiate between two objects that are overlapping
            //Calculate average distance to a contour (by averaging the distance to every pixel in the contour
            //vector<float> ave_distance = Average_Distance(depth, contours, boundRect);

            //calculate the center of the contour using nth order (1st order) moments
            Point2f center = Calculate_Center(contours[i]);

            Rect boundrect = boundingRect(contours[i]);

            Point closest = get_closest_point(img, contours[i]);
            Point left = get_min_x(contours[i], closest);
            Point right = get_max_x(contours[i], closest);
            Point bottom = get_max_y(boundrect);
            Point top = get_min_y(boundrect);

            circle(drawing, closest, 2, COLOR_BLUE, 1, 8, 0);
            circle(drawing, top, 2, COLOR_RED, 1, 8, 0);
            circle(drawing, bottom, 2, COLOR_RED, 1, 8, 0);
            circle(drawing, right, 2, COLOR_RED, 1, 8, 0);
            circle(drawing, left, 2, COLOR_RED, 1, 8, 0);



            //Find distance based off pixel intensity
            double distance = Calculate_Real_Distance(img, center);

            //Check color to tell what game piece, if any, we are looking at.
            Determine_Game_Piece(center, unknown_game_piece, top, bottom);

            //draw what we know
            drawContours(drawing, contours,i, COLOR_RED, 1, 8, hierarchy, 0, Point() );
            rectangle(drawing, boundRect.tl(), boundRect.br(), COLOR_RED, 2, 8, 0 );
            circle(drawing, center, 2, COLOR_RED, 1, 8, 0);

            //check to see if the game piece is a tote
            if(unknown_game_piece.get_piece_type() == 1 || unknown_game_piece.get_piece_type() == 2)
            {
                //Determine stack height
                int totes = find_number_of_totes(img, unknown_game_piece, center, top);

                unknown_game_piece.set_totes_high(totes);

                //Determine offset
                unknown_game_piece.set_rotation(find_orientation(img, left, closest, right));
            }

            //Populate our class with values that we calculated
            unknown_game_piece.set_xrot(Calculate_Xrot(center));
            unknown_game_piece.set_distance(distance);

            game_piece.push_back(unknown_game_piece);

            Display_Game_Piece(unknown_game_piece, drawing, center);
        }
    }

    imshow("Drawing", drawing);
    imshow("Calibrated", thresholded2);


    return game_piece;
}
