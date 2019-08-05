/*
Written by @Kolsha in 2019
*/

#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include <QVector>
#include <QMessageBox>
#include <QFileDialog>

#include "opencvcalibrator.h"


namespace Ui {
class CalibrationDialog;
}

class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationDialog(QWidget *parent = 0);
    ~CalibrationDialog();

    void start_calibration(const QVector<QString> &images,
                           const QSize &corners,
                           const QSizeF &square_size,
                           const int resize_scale = 1,
                           const int auto_search_iter_count = -1,
                           const OpenCVCalibrator::BestCalibrationSelection
                           bcs = OpenCVCalibrator::BestCalibrationSelection::bcsByEuclideanDistanceOrTriangulationErrors);

private slots:

    void on_info(QString msg);

    void on_result(QString calibration);

    void on_calibrator_finished();

    void on_close_button_clicked();

    void on_save_button_clicked();

private:
    Ui::CalibrationDialog *ui;

    void save_calibration();

    QString m_last_calibration;

    OpenCVCalibrator *m_calibrator = nullptr;
};

#endif // CALIBRATIONDIALOG_H
