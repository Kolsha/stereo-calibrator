/*
Written by @Kolsha in 2019
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QWidget>
#include <QDir>
#include <QList>
#include <QDebug>
#include <QFileDialog>
#include <QStringListModel>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QMetaEnum>


#include "capturedialog.h"
#include "calibrationdialog.h"

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_images_list_doubleClicked(const QModelIndex &index);

    void on_select_all_button_clicked();

    void on_select_none_button_clicked();

    void on_change_dir_button_clicked();

    void on_capture_button_clicked();

    void on_calibrate_button_clicked();

    void on_sort_radio_clicked();

private:
    Ui::MainWindow *ui;

    QStandardItemModel *model = nullptr;

    void set_check_state_for_all(const Qt::CheckState &state);

    bool load_images_from_dir(const QString &dir_path, const QDir::SortFlags &sort);

    QString m_dir_path;
    QDir::SortFlags m_sort = QDir::Name;


    CaptureDialog m_capturedialog;
    CalibrationDialog m_calibrationdialog;

    QMetaEnum m_calib_search_m_e;

};

#endif // MAINWINDOW_H
