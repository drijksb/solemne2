/**
 * @file TurboJPEGWriter.cpp
 * @brief Implementación para guardar imágenes JPEG usando TurboJPEG.
 */

#include "TurboJPEGWriter.h"
#include <turbojpeg.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

/**
 * @brief Comprime y guarda una imagen OpenCV en formato JPEG usando TurboJPEG.
 * 
 * La imagen debe estar en formato BGR de 8 bits y con 3 canales.
 * 
 * @param image Imagen OpenCV (cv::Mat) a comprimir y guardar.
 * @param filename Nombre (y ruta) del archivo de salida.
 * @param quality Calidad de compresión JPEG (0-100).
 * @return true si la imagen fue guardada correctamente, false en caso de error.
 * 
 * @note Explicación detallada de variables y parámetros:
 * 
 * - `unsigned char* jpegBuf = nullptr;`
 *   Puntero a un buffer donde TurboJPEG almacenará la imagen JPEG comprimida. 
 *   Inicialmente apunta a nullptr porque TurboJPEG asigna la memoria dinámicamente 
 *   dentro de `tjCompress2`. Es importante liberar esta memoria después con `tjFree`.
 * 
 * - `unsigned long jpegSize = 0;`
 *   Variable donde TurboJPEG guardará el tamaño en bytes del buffer JPEG generado.
 * 
 * - `image.cols`
 *   Miembro público de `cv::Mat` que indica el ancho de la imagen en píxeles (número de columnas).
 *   Este valor está almacenado dentro del objeto de imagen y no requiere cálculo adicional.
 * 
 * - `image.step1()`
 *   Retorna el stride, es decir, el número de bytes que ocupa una fila completa de la imagen en memoria.
 *   Esto es importante porque cada fila puede incluir padding o tener varios canales (bytes por píxel).
 *   Por ejemplo, una imagen 1920x1080 con 3 canales BGR típicamente tiene un stride de 1920 * 3 = 5760 bytes.
 * 
 * - `std::ofstream out(filename, std::ios::binary);`
 *   Abre un archivo para escritura en modo binario. El modo binario evita que se interpreten caracteres especiales,
 *   asegurando que los datos binarios (como JPEG) se escriban tal cual sin modificaciones.
 * 
 * - `out.write(reinterpret_cast<char*>(jpegBuf), jpegSize);`
 *   Escribe `jpegSize` bytes desde el buffer `jpegBuf` en el archivo. Se usa `reinterpret_cast<char*>` para 
 *   convertir el puntero de bytes sin signo (`unsigned char*`) a un puntero de caracteres (`char*`), 
 *   que es el tipo requerido por `write`.
 * 
 * Parámetros clave usados en `tjCompress2`:
 * - `compressor`: manejador TurboJPEG creado con `tjInitCompress`.
 * - `image.data`: puntero a los datos originales de la imagen (BGR raw).
 * - `image.cols`: ancho en píxeles.
 * - `image.step1()`: número de bytes por fila (stride).
 * - `image.rows`: alto en píxeles.
 * - `TJPF_BGR`: indica que el formato de pixel de la fuente es BGR (como OpenCV).
 * - `&jpegBuf`: dirección del puntero donde TurboJPEG almacenará el buffer JPEG generado.
 * - `&jpegSize`: variable donde TurboJPEG almacenará el tamaño del buffer JPEG.
 * - `TJSAMP_420`: tipo de submuestreo cromático (4:2:0), que reduce tamaño manteniendo calidad.
 * - `quality`: calidad de compresión JPEG, de 0 (peor) a 100 (mejor).
 * - `TJFLAG_FASTDCT`: usa una versión rápida del algoritmo DCT para acelerar la compresión.
 * 
 * Si `tjCompress2` retorna distinto de 0, significa que ocurrió un error en la compresión.
 */
bool writeJPEG_turbo(const cv::Mat& image, const std::string& filename, int quality) {
    if (image.empty() || image.channels() != 3 || image.depth() != CV_8U) {
        std::cerr << "Solo imágenes BGR de 8 bits son soportadas.\n";
        return false;
    }

    tjhandle compressor = tjInitCompress();
    if (!compressor) {
        std::cerr << "Error inicializando TurboJPEG\n";
        return false;
    }

    unsigned char* jpegBuf = nullptr;
    unsigned long jpegSize = 0;

    int success = tjCompress2(
        compressor,
        image.data,
        image.cols,
        image.step1(),
        image.rows,
        TJPF_BGR,
        &jpegBuf,
        &jpegSize,
        TJSAMP_420,
        quality,
        TJFLAG_FASTDCT
    );

    if (success != 0) {
        std::cerr << "Error al comprimir: " << tjGetErrorStr() << std::endl;
        tjDestroy(compressor);
        return false;
    }

    // Escribir a archivo
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<char*>(jpegBuf), jpegSize);
    out.close();

    tjFree(jpegBuf);
    tjDestroy(compressor);
    return true;
}
