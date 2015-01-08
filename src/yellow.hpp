#include <vector>
#include <opencv2/core/core.hpp>

enum Side {
	UNK_SIDE = 0,
	LONG_SIDE,
	SHORT_SIDE
};

class YellowTote {
public:
	YellowTote(Side side, int x, int y); // constructor
	Side facingSide;
	double distance;
	double xrot();    // function
	cv::Point2f point();
	int x, y;         // variable
	bool paired = false;
	friend bool operator==(const YellowTote& one, const YellowTote& two);
};

std::vector<YellowTote> find_yellow_ir(cv::Mat img);

std::vector<YellowTote> find_yellow_color(cv::Mat img);
