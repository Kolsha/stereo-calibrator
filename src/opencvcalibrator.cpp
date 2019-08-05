/*
Written by @Kolsha in 2019
*/

#include <QDebug>
#include <QRunnable>
#include <QFile>
#include <QDir>
#include <limits>
#include <QMetaEnum>

#include "opencvcalibrator.h"


template<class _InputIterator, class _OutputIterator>
_OutputIterator
copy_except_one(_InputIterator __first, _InputIterator __last,
                _OutputIterator __result, size_t nth)
{
    _InputIterator begin = __first;

    for (; __first != __last; ++__first)
    {
        if ((begin + long(nth)) != __first)
        {
            *__result = *__first;
            ++__result;
        }
    }
    return __result;
}



/* exp = 7, 5 */
//const int calib_flags = CALIB_FIX_ASPECT_RATIO +

//        CALIB_USE_INTRINSIC_GUESS +
//        CALIB_SAME_FOCAL_LENGTH +
//        CALIB_TILTED_MODEL +



//        CALIB_THIN_PRISM_MODEL
//        + CALIB_FIX_PRINCIPAL_POINT
//        ;



///* exp = 8, 10 BEST NOW */
const int calib_flags =
        CALIB_FIX_ASPECT_RATIO +
        CALIB_USE_INTRINSIC_GUESS +
        CALIB_SAME_FOCAL_LENGTH +
        CALIB_TILTED_MODEL +
        CALIB_RATIONAL_MODEL +
        CALIB_THIN_PRISM_MODEL +
        CALIB_FIX_PRINCIPAL_POINT
        ;



/* exp = 9

  without CALIB_FIX_PRINCIPAL_POINT only nan


  without CALIB_FIX_PRINCIPAL_POINT  and without CALIB_SAME_FOCAL_LENGTH
  nan arises later

*/
//const int calib_flags =
//        CALIB_FIX_ASPECT_RATIO +
//        CALIB_USE_INTRINSIC_GUESS +
//        CALIB_TILTED_MODEL +
//        CALIB_RATIONAL_MODEL +
//        CALIB_THIN_PRISM_MODEL
//        ;

//const int calib_flags = CALIB_FIX_ASPECT_RATIO +

//        CALIB_USE_INTRINSIC_GUESS +
//        CALIB_SAME_FOCAL_LENGTH +
//        CALIB_TILTED_MODEL +

//        CALIB_RATIONAL_MODEL +

//        CALIB_THIN_PRISM_MODEL
//        + CALIB_FIX_PRINCIPAL_POINT
//        ;



/* exp = 6 */
//const int calib_flags = CALIB_FIX_ASPECT_RATIO +
////        CALIB_ZERO_TANGENT_DIST +
//        CALIB_USE_INTRINSIC_GUESS +
////        CALIB_SAME_FOCAL_LENGTH +
//        CALIB_TILTED_MODEL +

//        //CALIB_RATIONAL_MODEL +

//        CALIB_THIN_PRISM_MODEL
////        + CALIB_FIX_PRINCIPAL_POINT
//        //+ CALIB_FIX_K3 + CALIB_FIX_K4
//        //+ CALIB_FIX_K5
//        ;




/* exp = 12*/
//const int calib_flags =
//        CALIB_FIX_ASPECT_RATIO +
//        CALIB_ZERO_TANGENT_DIST +
//        CALIB_USE_INTRINSIC_GUESS +
//        CALIB_SAME_FOCAL_LENGTH +

//        CALIB_RATIONAL_MODEL + // 8
//        CALIB_THIN_PRISM_MODEL //+ // 12
////        CALIB_TILTED_MODEL + // 14

////        CALIB_FIX_PRINCIPAL_POINT
//        ;

#ifdef WRITE_LOGS

const size_t experiment_number = 2;

const QString base_dir_tpl = "/Users/kolsha/Documents/Study/8 term/Diploma/Experiment/13.03.19/calib_results/";
// "C:\\Users\\Kolsha\\Documents\\22.02.19\\new_calib\\";

