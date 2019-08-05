/*
Written by @Kolsha in 2019
*/
#include <QDebug>

#include "capturedialog.h"
#include "ui_capturedialog.h"



using namespace cv;

CaptureDialog::CaptureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaptureDialog)
{
    ui->setupUi(this);


    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);


    setAutoFillBackground(true);

    m_pal = QPalette(palette());
}

CaptureDialog::~CaptureDialog()
{
    delete ui;
}

void CaptureDialog::init_capture(QSize corners)
{
    QThread* thread = new QThread;
    m_stereo_input = new OpenCVStereoInput();



    m_stereo_input->moveToThread(thread);


    connect(thread, &QThread::started, m_stereo_input, &OpenCVStereoInput::process);

    connect(m_stereo_input, &OpenCVStereoInput::finished, thread, &QThread::quit);

    connect(m_stereo_input, &OpenCVStereoInput::finished, m_stereo_input, &OpenCVStereoInput::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    connect(m_stereo_input, &OpenCVStereoInput::new_images, this, &CaptureDialog::on_new_images);

    m_stereo_input->set_cam_idx(ui->left_camera_combo->currentIndex() - 1, ui->right_camera_combo->currentIndex() - 1);

    thread->start();

    ui->corner_count_x_spin->setValue(corners.width());
    ui->corner_count_y_spin->setValue(corners.height());

    ui->output_dir_line->clear();


    ui->current_message_label->setText("");

    ui->progress_bar->setValue(0);


}

QString CaptureDialog::get_outputdir()
{
    return ui->output_dir_line->text();
}

void CaptureDialog::on_new_images(cv::Mat left, cv::Mat right)
{

    ui->left_camera_image->setImage(left);
    ui->right_camera_image->setImage(right);

    {
        QMutexLocker locker(&m_grubber_mutex);
        if(m_grubber){
            m_grubber->on_new_images(left, right);
        } else {
            ui->current_message_label->setText(QString("Real size left: %1x%2, right: %3x%4").
                                               arg(left.cols).
                                               arg(left.rows).
                                               arg(right.cols).
                                               arg(right.rows));
        }
    }
}







void CaptureDialog::on_CaptureDialog_finished(int )
{
    m_stereo_input->stop();
}


void CaptureDialog::on_left_camera_combo_currentIndexChanged(int index)
{
    if(!m_stereo_input->set_cam_idx(index - 1, ui->right_camera_combo->currentIndex() - 1)){
        ui->left_camera_combo->setCurrentIndex(0);
    }
}

void CaptureDialog::on_right_camera_combo_currentIndexChanged(int index)
{
    if(!m_stereo_input->set_cam_idx(ui->left_camera_combo->currentIndex() - 1, index - 1)){
        ui->right_camera_combo->setCurrentIndex(0);
    }
}

void CaptureDialog::on_auto_detect_corners_check_stateChanged(int arg1)
{
    ui->corner_count_x_spin->setEnabled(arg1 > 0);
    ui->corner_count_y_spin->setEnabled(arg1 > 0);


}



void CaptureDialog::on_capture_button_clicked()
{


    {
        QMutexLocker locker(&m_grubber_mutex);
        if(m_grubber){
            m_grubber->grub();
            return;
        }
    }


    if(ui->left_camera_combo->currentIndex() < 1 || ui->right_camera_combo->currentIndex() < 1){
        QMessageBox::warning(this, "Capture error", "Please choose cameras!");
        return ;
    }

    if(ui->left_camera_combo->currentIndex() == ui->right_camera_combo->currentIndex()){
        if (QMessageBox::No == QMessageBox::question(this,
                                                     tr("Capture"),
                                                     tr("Are you sure what u want cap from same cameras?")))
        {
            return ;
        }
    }




    QString ouput_dir = ui->output_dir_line->text();




    if(ouput_dir.isEmpty() || !QDir(ouput_dir).exists()){
        QMessageBox::warning(this, "Capture error", "Please specify output directory!");
        return ;
    }


    int max_count = ui->max_images_spin->value();

    bool auto_detect_corners = ui->auto_detect_corners_check->isChecked();

    QSize corners(ui->corner_count_x_spin->value(), ui->corner_count_y_spin->value());

    if(auto_detect_corners && (corners.width() < 1 || corners.height() < 1)){
        QMessageBox::warning(this, "Capture error", "Please specify valid corners size!");
        return ;
    }

    ui->progress_bar->setMaximum(max_count);


    int timeout = -1;

    if(ui->mode_auto_button->isChecked()){

        bool ok;
        timeout = QInputDialog::getInt(this, "Auto mode", "Specify timeout in sec.", 1, 1, 100, 1, &ok);
        if(!ok) return;

    }



    QThread* thread = new QThread;
    m_grubber = new OpenCVGrabber(ouput_dir, max_count, auto_detect_corners, corners);









    m_grubber->moveToThread(thread);

    connect(thread, &QThread::started, this, &CaptureDialog::on_grub_started);

    connect(thread, &QThread::started, m_grubber, &OpenCVGrabber::process);



    connect(m_grubber, &OpenCVGrabber::finished, this, &CaptureDialog::on_grub_finished);

    connect(m_grubber, &OpenCVGrabber::new_grubbed, this, &CaptureDialog::on_new_grubbed);

    connect(m_grubber, &OpenCVGrabber::error, this, &CaptureDialog::on_error);


    connect(m_grubber, &OpenCVGrabber::finished, thread, &QThread::quit);

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);



    thread->start();



    if(timeout > 0){
        QTimer *timer = new QTimer(this);

        connect(timer, &QTimer::timeout, this, &CaptureDialog::on_grub_timer);

        connect(m_grubber, &OpenCVGrabber::finished, timer, &QTimer::stop);

        connect(m_grubber, &OpenCVGrabber::finished, timer, &QTimer::deleteLater);

        timer->start(timeout * 1000);
    }
}

