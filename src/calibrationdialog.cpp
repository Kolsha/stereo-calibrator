/*
Written by @Kolsha in 2019
*/

#include "calibrationdialog.h"
#include "ui_calibrationdialog.h"

CalibrationDialog::CalibrationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrationDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

}

CalibrationDialog::~CalibrationDialog()
{
    delete ui;
}

void CalibrationDialog::start_calibration(const QVector<QString> &images,
                                          const QSize &corners,
                                          const QSizeF &square_size,
                                          const int resize_scale,
                                          const int auto_search_iter_count,
                                          const OpenCVCalibrator::BestCalibrationSelection bcs)
{
    QThread* thread = new QThread;
    m_calibrator = new OpenCVCalibrator(images, corners, square_size, resize_scale, auto_search_iter_count);

    m_calibrator->setBestCalibrationSelection(bcs);

    m_calibrator->moveToThread(thread);


    connect(thread, &QThread::started, m_calibrator, &OpenCVCalibrator::process);
    connect(m_calibrator, &OpenCVCalibrator::finished, thread, &QThread::quit);





    connect(m_calibrator, &OpenCVCalibrator::finished, m_calibrator, &OpenCVCalibrator::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);



    connect(m_calibrator, &OpenCVCalibrator::finished, this, &CalibrationDialog::on_calibrator_finished);
    connect(m_calibrator, &OpenCVCalibrator::info, this, &CalibrationDialog::on_info);
    connect(m_calibrator, &OpenCVCalibrator::result, this, &CalibrationDialog::on_result);




    thread->start();

    ui->message_text->clear();

}

void CalibrationDialog::on_info(QString msg)
{
    ui->message_text->append(msg);
}

void CalibrationDialog::on_result(QString calibration)
{
    m_last_calibration = calibration;
    save_calibration();

}

void CalibrationDialog::on_calibrator_finished()
{
    m_calibrator = nullptr;
    on_info("Calibration ended.");
}

void CalibrationDialog::on_close_button_clicked()
{
    if(m_calibrator){
        if (QMessageBox::Yes == QMessageBox::question(this,
                                                      tr("Calibration"),
                                                      tr("Calibration already running. Do you want to stop it?")))
        {
            m_calibrator->stop();
        }
        return;
    }
    accept();
}

void CalibrationDialog::save_calibration()
{
    if(m_last_calibration.isEmpty()){
        QMessageBox::information(this, tr("Info"),
                                 tr("Nothing to save!"));
        return;
    }
    while (QMessageBox::Yes == QMessageBox::question(this,
                                                     tr("Calibration"),
                                                     tr("Calibration complete. Do you want to save it?")))
    {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save Calibration"), "calibration",
                                                        tr("Calibration (*.json);;"));
        if(fileName.isEmpty())
            continue;


        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            continue;
        }

        file.write(m_last_calibration.toLocal8Bit());

        file.close();

        on_info("Calibration saved to " + fileName);

        break;


    }
}

void CalibrationDialog::on_save_button_clicked()
{
    save_calibration();
}
