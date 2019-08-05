#
##Written by @Kolsha in 2019
#

#-------------------------------------------------
#
# Project created by QtCreator 2018-10-18T21:25:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Calibtrator
TEMPLATE = app

CONFIG += thread
CONFIG += c++11


DEFINES += QT_DEPRECATED_WARNINGS


# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# Windows 7
win32:OPENCV_DIR = "C:/opencv/build"
win32:OPENCV_LIB_DIR = $$OPENCV_DIR/x86/vc10/lib

# Mac OS X
macx:OPENCV_DIR = "/usr/local/Cellar/opencv@3/3.4.5"
macx:OPENCV_LIB_DIR = $$OPENCV_DIR/lib

CV_LIB_NAMES = core imgproc highgui calib3d features2d flann videoio imgcodecs

for(lib, CV_LIB_NAMES) {
    CV_LIBS += -lopencv_$$lib
}

win32 {
    DEFINES += NOMINMAX _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS _USE_MATH_DEFINES
    QMAKE_CXXFLAGS_WARN_ON += -W3 -wd4396 -wd4100 -wd4996
    QMAKE_LFLAGS += /INCREMENTAL:NO
    DSHOW_LIBS = -lStrmiids -lVfw32 -lOle32 -lOleAut32

    CONFIG(release, debug|release) {
        CV_LIB_PREFIX = $$CV_VER
    }
    else {
        CV_LIB_PREFIX = $${CV_VER}d
    }
    for(lib, CV_LIBS) {
        CV_LIBS_NEW += $$lib$$CV_LIB_PREFIX
    }
    CV_LIBS = $$CV_LIBS_NEW $$CV_EXT_LIBS $$DSHOW_LIBS
}

LIBS += -L$$OPENCV_LIB_DIR $$CV_LIBS
INCLUDEPATH += $$SOURCEDIR $$UI_DIR $$OPENCV_DIR/include


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    capturedialog.cpp \
    ImageLabel.cpp \
    io_util.cpp \
    opencvstereoinput.cpp \
    calibrationdialog.cpp \
    opencvcalibrator.cpp \
    opencvgrabber.cpp

HEADERS += \
        mainwindow.h \
    capturedialog.h \
    ImageLabel.hpp \
    io_util.hpp \
    opencvstereoinput.h \
    calibrationdialog.h \
    opencvcalibrator.h \
    opencvgrabber.h

FORMS += \
        mainwindow.ui \
    capturedialog.ui \
    calibrationdialog.ui


DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
