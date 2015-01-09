#include <opencv2/highgui/highgui.hpp>
#include <free.hpp>
#include "yellow.hpp"

using namespace cv;

int imdir() {
    int i = 0;
    while (true) {
        printf("%d\n", i);
        Mat color = imread("../images/yellow box/im_" + std::to_string(i) + ".jpg");
        std::vector<YellowTote> totes = find_yellow_color(color);
        imshow("Input", color);
        int raw = cv::waitKey(0) & 0xFFFF;
        printf("KEY: %X\n", raw);
        int key = raw & 0xFF;
        if ((raw & 0xFF00) == 0xFF00) {
            if (key == 0x51) {
                i--;
            } else if (key == 0x53) {
                i++;
            }
        } else if (key == 27) {
            break;
        }
    }
    cv::destroyAllWindows();
    return 0;
}

int main() {
    return imdir();
}