#endif


/*
 * this flag allow calibrate to last photo.
 * for example if u have 40 photos and this flag is true,
 * in the end u receive calibration on 2 best photos.
 * else calibration process will stop automatically when error rise.
 */
const bool calibrate_to_end  = false;


const TermCriteria calib_term = TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 130, 1e-6);


Point3f get_point3f(const Mat &points_3d, const int pos){
    //                                x=0,                         y=1,                          z=2
    return Point3f(points_3d.at<float>(0, pos), points_3d.at<float>(1, pos), points_3d.at<float>(2, pos));
}


Mat get_triangulation(const StereoCalibResult &calib, const Mat img_pts[2]){


    Mat img_pts_res[2];
    for(int k = 0; k < 2; k++ )
    {

        undistortPoints(
                    img_pts[k],
                    img_pts_res[k],
                    calib.cameraMatrix[k],
                    calib.distCoeffs[k],
                    ((k == 0) ? calib.R1 : calib.R2)
                    ,((k == 0) ? calib.P1 : calib.P2)
                    );

    }


    Mat X;
    triangulatePoints(calib.P1, calib.P2, img_pts_res[0], img_pts_res[1], X);

    X.row(0) = X.row(0) / X.row(3);
    X.row(1) = X.row(1) / X.row(3);
    X.row(2) = X.row(2) / X.row(3);

    return X;
}


float calc_euclidean_dist(const cv::Point3f& a, const cv::Point3f& b)
{
    const cv::Point3f diff = a - b;
    return cv::sqrt(diff.dot(diff));
}


float calc_euclidean_dist_error(const Mat &points_3d, const vector<float> &distances){

    Q_ASSERT(size_t(points_3d.cols) <= distances.size());

    float mae = 0;

    float max_mae = 0;
    for(int i = 1; i < points_3d.cols; i++){
        Point3f a = get_point3f(points_3d, i - 1), b = get_point3f(points_3d, i);

        float dist = calc_euclidean_dist(a, b);

        float dist_error = fabs(distances[size_t(i - 1)] - dist);
        mae += dist_error;

        max_mae = max(max_mae, dist_error);

    }



    return mae / float(points_3d.cols);
}

float calc_euclidean_dist_error_all(const StereoCalibResult &calib,
                                    const vector<vector<Point2f> > imagePoints[2], const vector<Point3f> &objectPoints
) {


    const size_t nimages = imagePoints[0].size();
    float total_mae = 0;

    vector<float> distances;
    distances.reserve(objectPoints.size());

    for(size_t i = 1; i < objectPoints.size(); i++){
        float dist = calc_euclidean_dist(objectPoints[i - 1], objectPoints[i]);
        distances.push_back(dist);
    }

    for(size_t i = 0; i < nimages; i++ )
    {

        Mat imgpt[2] = {Mat(imagePoints[0][i]), Mat(imagePoints[1][i])};

        Mat X = get_triangulation(calib, imgpt);

        float current_mae = calc_euclidean_dist_error(X, distances);

        if(isnan(current_mae))
            current_mae = std::numeric_limits<float>::max();

        total_mae +=current_mae;


    }

    return total_mae / float(nimages);
}



float calc_triangulation_error(const Mat &points_3d){

    const int c_p1 = 2;
    const int c_p2 = points_3d.cols / 2;
    const int c_p3 = points_3d.cols - 1;



    Point3f p1(points_3d.at<float>(0, c_p1), points_3d.at<float>(1, c_p1), points_3d.at<float>(2, c_p1)),
            p2(points_3d.at<float>(0, c_p2), points_3d.at<float>(1, c_p2), points_3d.at<float>(2, c_p2)),
            p3(points_3d.at<float>(0, c_p3), points_3d.at<float>(1, c_p3), points_3d.at<float>(2, c_p3));

    Point3f v1(p3 - p1), v2(p2 - p1);
    Point3f cp = v1.cross(v2);




    float a = cp.x, b = cp.y, c = cp.z;

    float d = cp.dot(p3);



    Mat Z = (d - a * points_3d.row(0) - b * points_3d.row(1)) / c;

    Z = Z - points_3d.row(2);


    float mae = 0;
    for (int i = 0; i < Z.cols; i++){
        mae += fabs(Z.at<float>(0, i));
    }





    return (mae / float(Z.cols));

}


