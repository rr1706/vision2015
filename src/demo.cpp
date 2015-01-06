#include <opencv2/highgui/highgui.hpp>
#include <free.hpp>
#include "yellow.hpp"

using namespace cv;

int main() {
	cv::namedWindow("IR", CV_WINDOW_NORMAL);
	cv::namedWindow("Drawing", CV_WINDOW_NORMAL);
	while (true) {
		//~ cv::Mat ir = kinectIR(0);
		Mat ir = imread("../images/ir.png");
		cvtColor(ir, ir, CV_BGR2GRAY);
		std::vector<YellowTote> totes = find_yellow_ir(ir);
		cv::Mat drawing;
		cv::cvtColor(ir, drawing, CV_GRAY2BGR);
		imshow("IR", drawing);
		int key = cv::waitKey(33) & 0xFF;
		if (key == 27) {
			break;
		}
	}
	cv::destroyAllWindows();
}
