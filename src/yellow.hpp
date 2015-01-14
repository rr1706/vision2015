#ifndef YELLOW_HPP
#define YELLOW_HPP

#include <vector>
#include <opencv2/core/core.hpp>

const cv::Point2f fov(58, 45);

class Game_Piece {
private:
    float x_rot;
    float distance;
    float ratio;
    int totes_high;
    //ints, -1 = default, 1 = gray tote, 2 = yellow tote, 3 = bin
    int piece_type;

public:
    Game_Piece(); // constructor

    //mutalators
    void set_xrot(float xrot);
    void set_distance(float distance);
    void set_ratio(float rat);
    void set_totes_high(int stacks);
    void set_piece_type(int type);

    //accessors
    float get_xrot();
    float get_distance();
    float get_ratio();
    bool get_totes_high();
    int get_piece_type();

    friend void Display_Game_Piece(const Game_Piece object, cv::Mat img, cv::Point origin);
};

enum Side {
    UNK_SIDE = 0,
    LONG_SIDE,
    SHORT_SIDE
};

class YellowTote {
private:
    Side facing_Side;
    double area_ratio;
    double xrot;
    int stack_height;
    cv::Point2f center;

public:
    YellowTote(Side side); //constructor

    //mutalators
    void set_side(Side s);
    void set_ratio(double ratio);
    void set_xrot(double rotation);
    void set_stacked(int stacked);
    void set_center(cv::Point2f c);

    //accessors
    Side get_side();
    double get_ratio();
    double get_xrot();
    int get_stacked();
    cv::Point2f get_center();
    float get_center_x();
    float get_center_y();

    //functions
    //overloading the extraction operator, but prints it out on the image at point origin
    friend void Display_YellowTote(const YellowTote tote, cv::Mat img, cv::Point origin);

    friend bool operator==(YellowTote& one, YellowTote& two);

};

enum LSide {
    LEFT_SIDE = 0,
    RIGHT_SIDE
};

//maybe make into class?
struct SingleL {
    cv::Point2f center;
    LSide side;
    cv::Rect bound;
    float area;
    bool paired = false;
};

#endif
