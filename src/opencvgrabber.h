/*
Written by @Kolsha in 2019
*/

#ifndef OPENCVGRABBER_H
#define OPENCVGRABBER_H


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/opencv.hpp>

#include <QMutex>
#include <QMutexLocker>
#include <QThread>

#include <QSize>
#include <QSizeF>


class OpenCVGrabber : public QObject {
    Q_OBJECT
public:
    OpenCVGrabber(QString &output_dir, int max_count,
                  bool auto_detect_corners = false, QSize corners = QSize())
    {
        qRegisterMetaType<cv::Mat>("cv::Mat");
        m_auto_detect_corners = auto_detect_corners;
        m_corners = corners;
        m_output_dir = output_dir;
        m_max_count = max_count;
    }
    ~OpenCVGrabber(){}
    void stop(){
        m_stopped = true;
    }
public slots:
    void process();
    void on_new_images(cv::Mat left, cv::Mat right);
    void grub(){
        m_grub_now = true;
    }



signals:
    void finished();
    void new_grubbed(int count);
    void error(const QString &msg);
private:
    QString m_output_dir;
    QSize m_corners;
    bool m_auto_detect_corners = false;

    QMutex m_grub_mutex;
    bool m_grub_now = false;
    bool m_stopped = false;

    int m_max_count = 2;


    cv::Mat m_left, m_right;
};

#endif // OPENCVGRABBER_H
