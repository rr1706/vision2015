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
    Mat dst, detected_edges, drawing;

    dst.create( img.size(), img.type() );

    blur(rgb, rgb, Size(20,20), Point(-1,-1), BORDER_CONSTANT);

    //Blur the image to smooth it
    //blur(img, img, Size(3,3), Point(-1,-1), BORDER_CONSTANT);

    Laplacian(img, detected_edges);

    imshow("detected edges", detected_edges);

    ///Make sure these Mat's are empty
    dst = Scalar::all(0);

    img.copyTo( dst, detected_edges);

    vector<Game_Piece> game_piece;

    cvtColor(img, drawing, CV_GRAY2RGB);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    Game_Piece unknown_game_piece;

    findContours(dst, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0) );

    Rect boundRect;
    vector<Contour> polygons(contours.size());

    for( unsigned int i = 0; i < contours.size(); i++ )
    {
        if(contourArea(contours[i])>2500 && contourArea(contours[i]) < 500000)
        {

            vector<Point> close, back;
            double stddev = contour_stddev(img, contours[i]);
            printf("STDDEV %f\n\n", stddev);
//            seperate_Contours(img, contours[i], close, back);

            drawContours(drawing, contours, i, COLOR_RED, 1, 8);

            approxPolyDP(contours[i], polygons[i], 15, true);
            if (!isContourConvex(polygons[i])) {
                continue;
            }
            drawContours(drawing, polygons, i, COLOR_GREEN, 3, 8);

            //todo: differentiate between two objects that are overlapping
            //Calculate average distance to a contour (by averaging the distance to every pixel in the contour
            //vector<float> ave_distance = Average_Distance(depth, contours, boundRect);

            //calculate the center of the contour using nth order (1st order) moments
            Point2f center = Calculate_Center(contours[i]);

            Rect boundrect = boundingRect(contours[i]);

            Point closest = get_closest_point(img, contours[i]);
            Point left = get_min_x(img, boundrect, contours[i]);
            Point right = get_max_x(img, boundrect, contours[i]);
            Point bottom = get_max_y(img, boundrect, contours[i]);
            Point top = get_min_y(img, boundrect, contours[i]);

            circle(drawing, closest, 2, COLOR_BLUE, 1, 8, 0);
            circle(drawing, top, 2, COLOR_RED, 1, 8, 0);
            circle(drawing, bottom, 2, COLOR_RED, 1, 8, 0);
            circle(drawing, right, 2, COLOR_RED, 1, 8, 0);
            circle(drawing, left, 2, COLOR_RED, 1, 8, 0);



            //Find distance based off pixel intensity
            double distance = Calculate_Real_Distance(img, closest);

            //Check color to tell what game piece, if any, we are looking at.
            Determine_Game_Piece(rgb, center, unknown_game_piece, top, bottom);

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
            unknown_game_piece.set_distance(stddev);

            game_piece.push_back(unknown_game_piece);

            Display_Game_Piece(unknown_game_piece, drawing, center);
        }
    }
    drawing.copyTo(output);

    imshow("Drawing", drawing);

    return game_piece;
}
