/**
 * @file ImageGenerator.cpp
 * @brief Genera imágenes aleatorias y las encola en un hilo separado manteniendo un FPS objetivo.
 */

#include "ImageGenerator.h"
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <iomanip>
#include <thread>

/**
 * @brief Genera una imagen aleatoria con valores de píxel entre 0 y 255.
 * 
 * @param width Ancho de la imagen.
 * @param height Alto de la imagen.
 * @return cv::Mat Imagen aleatoria de tipo CV_8UC3.
 */
cv::Mat generateRandomImage(int width, int height) {
    cv::Mat randomImage(height, width, CV_8UC3);
    cv::randu(randomImage, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
    return randomImage;
}

/**
 * @brief Función para el hilo que genera imágenes aleatorias a una tasa constante.
 * 
 * Genera imágenes de tamaño `width` x `height` y las encola en `queue` para ser procesadas
 * en otro hilo. Intenta mantener un framerate de `targetFPS` durante `runDuration`.
 * 
 * @param queue Cola segura para compartir las imágenes generadas.
 * @param width Ancho de cada imagen generada.
 * @param height Alto de cada imagen generada.
 * @param targetFPS Tasa de frames por segundo deseada.
 * @param runDuration Duración total para la generación de imágenes.
 * @param statsImageCount Referencia atómica que contabiliza el total de imágenes generadas.
 */
void imageGeneratorThread(
    ThreadSafeQueue& queue, 
    int width, 
    int height, 
    int targetFPS,
    std::chrono::seconds runDuration,
    std::atomic<size_t>& statsImageCount) {
    
    const auto startTime = std::chrono::steady_clock::now();
    const auto endTime = startTime + runDuration;
    size_t frameCount = 0;
    
    // Duración de cada frame en microsegundos
    const auto frameDuration = std::chrono::microseconds(1000000 / targetFPS);
    
    std::cout << "Iniciando generador de imágenes a " << targetFPS << " FPS durante " 
              << runDuration.count() << " segundos." << std::endl;
    
    while (std::chrono::steady_clock::now() < endTime) {
        auto frameStartTime = std::chrono::steady_clock::now();
        
        // Generar imagen
        cv::Mat img = generateRandomImage(width, height);
        
        // Encolar imagen para ser grabada
        queue.push(ImageData(img, frameCount));
        
        // Actualizar estadísticas
        frameCount++;
        statsImageCount++;
        
        // Regular FPS
        auto frameEndTime = std::chrono::steady_clock::now();
        auto processingTime = frameEndTime - frameStartTime;
        
        if (processingTime < frameDuration) {
            std::this_thread::sleep_for(frameDuration - processingTime);
        }
        
        // Mostrar estadísticas cada segundo
        if (frameCount % targetFPS == 0) {
            auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - startTime).count();
            
            if (elapsedSeconds > 0) {
                float currentFPS = static_cast<float>(frameCount) / elapsedSeconds;
                std::cout << "Generando: " << std::fixed << std::setprecision(2) 
                          << currentFPS << " FPS (Cola: " << queue.size() << ")" << std::endl;
            }
        }
    }
    
    std::cout << "Generador de imágenes finalizado. Total: " << frameCount << " imágenes" << std::endl;
}
