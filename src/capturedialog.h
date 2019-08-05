/*
Written by @Kolsha in 2019
*/

#ifndef CAPTUREDIALOG_H
#define CAPTUREDIALOG_H

#include <QDialog>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/opencv.hpp>

#include <QMutex>
#include <QMutexLocker>
#include <QThread>

#include <QMessageBox>
#include <QFileDialog>

#include <QInputDialog>

#include <QTimer>

#include "opencvstereoinput.h"
#include "opencvgrabber.h"









namespace Ui {
class CaptureDialog;
}

class CaptureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CaptureDialog(QWidget *parent = 0);
    ~CaptureDialog();

    void init_capture(QSize corners = QSize());

    QString get_outputdir();
public slots:
    void on_new_images(cv::Mat left, cv::Mat right);




private slots:
    void on_CaptureDialog_finished(int result);

    void on_left_camera_combo_currentIndexChanged(int index);

    void on_right_camera_combo_currentIndexChanged(int index);

    void on_auto_detect_corners_check_stateChanged(int arg1);

    void on_capture_button_clicked();

    void on_grub_timer();

    void on_grub_started();
    void on_grub_finished();

    void on_new_grubbed(int count);

    void on_error(const QString &msg);

    void on_close_cancel_button_clicked();


    void on_output_dir_button_clicked();

private:
    Ui::CaptureDialog *ui;
    OpenCVStereoInput *m_stereo_input = nullptr;
    OpenCVGrabber *m_grubber = nullptr;

    QMutex m_grubber_mutex;

    QPalette m_pal;
};






#endif // CAPTUREDIALOG_H
