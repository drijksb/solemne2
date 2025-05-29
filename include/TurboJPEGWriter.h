#ifndef TURBOJPEGWRITER_H
#define TURBOJPEGWRITER_H

#include <opencv2/core.hpp>
#include <string>

/**
 * @brief Escribe una imagen JPEG usando libjpeg-turbo.
 *
 * @param image Imagen BGR (cv::Mat) de 8 bits por canal.
 * @param filename Ruta de salida del archivo JPEG.
 * @param quality Calidad JPEG entre 0 (muy baja) y 100 (máxima). Por defecto: 90.
 * @return true si la imagen se guardó correctamente, false en caso de error.
 */
bool writeJPEG_turbo(const cv::Mat& image, const std::string& filename, int quality = 90);

#endif // TURBOJPEGWRITER_H