float calc_triangulation_error_all(const StereoCalibResult &calib,
                                   const vector<vector<Point2f> > imagePoints[2]) {

    const size_t nimages = imagePoints[0].size();
    float total_mae = 0;
    float max_mae = 0;
    for(size_t i = 0; i < nimages; i++ )
    {

        Mat imgpt[2] = {Mat(imagePoints[0][i]), Mat(imagePoints[1][i])};

        Mat X = get_triangulation(calib, imgpt);

        float current_mae = calc_triangulation_error(X);

        if(isnan(current_mae))
            current_mae = std::numeric_limits<float>::max();

        total_mae += current_mae;

        qDebug() << "Triang: " << current_mae << isnan(current_mae);

        if (current_mae >= max_mae){
            max_mae = current_mae;
        }

    }



    total_mae /= float(nimages);

    return total_mae;
}




QString convert_calib_to_string(const StereoCalibResult &calib){
    FileStorage fs("calibration.json", FileStorage::WRITE | FileStorage::FORMAT_JSON | FileStorage::MEMORY);
    if (fs.isOpened())
    {
        // intrisic
        fs << "K1" << calib.cameraMatrix[0];
        fs << "K2" << calib.cameraMatrix[1];

        fs << "D1" << calib.distCoeffs[0];
        fs << "D2" << calib.distCoeffs[1];

        fs << "F" << calib.F;
        fs << "E" << calib.E;


        //extrinsics
        fs << "R" << calib.R;
        fs << "T" << calib.T;
        fs << "R1" << calib.R1;
        fs << "R2" << calib.R2;
        fs << "P1" << calib.P1;
        fs << "P2" << calib.P2;
        fs << "Q" << calib.Q;

        fs << "IMGSIZE" << calib.image_size;

        fs << "OpenCV_reprojection_error" << calib.opencv_reprojection_error;

        fs << "Triangulation_ms_error" << calib.triangulation_error;

        fs << "Epipolar_error" << calib.epipolar_error;

        fs << "Euclidean_error" << calib.euclidean_dist_error;




        string calib = fs.releaseAndGetString();
        return QString::fromStdString(calib);
    }

    return "";

}




OpenCVCalibrator::OpenCVCalibrator(const QVector<QString> &images, const QSize &corners,
                                   const QSizeF &square_size, const int resize_scale,
                                   const int auto_search_iter_count, const BestCalibrationSelection bcs)
{

    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<size_t>("StereoCalibResult");
    m_images = images;
    m_corners = corners;
    m_square_size = square_size;
    m_resize_scale = (resize_scale > 0) ? resize_scale : 1;

    m_auto_search_iter_count = auto_search_iter_count;

    m_bcs = bcs;

}








StereoCalibResult calibrate_stereo(const vector<vector<Point2f> > imagePoints[2], const vector<vector<Point3f> > &objectPoints, Size imageSize){
    Rect validRoi[2];
    StereoCalibResult res;

    res.cameraMatrix[0] = initCameraMatrix2D(objectPoints, imagePoints[0], imageSize, 0);
    res.cameraMatrix[1] = initCameraMatrix2D(objectPoints, imagePoints[1], imageSize, 0);

    double reprojection_error =
            stereoCalibrate(objectPoints, imagePoints[0], imagePoints[1],
            res.cameraMatrix[0], res.distCoeffs[0],
            res.cameraMatrix[1], res.distCoeffs[1],
            imageSize, res.R, res.T, res.E, res.F
            ,
            calib_flags
            ,
            calib_term);

    res.opencv_reprojection_error = float(reprojection_error);

    stereoRectify(res.cameraMatrix[0], res.distCoeffs[0],
            res.cameraMatrix[1], res.distCoeffs[1],
            imageSize, res.R, res.T, res.R1, res.R2, res.P1, res.P2, res.Q,
            0/*CALIB_ZERO_DISPARITY*/, 1);//, 1, imageSize, &validRoi[0], &validRoi[1]);

    res.image_size = imageSize;

    return (res);
}






