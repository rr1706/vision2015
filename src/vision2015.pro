TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += demo.cpp \
    yellow.cpp \
    ../lib/free.cpp \
    udpsender.cpp \
    functions.cpp \
    color.cpp \
    depth.cpp \
    ir.cpp \
    solutionlog.cpp \
    ../lib/hsv.cpp \
    ../lib/input.cpp \
    robot.cpp

HEADERS += \
    util.hpp \
    yellow.hpp \
    ../lib/free.hpp \
    udpsender.hpp \
    tracker.hpp \
    solutionlog.hpp \
    ../lib/hsv.hpp \
    ../lib/input.hpp \
    functions.hpp

unix:LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lfreenect_sync
INCLUDEPATH += ../lib /usr/local/include
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_DEBUG += -DDEBUG

win32:INCLUDEPATH += C:/Users/connormonahan17/Documents/opencv/install/include
win32:LIBS += -LC:/Users/connormonahan17/Documents/opencv/install/x64/mingw/lib
win32:LIBS += -lopencv_core2410 -lopencv_imgproc2410 -lopencv_highgui2410
