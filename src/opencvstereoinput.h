/*
Written by @Kolsha in 2019
*/

#ifndef OPENCVSTEREOINPUT_H
#define OPENCVSTEREOINPUT_H

#include <QObject>
#include <QMetaType>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>

#include <QMutex>
#include <QMutexLocker>
#include <QThread>


class OpenCVStereoInput : public QObject {
    Q_OBJECT
public:
    OpenCVStereoInput(int left = -1, int right = -1) :
        m_left_cam_idx(left), m_right_cam_idx(right)
    {
        qRegisterMetaType<cv::Mat>("cv::Mat");
    }
    ~OpenCVStereoInput(){}
    bool set_cam_idx(int left, int right);
    void stop(){
        m_stopped = true;
    }
public slots:
    void process();
signals:
    void finished();
    void error(QString err);

    void new_images(cv::Mat left, cv::Mat right);
private:
    int m_left_cam_idx = -1;
    int m_right_cam_idx = -1;

    QMutex m_idx_mutex;
    bool m_stopped = false;

    cv::VideoCapture m_left_capture, m_right_capture;
};

#endif // OPENCVSTEREOINPUT_H
