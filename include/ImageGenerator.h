#ifndef IMAGEGENERATOR_H
#define IMAGEGENERATOR_H

#include <opencv2/core.hpp>
#include <chrono>
#include <atomic>
#include "ThreadSafeQueue.h"

/**
 * @brief Genera una imagen con ruido con las dimensiones especificadas.
 * @param width El ancho deseado de la imagen en píxeles.
 * @param height La altura deseada de la imagen en píxeles.
 * @return cv::Mat Una matriz OpenCV que representa la imagen aleatoria generada.
 */
cv::Mat generateRandomImage(int width, int height);

/**
 * @brief Hilo generador de imágenes
 * @param queue Cola donde se colocarán las imágenes generadas
 * @param width Ancho de las imágenes a generar
 * @param height Alto de las imágenes a generar
 * @param targetFPS Velocidad objetivo de generación (fotogramas por segundo)
 * @param runDuration Duración total de la ejecución en segundos
 * @param statsImageCount Contador atómico de imágenes generadas
 */
void imageGeneratorThread(
    ThreadSafeQueue& queue, 
    int width, 
    int height, 
    int targetFPS,
    std::chrono::seconds runDuration,
    std::atomic<size_t>& statsImageCount,
    std::atomic<size_t>& imagesEnqueued);

#endif // IMAGEGENERATOR_H