void CaptureDialog::on_grub_timer()
{
    {
        QMutexLocker locker(&m_grubber_mutex);
        if(m_grubber){
            m_grubber->grub();
            return;
        }
    }

}

void CaptureDialog::on_grub_started()
{

    ui->progress_bar->setValue(0);
    ui->current_message_label->setText("Grub started");
    for(auto&& child:ui->grub_group->findChildren<QWidget *>()){
        child->setEnabled(false);
    }
}

void CaptureDialog::on_grub_finished()
{
    QMessageBox::information(this, "Capture", "Grubbed stopped");
    for(auto&& child:ui->grub_group->findChildren<QWidget *>()){
        child->setEnabled(true);
    }



    on_auto_detect_corners_check_stateChanged(ui->auto_detect_corners_check->isChecked());

    {
        QMutexLocker locker(&m_grubber_mutex);
        m_grubber->deleteLater();
        m_grubber = nullptr;
    }
}

void CaptureDialog::on_new_grubbed(int count)
{
    ui->progress_bar->setValue(count);
    ui->current_message_label->setText(QString("%1 pairs grubbed").arg(count));


    if( m_pal.color(QPalette::Background) != Qt::cyan ){
        m_pal.setColor(QPalette::Background, Qt::cyan);
    } else {
        m_pal.setColor(QPalette::Background, Qt::yellow);
    }

    setPalette(m_pal);
}

void CaptureDialog::on_error(const QString &msg)
{
    QMessageBox::warning(this, "Error", msg);
}



void CaptureDialog::on_close_cancel_button_clicked()
{
    if(m_grubber){
        if (QMessageBox::Yes == QMessageBox::question(this,
                                                      tr("Capture"),
                                                      tr("Capture already running. Do you want to stop it?")))
        {
            QMutexLocker locker(&m_grubber_mutex);
            if(m_grubber){
                m_grubber->stop();
                return;
            }
        }
        return;
    }

    if(ui->progress_bar->value() >= 2){
        accept();
    } else{
        reject();
    }
}


void CaptureDialog::on_output_dir_button_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty()){
        dir += QDir::separator();
        ui->output_dir_line->setText(dir);
    }
}
