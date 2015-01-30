#ifndef YELLOW_HPP
#define YELLOW_HPP

#include <vector>
#include <opencv2/core/core.hpp>

const cv::Point2f fov(58, 45);

enum Piece_Type {
    OBJECT_UNKNOWN = 0,
    OBJECT_GREY_TOTE = 1,
    OBJECT_YELLOW_TOTE = 2,
    OBJECT_GREEN_BIN = 3
};

class Game_Piece {
private:
    float x_rot;
    float distance;
    float rotation;
    int totes_high;
    //ints, -1 = default, 1 = gray tote, 2 = yellow tote, 3 = bin
    int piece_type;
    bool green_bin_top;
    cv::Point2f center;

public:
    Game_Piece(); // constructor

    //mutalators
    void set_xrot(float xrot);
    void set_distance(float distance);
    void set_rotation(float r0t);
    void set_totes_high(int stacks);
    void set_piece_type(int type);
    void set_green_bin(bool on_top);
    void set_center(cv::Point2f center);

    //accessors
    float get_xrot();
    float get_distance();
    float get_rotation();
    int get_totes_high();
    int get_piece_type();
    std::string get_piece_type_str();
    bool get_green_bin();
    cv::Point2f get_center();

    friend void Display_Game_Piece(const Game_Piece object, cv::Mat img, cv::Point origin);
};

enum Side {
    UNK_SIDE = 0,
    LONG_SIDE,
    SHORT_SIDE
};

class YellowTote {
private:
    double area_ratio;
    Side facing_Side;
    double xrot;
    int stack_height;
    cv::Point2f center;
    double offset;
    double distance;

public:
    YellowTote();
    YellowTote(Side side);
    //constructor

    //mutalators
    void set_side(Side s);
    void set_ratio(double ratio);
    void set_xrot(double rotation);
    void set_stacked(int stacked);
    void set_center(cv::Point2f c);
    void set_offset(double offset);
    void set_distance(double distance);

    //accessors
    Side get_side();
    double get_ratio();
    double get_xrot();
    int get_stacked();
    cv::Point2f get_center();
    float get_center_x();
    float get_center_y();
    double get_offset();
    double get_distance();

    //functions
    friend void Display_YellowTote(const YellowTote tote, cv::Mat img, cv::Point origin);
    YellowTote operator=(YellowTote tote);
    friend bool operator==(YellowTote& one, YellowTote& two);
    friend bool operator!=(YellowTote& one, YellowTote& two);

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