void save_all_triangulation_to_file(const StereoCalibResult &calib,
                                    const vector<vector<Point2f> > imagePoints[2], const QString &fn_prefix) {

    if(fn_prefix.isEmpty())
        return;



    const size_t nimages = imagePoints[0].size();

    for(size_t i = 0; i < nimages; i++ )
    {

        Mat imgpt[2] = {Mat(imagePoints[0][i]), Mat(imagePoints[1][i])};

        Mat X = get_triangulation(calib, imgpt);

        QString filename = fn_prefix + QString::number(i) + ".csv";
        QFile data(filename);
        if(!data.open(QFile::WriteOnly | QFile::Truncate))
            continue;

        QTextStream output(&data);

        output << "x,y,z" << endl;

        for (int c = 0; c < X.cols; c++) {

            float x = X.at<float>(0, c);
            float y = X.at<float>(1, c);
            float z = X.at<float>(2, c);

            output << x << "," << y << "," << z << endl;
        }

        data.close();


    }

}




float calc_epipolar_error_all(const StereoCalibResult &calib, const vector<vector<Point2f> > imagePoints[2]){
    const size_t nimages = imagePoints[0].size();
    float err = 0;
    int npoints = 0;
    vector<Vec3f> lines[2];
    for (size_t i = 0; i < nimages; i++)
    {
        size_t npt = imagePoints[0][i].size();

        Mat imgpt[2];
        for(int k = 0; k < 2; k++ )
        {

            undistortPoints(
                        imagePoints[k][i],
                        imgpt[k],
                        calib.cameraMatrix[k],
                        calib.distCoeffs[k],
                        ((k == 0) ? calib.R1 : calib.R2)
                        ,((k == 0) ? calib.P1 : calib.P2)
                        );

            computeCorrespondEpilines(imgpt[k], k + 1, calib.F, lines[k]);
        }

        for (size_t j = 0; j < npt; j++)
        {
            float errij = fabs(imagePoints[0][i][j].x*lines[1][j][0] +
                    imagePoints[0][i][j].y*lines[1][j][1] + lines[1][j][2]) +
                    fabs(imagePoints[1][i][j].x*lines[0][j][0] +
                    imagePoints[1][i][j].y*lines[0][j][1] + lines[0][j][2]);

            if(isnan(errij))
                errij = std::numeric_limits<float>::max();
            err += errij;
        }
        npoints += npt;
    }

    return err / float(npoints);
}



QString get_errors(const StereoCalibResult &calib){
    return QString("OpenCV: %1 \nTriangulation: %2 \nEpipolar: %3 \nEuclidean: %4").
            arg(double(calib.opencv_reprojection_error)).
            arg(double(calib.triangulation_error)).
            arg(double(calib.epipolar_error)).
            arg(double(calib.euclidean_dist_error));
}

