/**
 * @file ImageWriter.cpp
 * @brief Hilo encargado de escribir imágenes desde una cola en archivos JPEG usando TurboJPEG.
 */

#include "ImageWriter.h"
#include "TurboJPEGWriter.h"
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <vector>

/**
 * @brief Función para el hilo que escribe imágenes JPEG desde una cola segura.
 * 
 * Este hilo extrae imágenes de una cola segura (`queue`), las guarda en disco en el directorio
 * `outputDir` con un nombre basado en el número de secuencia y el ID del hilo. Usa TurboJPEG
 * para la compresión con calidad fija. Actualiza estadísticas atómicas del total de bytes escritos.
 * 
 * @param queue Cola segura de imágenes a escribir.
 * @param outputDir Directorio donde se guardarán las imágenes JPEG.
 * @param statsBytesWritten Contador atómico para el total de bytes escritos.
 * @param threadId Identificador del hilo para diferenciar archivos y logs.
 */
void imageWriterThread(
    ThreadSafeQueue& queue, 
    const std::string& outputDir,
    std::atomic<size_t>& statsBytesWritten,
    std::atomic<size_t>& imagesSaved,
    int threadId) {
    
    size_t imagesWritten = 0;
    ImageData data(cv::Mat(), 0);
    
    std::cout << "Iniciando hilo escritor #" << threadId << std::endl;
    
    while (queue.pop(data)) {
        // Crear nombre de archivo
        std::ostringstream filename;
        filename << outputDir << "/img_" << std::setw(8) << std::setfill('0') 
                << data.sequenceNumber << "_t" << threadId << ".jpg";

        // Escribir imagen JPG
        bool success = writeJPEG_turbo(data.image, filename.str(), 70);
        
        if (success) {
            // Actualizar estadísticas
            imagesWritten++;
            imagesSaved++;

            
            // Calcular tamaño del archivo y actualizar estadísticas
            std::filesystem::path filePath(filename.str());
            size_t fileSize = std::filesystem::file_size(filePath);
            statsBytesWritten += fileSize;
            
            // Mostrar progreso periódicamente
            if (imagesWritten % 100 == 0) {
                std::cout << "Hilo #" << threadId << " ha escrito " << imagesWritten << " imágenes" << std::endl;
            }
        } else {
            std::cerr << "Error al escribir imagen: " << filename.str() << std::endl;
        }
    }
    
    std::cout << "Hilo escritor #" << threadId << " finalizado. Total: " << imagesWritten << " imágenes" << std::endl;
}
