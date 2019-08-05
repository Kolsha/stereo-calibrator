#ifndef __IO_UTIL_HPP__
#define __IO_UTIL_HPP__

#include <QImage>
#include <opencv2/core/core.hpp>


namespace io_util
{

    QImage qImage(const cv::Mat & image);
    QImage qImageFromRGB(const cv::Mat & image);
    QImage qImageFromGray(const cv::Mat & image);

    bool write_pgm(const cv::Mat & image, const char * basename);
}

#endif  /* __IO_UTIL_HPP__ */
