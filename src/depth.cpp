#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "functions.h"
#include "util.hpp"
#include "tracker.hpp"

using namespace cv;
using namespace std;

std::vector<Game_Piece> DepthTracker::find_pieces(Mat img, Mat rgb, Mat &output)
{
    profile_start("filter");
    Mat dst, detected_edges, drawing;

    dst.create( img.size(), img.type() );

    GaussianBlur(img, img, Size(3, 3), 0, 0, BORDER_DEFAULT);
    Laplacian(img, detected_edges);

    dst = Scalar::all(0);

    img.copyTo( dst, detected_edges);

    vector<Game_Piece> game_piece;

    cvtColor(img, drawing, CV_GRAY2RGB);

    vector<Contour> contours, polygons;
    vector<Vec4i> hierarchy;

    Game_Piece unknown_game_piece;

    findContours(detected_edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0) );
    profile_end("filter");

//    Mat originalContours(480, 640, CV_8UC3);
//    Mat separatedContours(480, 640, CV_8UC3);
//    for (size_t i = 0, x = 0; i < contours.size(); i++, x += 20) {
//        if(hierarchy[i][3]!=-1)
//        {
//        drawContours(originalContours, contours, i, Scalar(x, 255, 255));
//        }
//    }
    vector<Contour> modCtrs;
    for (size_t i = 0, x = 0; i < contours.size(); i++, x += 20) {
        if(hierarchy[i][3]!=-1 && contourArea(contours[i]) > 5000)
        {
            modCtrs.push_back(contours[i]);
        }
    }
    modCtrs.swap(contours);
    profile_start("sep. contours");
    separate_contours(img, contours);
    profile_end("sep. contours");
//    for (size_t i = 0, x = 0; i < contours.size(); i++, x += 20) {
//        drawContours(separatedContours, contours, i, Scalar(x, 255, 255),1,8);
//    }
//    cvtColor(originalContours, originalContours, CV_HSV2BGR);
//    cvtColor(separatedContours, separatedContours, CV_HSV2BGR);
//    DEBUG_SHOW("Original Contours", originalContours);
//    DEBUG_SHOW("Separated Contours", separatedContours);
    Point2i center, closest, left, right, bottom, top;
    Rect boundrect;
    drawContours(drawing, contours, -1, COLOR_RED, 1, 8);
    for( unsigned int i = 0; i < contours.size(); i++ )
    {
        if(contourArea(contours[i])>2500 && contourArea(contours[i]) < 500000)
        {
            //calculate the center of the contour using nth order (1st order) moments
            profile_start("get points");
            center = Calculate_Center(contours[i]);
            Rect boundrect = boundingRect(contours[i]);
            closest = get_closest_point(img, contours[i]);
            left = get_min_x(img, boundrect, contours[i]) + Point(10, 0);
            right = get_max_x(img, boundrect, contours[i]) - Point(10, 0);
            bottom = get_max_y(img, boundrect, contours[i]) - Point(0, 10);
            top = get_min_y(img, boundrect, contours[i]) + Point(0, 10);
            profile_end("get points");

            circle(drawing, closest, 5, COLOR_BLUE, -1, 8, 0);
            circle(drawing, top, 2, COLOR_RED, -1, 8, 0);
            circle(drawing, bottom, 2, COLOR_GREEN, -1, 8, 0);
            circle(drawing, right, 2, COLOR_RED, -1, 8, 0);
            circle(drawing, left, 2, COLOR_RED, -1, 8, 0);

            //Find distance based off pixel intensity
            double distance = Calculate_Real_Distance(img, center);

            profile_start("game piece");
            //Check color to tell what game piece, if any, we are looking at.
            Determine_Game_Piece(rgb.clone(), center, unknown_game_piece, top, bottom);
            profile_end("game piece");

            //check to see if the game piece is a tote
//            if(unknown_game_piece.get_piece_type() == 1 || unknown_game_piece.get_piece_type() == 2)
            {
                //Determine stack height
                profile_start("stack height");
                int totes = find_number_of_totes(img, unknown_game_piece, center, top);
                profile_end("stack height");

                unknown_game_piece.set_totes_high(totes);

                profile_start("orientation");
                float orientation = find_orientation(img, left, closest, right);
                profile_end("orientation");
                if (abs(orientation) > 60)
                    orientation = NAN;
                //Determine offset
                unknown_game_piece.set_rotation(orientation);
            }

            //Populate our class with values that we calculated
            unknown_game_piece.set_xrot(Calculate_Xrot(img, center));
            unknown_game_piece.set_distance(distance);

            game_piece.push_back(unknown_game_piece);

            Display_Game_Piece(unknown_game_piece, drawing, center);
        }
    }
    drawing.copyTo(output);

    DEBUG_SHOW("Drawing", drawing);

    return game_piece;
}
