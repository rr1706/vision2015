TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += demo.cpp \
    yellow.cpp \
    ../lib/free.cpp

HEADERS += \
    util.hpp \
    yellow.hpp \
    ../lib/free.hpp

LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lfreenect_sync
INCLUDEPATH += ../lib
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_DEBUG += -DDEBUG