void OpenCVCalibrator::process()
{



    emit info("Calibration started");

    Size boardSize(m_corners.width(), m_corners.height());
    Size imageSize;

    vector<vector<Point2f> > imagePoints[2];
    vector<vector<Point3f> > objectPoints;


    int i, j, k, nimages = int(m_images.size()) / 2;


    imagePoints[0].resize(size_t(nimages));
    imagePoints[1].resize(size_t(nimages));



    for (i = j = 0; i < nimages; i++)
    {

        for (k = 0; k < 2; k++)
        {
            if(m_stopped){
                emit finished();
                return;
            }
            const string& filename = m_images[i * 2 + k].toStdString();

            Mat img = imread(filename, cv::IMREAD_GRAYSCALE);

            // TODO: check original

            if (m_resize_scale != 1){
                double scale = 1.0 / double(m_resize_scale);
                resize(img, img, Size(), scale, scale, INTER_LINEAR_EXACT);
            }

            if (img.empty())
                break;
            if (imageSize == Size())
                imageSize = img.size();
            else if (img.size() != imageSize)
            {
                QString msg =
                        QString("The image %1 has the size different from the first image size. Skipping the pair").
                        arg(m_images[i * 2 + k]);
                emit info(msg);

                break;
            }



            bool found = false;
            vector<Point2f>& corners = imagePoints[size_t(k)][size_t(j)];



            found = findChessboardCorners(img, boardSize, corners,
                                          CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);

            emit info(".");
            if (!found)
                break;
            cornerSubPix(img, corners, Size(11, 11), Size(-1, -1),
                         TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
                                      300, 0.001));
        }
        if (k == 2)
        {
            j++;
        }
    }

    emit info(QString("%1  pairs have been successfully detected.").arg(j));

    nimages = j;
    if (nimages < 2)
    {
        emit info(QString("Error: too little pairs to run the calibration."));
        emit finished();
        return;
    }

    imagePoints[0].resize(size_t(nimages));
    imagePoints[1].resize(size_t(nimages));
    objectPoints.resize(size_t(nimages));

    const double test_size = 0.2;
    const long test_split = long(imagePoints[0].size() * test_size);





    const QMetaEnum auto_calib_search_m_e = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("BestCalibrationSelection"));

    const QString m_bcs_name = auto_calib_search_m_e.key(m_bcs);



    for (i = 0; i < nimages; i++)
    {
        for (j = 0; j < boardSize.height; j++)
            for (k = 0; k < boardSize.width; k++)
                objectPoints[i].push_back(Point3f(k * m_square_size.width(), j * m_square_size.height(), 0));
    }


    vector<vector<Point2f> > img_pts_holdout[2];// = {imagePoints[0], imagePoints[1]};

    vector<vector<Point3f> > obj_pts_holdout(objectPoints.begin(), objectPoints.begin() + test_split);

    objectPoints.erase(objectPoints.begin(), objectPoints.begin() + test_split);

    qDebug() << objectPoints.size() << " + " << obj_pts_holdout.size() << " = " << nimages;

    for (int k = 0; k < 2; k++){
        img_pts_holdout[k].resize(size_t(test_split));
        copy(imagePoints[k].begin(), imagePoints[k].begin() + test_split, img_pts_holdout[k].begin());

        imagePoints[k].erase(imagePoints[k].begin(), imagePoints[k].begin() + test_split);

    }



    emit info(QString("Running stereo calibration ..."));

    if(m_stopped){
        emit finished();
        return;
    }

    StereoCalibResult res = calibrate_stereo(imagePoints, objectPoints,imageSize);

    res.triangulation_error = calc_triangulation_error_all(res, img_pts_holdout);

    res.epipolar_error = calc_epipolar_error_all(res, img_pts_holdout);

    res.euclidean_dist_error = calc_euclidean_dist_error_all(res, img_pts_holdout, objectPoints[0]);



    emit info(QString("done with initial estimate \n%1").arg(get_errors(res)));







#ifdef WRITE_LOGS
    QString base_dir = base_dir_tpl + "exp_" + QString::number(experiment_number) + QDir::separator();


    QString base_filename = "as_by_";



    base_filename += m_bcs_name;
    base_dir += m_bcs_name;

    if(calibrate_to_end){
        base_dir += "_to_the_end";
    }

    base_dir += QDir::separator();



    if(!QDir(base_dir).exists() && !QDir().mkpath(base_dir)){

        emit finished();
        return ;

    }

    QString filename = base_dir + base_filename + ".csv";
    QFile data(filename);
    data.open(QFile::WriteOnly |QFile::Truncate);
    QTextStream output(&data);

    output << "OpenCV" << "," << "Epipolar" << "," << "Triangulation" << "," << "Euclidean" <<endl;
