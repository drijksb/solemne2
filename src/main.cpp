/**
 * @file main.cpp
 * @brief Programa principal para generar y guardar imágenes en múltiples hilos.
 * 
 * Este programa genera imágenes aleatorias con OpenCV a una tasa de FPS objetivo,
 * las encola en una cola segura y varios hilos escritores las guardan como archivos JPEG.
 * 
 * Se pueden configurar parámetros como FPS, duración, cantidad de hilos escritores, dimensiones,
 * y directorio de salida mediante argumentos de línea de comandos.
 */

#include "ThreadSafeQueue.h"
#include "ImageGenerator.h"
#include "ImageWriter.h"
#include "Utils.h"

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <string>

/**
 * @brief Función principal
 * 
 * Procesa argumentos, configura el entorno, crea los hilos de generación y escritura,
 * y muestra estadísticas finales al terminar.
 * 
 * @param argc Número de argumentos
 * @param argv Vector de argumentos
 * @return int Código de salida
 */
int main(int argc, char** argv) {
    // Parámetros por defecto
    int targetFPS = 50;
    int runTime = 300; // 5 minutos en segundos
    int numWriterThreads = 4;
    std::string outputDir = "output";
    int imageWidth = 1920;
    int imageHeight = 1280;
    
    // Procesar argumentos de línea de comandos
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            showUsage(argv[0]);
            return 0;
        } else if (arg == "-fps" && i + 1 < argc) {
            targetFPS = std::stoi(argv[++i]);
            if (targetFPS <= 0) {
                std::cerr << "Error: FPS debe ser mayor que 0" << std::endl;
                return 1;
            }
        } else if (arg == "-time" && i + 1 < argc) {
            runTime = std::stoi(argv[++i]);
            if (runTime <= 0) {
                std::cerr << "Error: Tiempo de ejecución debe ser mayor que 0" << std::endl;
                return 1;
            }
        } else if (arg == "-writers" && i + 1 < argc) {
            numWriterThreads = std::stoi(argv[++i]);
            if (numWriterThreads <= 0 || numWriterThreads > 7) {
                std::cerr << "Error: Número de hilos escritores debe estar entre 1 y 7" << std::endl;
                return 1;
            }
        } else if (arg == "-dir" && i + 1 < argc) {
            outputDir = argv[++i];
        } else if (arg == "-width" && i + 1 < argc) {
            imageWidth = std::stoi(argv[++i]);
            if (imageWidth <= 0) {
                std::cerr << "Error: Ancho debe ser mayor que 0" << std::endl;
                return 1;
            }
        } else if (arg == "-height" && i + 1 < argc) {
            imageHeight = std::stoi(argv[++i]);
            if (imageHeight <= 0) {
                std::cerr << "Error: Alto debe ser mayor que 0" << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Argumento desconocido: " << arg << std::endl;
            showUsage(argv[0]);
            return 1;
        }
    }
    
    // Crear directorio de salida
    if (!createDirectoryIfNotExists(outputDir)) {
        return 1;
    }
    
    // Mostrar configuración
    std::cout << "=== Configuración ===" << std::endl;
    std::cout << "Dimensiones: " << imageWidth << "x" << imageHeight << " píxeles" << std::endl;
    std::cout << "FPS objetivo: " << targetFPS << std::endl;
    std::cout << "Tiempo de ejecución: " << runTime << " segundos" << std::endl;
    std::cout << "Hilos escritores: " << numWriterThreads << std::endl;
    std::cout << "Directorio de salida: " << outputDir << std::endl;
    std::cout << "===================" << std::endl;
    
    // Cola de imágenes compartida
    ThreadSafeQueue imageQueue;
    
    // Contadores para estadísticas
    std::atomic<size_t> statsImageCount{0};
    std::atomic<size_t> statsBytesWritten{0};
    std::atomic<size_t> imagesEnqueued{0};
    std::atomic<size_t> imagesSaved{0};

    
    // Vector de hilos
    std::vector<std::thread> threads;
    
    // Tiempo de ejecución
    auto runDuration = std::chrono::seconds(runTime);
    
    // Iniciar hilo generador
    threads.emplace_back(
        imageGeneratorThread, 
        std::ref(imageQueue), 
        imageWidth, 
        imageHeight, 
        targetFPS, 
        runDuration, 
        std::ref(statsImageCount),
        std::ref(imagesEnqueued)
    );
    
    // Iniciar hilos escritores
    for (int i = 0; i < numWriterThreads; i++) {
        threads.emplace_back(
            imageWriterThread, 
            std::ref(imageQueue), 
            outputDir, 
            std::ref(statsBytesWritten),
            std::ref(imagesSaved),
            i + 1
        );
    }
    
    // Esperar que termine el tiempo
    std::this_thread::sleep_for(runDuration);
    
    // Finalizar cola y esperar a que terminen los hilos
    std::cout << "Tiempo completado. Finalizando..." << std::endl;
    imageQueue.finish();
    
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    // Mostrar estadísticas finales
    const double elapsedSeconds = runTime;
    const size_t totalImages = statsImageCount.load();
    const size_t totalBytes = statsBytesWritten.load();
    
    std::cout << "\n=== Resultados Finales ===" << std::endl;
    std::cout << "Tiempo total: " << elapsedSeconds << " segundos" << std::endl;
    std::cout << "Imágenes generadas: " << totalImages << std::endl;
    std::cout << "Imágenes encoladas: " << imagesEnqueued.load() << std::endl;
    std::cout << "Imágenes guardadas (total): " << imagesSaved.load() << std::endl;
    std::cout << "Velocidad promedio: " << std::fixed << std::setprecision(2) 
              << (totalImages / elapsedSeconds) << " FPS" << std::endl;
    std::cout << "Datos grabados: " << formatByteSize(totalBytes) << std::endl;
    std::cout << "Velocidad de escritura: " << formatByteSize(static_cast<size_t>(totalBytes / elapsedSeconds)) << "/s" << std::endl;
    std::cout << "=========================" << std::endl;
    
    return 0;
}
