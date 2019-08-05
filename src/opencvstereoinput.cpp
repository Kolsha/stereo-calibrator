/*
Written by @Kolsha in 2019
*/

#include "opencvstereoinput.h"


bool OpenCVStereoInput::set_cam_idx(int left, int right)
{
    QMutexLocker locker(&m_idx_mutex);
    if(left < 0 && right < 0){
        return false;
    }

    if(left == m_left_cam_idx && right == m_right_cam_idx){
        return true;
    }

    cv::Mat img;

    if(left != m_left_cam_idx){
        if(m_left_capture.open(left) && m_left_capture.read(img) && !img.empty()){
            m_left_cam_idx = left;
        } else{
            //m_left_capture.
            return false;
        }
    }

    if(right != m_right_cam_idx){
        if(m_right_capture.open(right) && m_right_capture.read(img) && !img.empty()){
            m_right_cam_idx = right;
        }else{
            //m_right_capture.release();
            return false;
        }
    }

    return true;

}

void OpenCVStereoInput::process()
{
    while(!m_stopped){
        QThread::currentThread()->msleep(20);
        cv::Mat left, right;
        QMutexLocker locker(&m_idx_mutex);

        if(m_left_capture.isOpened()){
            m_left_capture.read(left);
        }

        if(m_right_capture.isOpened()){
            m_right_capture.read(right);
        }

        emit new_images(left, right);
    }

    m_left_capture.release();
    m_right_capture.release();
    emit finished();

}
