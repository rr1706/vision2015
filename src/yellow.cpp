#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <algorithm>
#include "yellow.hpp"
#include "util.hpp"

using namespace std;
using namespace cv;

enum LSide {
	LEFT_SIDE = 0,
	RIGHT_SIDE
};

struct SingleL {
	Point2f center;
	LSide side;
	Rect bound;
	bool paired = false;
};

YellowTote::YellowTote(Side side, int x, int y) : facingSide(side), x(x), y(y)
{
}

double YellowTote::xrot()
{
	return 0;
}

Point2f YellowTote::point()
{
	return Point2f(x, y);
}

bool operator==(const YellowTote& one, const YellowTote& two)
{
	return one.x == two.x && one.y == two.y;
}

double distance(Point2f one, Point2f two) {
	return sqrt(pow(one.x - two.x, 2) + pow(one.y - two.y, 2));
}

static Mat kern = getStructuringElement(MORPH_CROSS, Size(3, 3), Point(-1, -1));


// this function may be bugged
std::vector<YellowTote> pairTotes(std::vector<SingleL> singles)
{
	std::vector<YellowTote> detected_totes;
	for (size_t i = 0; i < singles.size(); i++) {
		SingleL single = singles[i];
		if (single.paired)
			continue;
		for (size_t j = i + 1; j < singles.size(); j++) {
			SingleL other = singles[j];
			if (other.paired)
				continue;
			int dist = distance(single.center, other.center);
			int xdist = abs(single.center.x - other.center.x);
			if (dist < single.bound.height * 2 && xdist > 15) {
				other.paired = true;
				single.paired = true;
				YellowTote tote(LONG_SIDE, (single.center.x + other.center.x) / 2,
								(single.center.y + other.center.y) / 2);
				detected_totes.push_back(tote);
				break;
			}
		}
	}
	return detected_totes;
}

std::vector<YellowTote> find_yellow_ir(Mat ir)
{
	std::vector<SingleL> singles;
	Mat binary, draw;
	cvtColor(ir, draw, CV_GRAY2BGR);
	threshold(ir, binary, 250, 255, CV_THRESH_BINARY);
	dilate(binary, binary, kern, Point(-1,-1), 3);
	erode(binary, binary, kern, Point(-1,-1), 1);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	imshow("morphed", binary);
	findContours(binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<vector<Point> > polygons(contours.size());
	for (size_t i = 0; i < contours.size(); i++) {
		vector<Point> contour = contours[i];
		int area = contourArea(contour);
		if (area > 100000 || area < 100) {
			continue;
		}
		vector<Point> polygon;
		approxPolyDP(contour, polygon, 5, true);
		polygons[i] = polygon;
		Rect bound = boundingRect(polygon);
		RotatedRect rotated = minAreaRect(polygon);
		rectangle(draw, bound, Scalar(78, 45, 68));

		Point2f vertices[4];
		rotated.points(vertices);
		for (int i = 0; i < 4; i++) {
			//~ line(draw, vertices[i], vertices[(i+1)%4], Scalar(0,255,0));
		}
		Moments moment = moments(contour, false);
		Point2f massCenter(moment.m10/moment.m00, moment.m01/moment.m00);
		double boxCenter = bound.x + (bound.width / 2.0);
		line(draw, Point2f(boxCenter, 0), Point2f(boxCenter, 480), Scalar(0, 255, 0));
		SingleL single;
		single.center = massCenter;
		single.bound = bound;
		if (boxCenter - massCenter.x > 0) {
			// closer to left side
			putText(draw, "R", massCenter, CV_FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 255, 255), 2);
			single.side = RIGHT_SIDE;
		} else {
			putText(draw, "L", massCenter, CV_FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 200, 255), 2);
			single.side = LEFT_SIDE;
		}
		circle(draw, massCenter, 3, Scalar(0, 0, 255), 2);
		//~ drawContours(draw, contours, i, Scalar(255, 0, 0));
		//~ drawContours(draw, polygons, i, Scalar(255, 0, 255));
		singles.push_back(single);
	}
	std::vector<YellowTote> detected_totes = pairTotes(singles);
	// operation: split the detected totes into ones that look stacked and ones that aren't
	std::vector<YellowTote> stacked_totes, unstacked_totes;
	printf("~~~~~~~~~~~~~~~~~~\n");
	for (size_t tote_i = 0; tote_i < detected_totes.size(); tote_i++) {
		for (size_t other_i = tote_i + 1; other_i < detected_totes.size(); other_i++) {
			printf("Ti: %zu Oi: %zu X: %d Y: %d oX: %d oY: %d\n", tote_i, other_i, detected_totes[tote_i].x, detected_totes[tote_i].y, detected_totes[other_i].x, detected_totes[other_i].y);
			if (abs(detected_totes[tote_i].x - detected_totes[other_i].x) < 20) {
				if (!detected_totes[tote_i].paired) {
					detected_totes[tote_i].paired = true;
					stacked_totes.push_back(detected_totes[tote_i]);
				}
				if (!detected_totes[other_i].paired) {
					detected_totes[other_i].paired = true;
					stacked_totes.push_back(detected_totes[other_i]);
				}
			} else {
				if (!detected_totes[tote_i].paired) {
					detected_totes[tote_i].paired = true;
					unstacked_totes.push_back(detected_totes[tote_i]);
				}
				if (!detected_totes[other_i].paired) {
					detected_totes[other_i].paired = true;
					unstacked_totes.push_back(detected_totes[other_i]);
				}
			}
			break;
		}
	}
	printf("stacked %zu unstacked %zu \n", stacked_totes.size(), unstacked_totes.size());
	imshow("Drawing", draw);
	//~ sort(detected_totes.begin(), detected_totes.end(), [] (YellowTote one, YellowTote two) {
	//~ return one.center.y < two.center.y;
	//~ });
	return detected_totes;
}

