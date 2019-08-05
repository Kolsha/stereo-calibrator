#include "io_util.hpp"

#include <QPainter>

#include <iostream>
#include <fstream>
#include <float.h>

QImage io_util::qImage(const cv::Mat & image)
{
    switch (image.type())
    {
    case CV_8UC3: return io_util::qImageFromRGB(image); break;
    case CV_8UC1: return io_util::qImageFromGray(image); break;
    }
    return QImage();
}

QImage io_util::qImageFromRGB(const cv::Mat & image)
{
    if (image.type()!=CV_8UC3)
    {   //unsupported type
        return QImage();
    }

    QImage qimg(image.cols, image.rows, QImage::Format_RGB32);
    for (int h=0; h<image.rows; h++)
    {
        const cv::Vec3b * row = image.ptr<cv::Vec3b>(h);
        unsigned * qrow = reinterpret_cast<unsigned *>(qimg.scanLine(h));
        for (register int w=0; w<image.cols; w++)
        {
            const cv::Vec3b & vec = row[w];
            qrow[w] = qRgb(vec[2], vec[1], vec[0]);
        }
    }
    return qimg;
}

QImage io_util::qImageFromGray(const cv::Mat & image)
{
    if (image.type()!=CV_8UC1)
    {   //unsupported type
        return QImage();
    }

    QImage qimg(image.cols, image.rows, QImage::Format_RGB32);
    for (int h=0; h<image.rows; h++)
    {
        const unsigned char * row = image.ptr<unsigned char>(h);
        unsigned * qrow = reinterpret_cast<unsigned *>(qimg.scanLine(h));
        for (register int w=0; w<image.cols; w++)
        {
            qrow[w] = qRgb(row[w], row[w], row[w]);
        }
    }
    return qimg;
}
