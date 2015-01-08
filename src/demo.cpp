#include <opencv2/highgui/highgui.hpp>
#include <free.hpp>
#include "yellow.hpp"

using namespace cv;

int main() {
	while (true) {
		Mat color = imread("../images/color.jpg");
		std::vector<YellowTote> totes = find_yellow_color(color);
		imshow("Input", color);
		int key = cv::waitKey(33) & 0xFF;
		if (key == 27) {
			break;
		}
	}
	cv::destroyAllWindows();
}
