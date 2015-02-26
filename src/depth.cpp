#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <thread>
#include "functions.hpp"
#include "util.hpp"
#include "tracker.hpp"

#define SHOW_SEPARATE_CONTOURS false
#define SHOW_MIN_POINTS false

using namespace cv;
using namespace std;

struct ContourData {
    bool ignore;
    int i;
    Contour contour;
    Game_Piece unknown_game_piece;
    Mat drawing;
    Mat rgb, depth;
};

static void process_contour(ContourData *dat)
{
    Contour contour = dat->contour;
    Mat img = dat->depth;
    Mat rgb = dat->rgb;
    Mat drawing = dat->drawing;
    Game_Piece unknown_game_piece = dat->unknown_game_piece;
    float distance, orientation;
    int totes;
    Point2i center, closest, left, right, bottom, top;
    Rect boundrect;

    profile_start("contour_" + std::to_string(dat->i));
    profile_start("get points");
    //calculate the center of the contour using nth order (1st order) moments
    center = Calculate_Center(contour);
    // Fit a rectangle around the contour
    boundrect = boundingRect(contour);
    // Find the point that has the least distance from the robot
    closest = get_closest_point(img, contour);
    // The following four grab the side from the bounding rectangle and return the first one with a distance value
    left = get_min_x(img, boundrect, contour) + Point(10, 0);
    right = get_max_x(img, boundrect, contour) - Point(10, 0);
    bottom = get_max_y(img, boundrect, contour) - Point(0, 10);
    top = get_min_y(img, boundrect, contour) + Point(0, 10);
    // calculate distance based on pixel intensity of the depth map
    distance = Calculate_Real_Distance(img, center);
    profile_end("get points");

    if (distance < 60.) {
        dat->ignore = true;
        return;
    }

    if (SHOW_MIN_POINTS) {
        circle(drawing, closest, 5, COLOR_BLUE, -1, 8, 0);
        circle(drawing, top, 2, COLOR_RED, -1, 8, 0);
        circle(drawing, bottom, 2, COLOR_GREEN, -1, 8, 0);
        circle(drawing, right, 2, COLOR_RED, -1, 8, 0);
        circle(drawing, left, 2, COLOR_RED, -1, 8, 0);
    }

    profile_start("game piece");
    //Check color to tell what game piece, if any, we are looking at.
    unknown_game_piece.set_piece_type(OBJECT_UNKNOWN);
//    Determine_Game_Piece(rgb, center, unknown_game_piece, top, bottom);
    float ratio = static_cast<float>(boundrect.width) / boundrect.height;
    if (ratio > 8)
        unknown_game_piece.set_piece_type(OBJECT_BUMP);
    profile_end("game piece");

    //check to see if the game piece is a tote
//        if(unknown_game_piece.get_piece_type() == 1 || unknown_game_piece.get_piece_type() == 2)
    {
        //Determine stack height
        profile_start("height");
        totes = find_number_of_totes(img, unknown_game_piece, center, top);

        unknown_game_piece.set_totes_high(totes);
        profile_end("height");

        profile_start("orientation");
        orientation = find_orientation(img, left, closest, right);
        if (abs(orientation) > 60) {
            orientation = 0;
        }
        //Determine offset
        unknown_game_piece.set_rotation(orientation);
        profile_end("orientation");
    }

    //Populate our class with values that we calculated
    unknown_game_piece.set_xrot(Calculate_Xrot(img, center));
    unknown_game_piece.set_distance(distance);
    unknown_game_piece.set_center(center);

    dat->unknown_game_piece = unknown_game_piece;
    profile_end("contour_" + std::to_string(dat->i));
}

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

    vector<Contour> contours;
    vector<Vec4i> hierarchy;

    findContours(detected_edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0) );
    profile_end("filter");

    if (SHOW_SEPARATE_CONTOURS) {
        Mat originalContours(480, 640, CV_8UC3);
        for (size_t i = 0, x = 0; i < contours.size(); i++, x += 20) {
            if (hierarchy[i][3]!=-1) {
                drawContours(originalContours, contours, i, Scalar(x, 255, 255));
            }
        }
        cvtColor(originalContours, originalContours, CV_HSV2BGR);
        imshow("Original Contours", originalContours);
    }
    profile_start("sep. contours");
    { // interior contours finding block
        vector<Contour> modCtrs;
        for (size_t i = 0, x = 0; i < contours.size(); i++, x += 20) {
            if (hierarchy[i][3]!=-1 && contourArea(contours[i]) > 5000) {
                modCtrs.push_back(contours[i]);
            }
        }
        // put outputs back in real contour storage
        modCtrs.swap(contours);
    }
    // updates the contours storage with the results from the stddev-based separator
    separate_contours(img, contours);
    profile_end("sep. contours");
    if (SHOW_SEPARATE_CONTOURS) {
        Mat separatedContours(480, 640, CV_8UC3);
        for (size_t i = 0, x = 0; i < contours.size(); i++, x += 20) {
            drawContours(separatedContours, contours, i, Scalar(x, 255, 255),1,8);
        }
        cvtColor(separatedContours, separatedContours, CV_HSV2BGR);
        imshow("Separated Contours", separatedContours);
    }
    vector<Contour> polygons(contours.size());
    profile_start("contours");
    map<thread*, ContourData*> threads;
    for (size_t i = 0; i < contours.size(); i++) {
        if (contourArea(contours[i]) < 2500 || contourArea(contours[i]) > 500000) {
            continue;
        }
        approxPolyDP(contours[i], polygons[i], 15, true);
        ContourData *dat = new ContourData;
        dat->ignore =  false;
        dat->contour = contours[i];
        dat->depth = img;
        dat->drawing = drawing.clone();
        dat->i = i;
        dat->rgb = rgb;
        thread *t = new thread(process_contour, dat);
        threads[t] = dat;
    }
    if (SHOW_IMAGES) {
        drawContours(drawing, contours, -1, COLOR_RED, 1, 8);
        drawContours(drawing, polygons, -1, COLOR_WHITE, 1, 8);
    }
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        it->first->join();
        if (it->second->ignore)
            continue;
        game_piece.push_back(it->second->unknown_game_piece);
        if (SHOW_IMAGES) {
            Display_Game_Piece(it->second->unknown_game_piece, drawing, it->second->unknown_game_piece.get_center());
        }
        delete it->first;
        delete it->second;
    }
    profile_end("contours");
    drawing.copyTo(output);

    if (SHOW_IMAGES) {
        imshow("Drawing", drawing);
        imshow("RGB", rgb);
    }

    return game_piece;
}
