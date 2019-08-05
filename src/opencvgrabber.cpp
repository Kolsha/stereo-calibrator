/*
Written by @Kolsha in 2019
*/

#include "opencvgrabber.h"

using namespace cv;

void OpenCVGrabber::process()
{
    int grubbed_count = 0;

    const int auto_detect_error_max = 20;
    int auto_detect_error_count = 0;
    while(!m_stopped){
        if(!m_grub_now || (m_left.size().empty() && m_right.size().empty()) ){
            QThread::currentThread()->msleep(200);
            continue;
        }
        m_grub_now = false;


        cv::Mat left, right;
        {
            QMutexLocker locker(&m_grub_mutex);

            m_left.copyTo(left);
            m_right.copyTo(right);
        }
        bool valid_images = true;

        if(m_auto_detect_corners){

            Size boardSize(m_corners.width(), m_corners.height());
            std::vector<Point2f> corners;

            valid_images = findChessboardCorners(left, boardSize, corners,
                                                 CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);

            if(valid_images){
                valid_images = findChessboardCorners(right, boardSize, corners,
                                                     CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
            }


        }


        if(!valid_images){
            if(m_auto_detect_corners)
                auto_detect_error_count++;

            if(auto_detect_error_count > auto_detect_error_max){
                emit error(QString("To many fail on autodetect, check corners"));
                break;
            }



            continue;
        }
        auto_detect_error_count = 0;
        grubbed_count++;
//        TODO: find corners

        QString left_fn = QString("%1%2_left.jpg").arg(m_output_dir).arg(grubbed_count);
        QString right_fn = QString("%1%2_right.jpg").arg(m_output_dir).arg(grubbed_count);



        cv::imwrite(left_fn.toStdString(), left);
        cv::imwrite(right_fn.toStdString(), right);



//        QThread::currentThread()->msleep(1000);


        emit new_grubbed(grubbed_count);
        if(grubbed_count >= m_max_count){
            break;
        }
    }

    emit finished();
}

void OpenCVGrabber::on_new_images(cv::Mat left, cv::Mat right)
{

    QMutexLocker locker(&m_grub_mutex);
    left.copyTo(m_left);
    right.copyTo(m_right);

    //qDebug() << m_right.size().height;

}