#endif


    m_auto_search_iter_count = MIN(m_auto_search_iter_count, int(imagePoints[0].size()) - 2);
    if(m_auto_search_iter_count > 0){

        m_global_minimum = res;

        emit info(
                    QString("Start search with iters = %1 and algo = %2").
                    arg(m_auto_search_iter_count).
                    arg(m_bcs_name)
                    );



        for(int iter = 0; iter < m_auto_search_iter_count; iter++){


#ifdef WRITE_LOGS
            output << res.opencv_reprojection_error << "," <<
                      res.epipolar_error << "," <<
                      res.triangulation_error << "," <<
                      res.euclidean_dist_error << endl;
#endif

            if(m_stopped){
                emit finished();
                return;
            }





            const size_t img_count = imagePoints[0].size();

            if(calibrate_to_end){

                m_auto_calib_result.epipolar_error = numeric_limits<float>::max();

                m_auto_calib_result.triangulation_error = numeric_limits<float>::max();

                m_auto_calib_result.opencv_reprojection_error = numeric_limits<float>::max();

                m_auto_calib_result.euclidean_dist_error = numeric_limits<float>::max();
            } else {
                m_auto_calib_result = res;
            }


            m_auto_calib_result_excluded = -1;

            m_pool = new QThreadPool(this);
            m_pool->setMaxThreadCount(4);
            m_pool->setExpiryTimeout(-1);

            for(size_t i = 0; i < img_count; i++){

                if(m_stopped){
                    emit finished();
                    return;
                }

                CalibTester *tester = new CalibTester();

                tester->image_size = imageSize;
                tester->excluded = int(i);
                tester->setAutoDelete(true);


                tester->img_pts[0].resize(img_count-1);
                tester->img_pts[1].resize(img_count-1);
                tester->obj_pts.resize(img_count-1);

                tester->img_pts_holdout = img_pts_holdout;




                copy_except_one(imagePoints[0].begin(), imagePoints[0].end(), tester->img_pts[0].begin(), i);

                copy_except_one(imagePoints[1].begin(), imagePoints[1].end(), tester->img_pts[1].begin(), i);

                copy_except_one(objectPoints.begin(), objectPoints.end(), tester->obj_pts.begin(), i);

                m_pool->start(tester);

                connect(tester, &CalibTester::result, this, &OpenCVCalibrator::on_calib_tester_finish, Qt::DirectConnection);

            }

            m_pool->waitForDone();
            m_pool->deleteLater();
            m_pool = nullptr;



            if( (!calibrate_to_end && !is_new_calib_better(res, m_auto_calib_result))
                    || m_auto_calib_result_excluded < 0){ // !is_new_calib_better(res, m_auto_calib_result) ||
                break;
            }

            res = m_auto_calib_result;


            objectPoints.erase(objectPoints.begin() + m_auto_calib_result_excluded);

            for(int k = 0; k < 2; k++){
                imagePoints[k].erase(imagePoints[k].begin() + m_auto_calib_result_excluded);

                //                std::rotate(imagePoints[k].begin(), imagePoints[k].begin() + 1, imagePoints[k].end());

                //                imagePoints[1].erase(imagePoints[1].begin() + pos);
            }

            emit info(
                        QString("Find new min, now count: %1 \n%2").
                        arg(objectPoints.size()).
                        arg(get_errors(res))

                        );




        }







        if(m_stopped){
            emit finished();
            return;
        }



        emit info(QString("done with final \n%1").arg(get_errors(res)));
    }

#ifdef WRITE_LOGS
    output << res.opencv_reprojection_error << "," <<
              res.epipolar_error << "," <<
              res.triangulation_error << "," <<
              res.euclidean_dist_error << endl;

    data.close();

    QString planes_filename = base_dir + "planes_holdout_" + base_filename;

    save_all_triangulation_to_file(res, img_pts_holdout, planes_filename);
