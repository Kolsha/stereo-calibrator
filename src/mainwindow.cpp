/*
Written by @Kolsha in 2019
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "opencv2/calib3d.hpp"

#include <QListWidgetItem>
#include <QItemDelegate>
#include <QMessageBox>

#include <QApplication>

#include <QProcess>

#include <QMetaObject>
#include <QMetaEnum>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->sort_name_radio, &QRadioButton::clicked, this, &MainWindow::on_sort_radio_clicked);
    connect(ui->sort_time_radio, &QRadioButton::clicked, this, &MainWindow::on_sort_radio_clicked);

    m_calib_search_m_e = OpenCVCalibrator::staticMetaObject.enumerator(OpenCVCalibrator::staticMetaObject.indexOfEnumerator("BestCalibrationSelection"));

    for(int i=0; i < m_calib_search_m_e.keyCount(); ++i)
    {
        ui->calib_search_combo->addItem(m_calib_search_m_e.key(i));
    }

    ui->calib_search_combo->setCurrentIndex(-1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_images_list_doubleClicked(const QModelIndex &index)
{

    Qt::CheckState state = (model->item(index.row(), 0)->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
    model->item(index.row(), 0)->setCheckState(state);
}



void MainWindow::set_check_state_for_all(const Qt::CheckState &state)
{
    if(!model){
        return ;
    }

    for(int i = 0; i < model->rowCount(); i++){
        model->item(i, 0)->setCheckState(state);
    }
}

bool MainWindow::load_images_from_dir(const QString &dir_path, const QDir::SortFlags &sort)
{
    if(dir_path.isEmpty())
        return false;


    QStringList thumbnails;
    QDir dir(dir_path);
    QStringList filter;
    filter << QLatin1String("*.png") << QLatin1String("*.jpeg") << QLatin1String("*.jpg");
    dir.setNameFilters(filter);

    dir.setSorting(sort);



    QFileInfoList filelistinfo = dir.entryInfoList();
    foreach (const QFileInfo &fileinfo, filelistinfo) {
        thumbnails << fileinfo.absoluteFilePath();
        qApp->processEvents();
    }

    qDebug() << dir << filelistinfo;

    int count = thumbnails.count();
    qDebug()<<"The Total Images are" << count;

    if(count % 2 != 0)
        count--;

    if(count < 2){
        QMessageBox::warning(this, "Warning", "Few images count!");
        return false;
    }


    const int numRows = count / 2;

    if(model){
        model->deleteLater();
    }

    model = new QStandardItemModel(numRows, 3, this);

    for(int i = 0;i < numRows;++i)
    {
        qApp->processEvents();
        auto fn1 = QFileInfo(thumbnails.at(i * 2 + 0)).baseName();

        auto fn2 = QFileInfo(thumbnails.at(i * 2 + 1)).baseName();

        model->setData( model->index(i, 0), QString("%1 / %2").arg(fn1, fn2), Qt::DisplayRole);
        model->item(i, 0)->setCheckable(true);

        model->item(i, 0)->setCheckState(Qt::Checked);


        for(int pos = 1; pos <= 2; pos++){
            QIcon icon( thumbnails.at( i * 2 + pos - 1 ) );
            if ( icon.isNull() ){
                qDebug() << "Failed to load pixmap from file: " << thumbnails.at( i * 2 + pos - 1 );
                continue;
            }


            model->setData( model->index(i, pos), thumbnails.at( i * 2 + pos - 1 ), Qt::UserRole);
            model->item(i, pos)->setIcon(icon);
        }

    }



    ui->images_list->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    ui->images_list->setIconSize(QSize(200,200));

    ui->images_list->setModel(model);

    m_dir_path = dir_path;

    return true;
}


void MainWindow::on_select_all_button_clicked()
{
    set_check_state_for_all(Qt::Checked);
}


void MainWindow::on_select_none_button_clicked()
{
    set_check_state_for_all(Qt::Unchecked);
}

void MainWindow::on_change_dir_button_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!load_images_from_dir(dir, m_sort)){
        QMessageBox::warning(this, "Warning", "Fail to load images");
    } else{
        QMessageBox::information(this, "Ok", "Images loaded");
        ui->statusbar->showMessage(QString("%1 pairs of image").arg(model->rowCount()));
    }
}

void MainWindow::on_capture_button_clicked()
{

    m_capturedialog.init_capture(QSize(ui->corner_count_x_spin->value(), ui->corner_count_y_spin->value()));
    if(m_capturedialog.exec() == QDialog::Accepted){
        load_images_from_dir(m_capturedialog.get_outputdir(), m_sort);
    }
}

void MainWindow::on_calibrate_button_clicked()
{
    if(!model){
        QMessageBox::warning(this, "Error", "Please select images");
        return ;
    }

    QVector<QString> images;

    for(int i = 0; i < model->rowCount(); i++){
        if(model->item(i, 0)->checkState() != Qt::Checked){
            continue;
        }



        for(int pos = 1; pos <= 2; pos++){
            int real_pos = (ui->swap_check->isChecked()) ? (3 - pos) : pos;
            images.push_back(model->item(i, real_pos)->data(Qt::UserRole).toString());
        }

    }

    if(!images.size()){
        QMessageBox::warning(this, "Error", "Please select images");
        return;
    }



    QSize corners(ui->corner_count_x_spin->value(), ui->corner_count_y_spin->value());

    QSizeF square_size(ui->corners_width_line->value(), ui->corners_height_line->value());
    int reduce_size = ui->reduce_spin->value();

    if((square_size.width() * square_size.height()) <= 0){
        QMessageBox::warning(this, "Error", "Please fill square size");
        return;
    }

    if(ui->auto_calib_check->isChecked() && (ui->calib_iter_spin->value() + 5)>= images.size()){
        QMessageBox::warning(this, "Error", "Too many iters for auto search best calibration!");
        return;
    }

    int auto_search_iter_count = ui->auto_calib_check->isChecked() ? ui->calib_iter_spin->value() : 0;


    if(ui->auto_calib_check->isChecked() && ui->calib_search_combo->currentText().isEmpty()){
        QMessageBox::warning(this, "Error", "Select algo!");
        return;
    }

    OpenCVCalibrator::BestCalibrationSelection bcs =  static_cast<OpenCVCalibrator::BestCalibrationSelection>(
                m_calib_search_m_e.keyToValue(ui->calib_search_combo->currentText().toStdString().c_str())
                );

    m_calibrationdialog.start_calibration(images, corners, square_size, reduce_size, auto_search_iter_count, bcs);

    if(m_calibrationdialog.exec() == QDialog::Accepted){
        //todo
    }
}

void MainWindow::on_sort_radio_clicked()
{
    QDir::SortFlags new_flags = (ui->sort_name_radio->isChecked()) ? QDir::Name : QDir::Time;

    if(m_sort == new_flags){
        return ;
    }

    m_sort = new_flags;

    load_images_from_dir(m_dir_path, m_sort);
}