std::vector<YellowTote> find_yellow_color(cv::Mat img)
{
	std::vector<YellowTote> totes;
	Mat hsv, binary, draw;
	draw = img.clone();
	cvtColor(img, hsv, CV_BGR2HSV);
	inRange(hsv, Scalar(0, 128, 100), Scalar(30, 255, 255), binary);
	erode(binary, binary, kern, Point(-1,-1), 1);
	dilate(binary, binary, kern, Point(-1,-1), 1);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	DEBUG_SHOW("binary image", binary);
	findContours(binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<vector<Point> > polygons(contours.size());
	for (size_t i = 0; i < contours.size(); i++) {
		vector<Point> contour = contours[i];
		int area = contourArea(contour);
		if (area > 100000 || area < 500) {
			continue;
		}
		drawContours(draw, contours, i, COLOR_RED);
		vector<Point> polygon;
		approxPolyDP(contour, polygon, 5, true);
		polygons[i] = polygon;
		drawContours(draw, polygons, i, COLOR_GREEN);
		Rect bound = boundingRect(polygon);
		RotatedRect rotated = minAreaRect(polygon);
		rectangle(draw, bound, COLOR_BLUE);
		Point2f vertices[4];
		rotated.points(vertices);
		for (int i = 0; i < 4; i++) {
			line(draw, vertices[i], vertices[(i+1)%4], COLOR_WHITE);
		}
		Point2f center;
		// check to see if the contour and the box have similar area
		double areaRatio = static_cast<double>(bound.area()) / area;
		if (areaRatio > 0.8) {
			// if it is close to 1, then we are seeing a face probably
			// now check the height and width ratio
			double hwRatio = static_cast<double>(bound.height) / bound.width;
			if (hwRatio > 0.8 && hwRatio < 1.5) {
				// it's the square short side
				Moments moment = moments(contour, false);
				center = Point2f(moment.m10/moment.m00, moment.m01/moment.m00);
			} else {
				// it's the long side
				continue; // todo: return xrot as 90
			}
		} else {
			// if not, then we are seeing the box on the corner
			continue; // possible solution: split the boxes in half
		}
	}
	DEBUG_SHOW("Processed", draw);
	return totes;
}
