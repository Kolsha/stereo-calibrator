/*
Written by @Kolsha in 2019
*/

#ifndef OPENCVCALIBRATOR_H
#define OPENCVCALIBRATOR_H

#include <QObject>
#include <QVector>
#include <QThread>
#include <QSize>
#include <QSizeF>

#include <QThreadPool>

#include <QMutex>
#include <QMutexLocker>

#include <opencv2/core.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/opencv.hpp>




using namespace std;
using namespace cv;



struct StereoCalibResult {
    Mat cameraMatrix[2], distCoeffs[2];


    Mat R, T, E, F;

    Mat R1, R2, P1, P2, Q;

    float triangulation_error;

    float opencv_reprojection_error;

    float epipolar_error;

    float euclidean_dist_error;

    Size image_size;
};










class OpenCVCalibrator : public QObject
{
    Q_OBJECT

public:

    enum BestCalibrationSelection{
        bcsByTriangulationError,
        bcsByOpenCVError,
        bcsByEpipolarError,
        bcsByEpipolarAndTriangulationErrors,
        bcsByEuclideanDistanceError,
        bcsByEuclideanDistanceAndTriangulationErrors,
        bcsByEuclideanDistanceOrTriangulationErrors,
    };

    Q_ENUMS(BestCalibrationSelection)

    explicit OpenCVCalibrator(const QVector<QString> &images, const QSize &corners,
                              const QSizeF &square_size, const int resize_scale,
                              const int auto_search_iter_count = -1,
                              const BestCalibrationSelection bcs = bcsByTriangulationError);

    void stop(){
        m_stopped = true;
    }

    void setBestCalibrationSelection(BestCalibrationSelection bcs){
        m_bcs = bcs;
    }

    ~OpenCVCalibrator(){
        delete m_pool;
    }

public slots:
    void process();

    void on_calib_tester_finish(StereoCalibResult calib, int excluded);


signals:
    void finished();
    void info(QString msg);

    void result(QString calibration);



private:

    bool m_stopped = false;

    QVector<QString> m_images;
    QSize m_corners;
    QSizeF m_square_size;
    int m_resize_scale = 1;
    int m_auto_calib_result_excluded = -1;

    BestCalibrationSelection m_bcs = bcsByTriangulationError;

    QThreadPool *m_pool = nullptr;

    StereoCalibResult m_auto_calib_result;
    QMutex m_auto_calib_mutex;


    StereoCalibResult m_global_minimum;

    int m_auto_search_iter_count;

    bool is_new_calib_better(const StereoCalibResult&old_calib, const StereoCalibResult& new_calib);
};


class CalibTester : public QObject, public QRunnable
{
    Q_OBJECT
public:

    void run();





    vector<vector<Point2f> > img_pts[2];
    vector<vector<Point3f> > obj_pts;
    Size image_size;

    int excluded = 0;

    vector<vector<Point2f> > *img_pts_holdout = nullptr;

signals:
    void result(StereoCalibResult calib, int excluded);
};


#endif // OPENCVCALIBRATOR_H