#endif





    QString qcalib = convert_calib_to_string(res);
    if(!qcalib.isEmpty()){

#ifdef WRITE_LOGS
        QFile calib_file(base_dir + "calibration.json");
        calib_file.open(QFile::WriteOnly |QFile::Truncate);

        QTextStream calib_output(&calib_file);

        calib_output << qcalib;

        calib_file.close();
#endif




        emit result(qcalib);

    } else{
        // error
    }


    //    qcalib = convert_calib_to_string(m_global_minimum);
    //    if(!qcalib.isEmpty()){
    //        QFile calib_file(base_dir + "calibration_global.json");
    //        calib_file.open(QFile::WriteOnly |QFile::Truncate);

    //        QTextStream calib_output(&calib_file);

    //        calib_output << qcalib;

    //        calib_file.close();


    //        //        emit result(qcalib);

    //    } else{
    //        // error
    //    }





    emit finished();


}

void OpenCVCalibrator::on_calib_tester_finish(StereoCalibResult calib, int excluded)
{
    QMutexLocker locker(&m_auto_calib_mutex);
    qDebug() << "Signal: " << calib.triangulation_error << " at " << excluded;
    if (is_new_calib_better(m_auto_calib_result, calib)) { //(calib.triangulation_ms_error < m_auto_calib_result.triangulation_ms_error){
        m_auto_calib_result = calib;
        m_auto_calib_result_excluded = excluded;
    }


    if (is_new_calib_better(m_global_minimum, calib)) {
        m_global_minimum = calib;
    }
}

bool OpenCVCalibrator::is_new_calib_better(const StereoCalibResult &old_calib, const StereoCalibResult &new_calib)
{
    switch (m_bcs) {
    case bcsByOpenCVError:
        return old_calib.opencv_reprojection_error > new_calib.opencv_reprojection_error;
        //        break;


    case bcsByEpipolarError:
        return old_calib.epipolar_error > new_calib.epipolar_error;
        //        break;


    case bcsByTriangulationError:
        return old_calib.triangulation_error > new_calib.triangulation_error;
        //        break;


    case bcsByEpipolarAndTriangulationErrors:
        return (old_calib.epipolar_error > new_calib.epipolar_error) &&
                (old_calib.triangulation_error > new_calib.triangulation_error);
        //        break;


    case bcsByEuclideanDistanceError:
        return old_calib.euclidean_dist_error > new_calib.euclidean_dist_error;
        //        break;


    case bcsByEuclideanDistanceAndTriangulationErrors:
        return (old_calib.euclidean_dist_error > new_calib.euclidean_dist_error) &&
                (old_calib.triangulation_error > new_calib.triangulation_error);
        //        break;
    case bcsByEuclideanDistanceOrTriangulationErrors:
        return (old_calib.euclidean_dist_error > new_calib.euclidean_dist_error) ||
                (old_calib.triangulation_error > new_calib.triangulation_error);
    }

    return false;
}


void CalibTester::run()
{
    //    qDebug() << "TH[" << QThread::currentThread() << "] " << excluded;


    StereoCalibResult res = calibrate_stereo(img_pts, obj_pts,image_size);

    //    qDebug() << "TH[" << QThread::currentThread() << "] " << "calib ended, calc errors";


    if(img_pts_holdout == nullptr){
        //        qDebug() << "TH[" << QThread::currentThread() << "] " << "use train";

        res.triangulation_error = calc_triangulation_error_all(res, img_pts);

        res.epipolar_error = calc_epipolar_error_all(res, img_pts);

        res.euclidean_dist_error = calc_euclidean_dist_error_all(res, img_pts, obj_pts[0]);
    } else {
        //        qDebug() << "TH[" << QThread::currentThread() << "] " << "use test";

        res.triangulation_error = calc_triangulation_error_all(res, img_pts_holdout);

        res.epipolar_error = calc_epipolar_error_all(res, img_pts_holdout);

        res.euclidean_dist_error = calc_euclidean_dist_error_all(res, img_pts_holdout, obj_pts[0]);

    }



    emit result(res, excluded);

    //    qDebug() << "CalibTester RMS: " << rms;


}
