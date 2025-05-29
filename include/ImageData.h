#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <opencv2/core.hpp>

/**
 * @brief Estructura para almacenar una imagen y su número de secuencia
 */
struct ImageData {
    cv::Mat image;
    size_t sequenceNumber;
    
    ImageData(cv::Mat img, size_t seq) : image(img), sequenceNumber(seq) {}
};

#endif // IMAGEDATA_H