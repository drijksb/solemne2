#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <filesystem>
#include <vector>
#include <random>

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
 * @brief Valida que la extensión de imagen sea soportada por OpenCV
 * 
 * @param extension Extensión a validar (ej: "jpg", "png", etc.)
 * @return true si es válida, false en caso contrario
 */
bool isValidImageExtension(const std::string& extension) {
    std::vector<std::string> validExtensions = {
        "jpg", "jpeg", "png", "bmp", "tiff", "tif", "webp", "pbm", "pgm", "ppm", "pxm", "pnm"
    };
    
    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    
    return std::find(validExtensions.begin(), validExtensions.end(), lowerExt) != validExtensions.end();
}

/**
 * @brief Muestra la ayuda del programa
 */
void showHelp() {
    std::cout << "Uso: image_writer [opciones]\n"
              << "Opciones:\n"
              << "  -h, --help              Muestra esta ayuda\n"
              << "  -c, --count <número>    Número de imágenes a generar (default: 1000)\n"
              << "  -w, --width <píxeles>   Ancho de las imágenes (default: 1920)\n"
              << "  -h, --height <píxeles>  Alto de las imágenes (default: 1080)\n"
              << "  -e, --ext <extensión>   Extensión de archivo (default: jpg)\n"
              << "  -o, --output <directorio> Directorio de salida (default: ./output)\n"
              << "  -p, --prefix <prefijo>  Prefijo para nombres de archivo (default: img)\n"
              << "\nExtensiones soportadas: jpg, jpeg, png, bmp, tiff, tif, webp, pbm, pgm, ppm, pxm, pnm\n"
              << "\nEjemplos:\n"
              << "  image_writer -c 500 -e png\n"
              << "  image_writer --count 2000 --width 1280 --height 720 --ext jpg\n";
}

/**
 * @brief Estructura para almacenar la configuración del programa
 */
struct Config {
    int imageCount = 1000;
    int width = 1920;
    int height = 1080;
    std::string extension = "jpg";
    std::string outputDir = "./tests_output";
    std::string prefix = "img";
};

/**
 * @brief Parsea los argumentos de línea de comandos
 * 
 * @param argc Número de argumentos
 * @param argv Array de argumentos
 * @param config Configuración a llenar
 * @return true si los argumentos son válidos, false en caso contrario
 */
bool parseArguments(int argc, char* argv[], Config& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            showHelp();
            return false;
        }
        else if ((arg == "-c" || arg == "--count") && i + 1 < argc) {
            config.imageCount = std::atoi(argv[++i]);
            if (config.imageCount <= 0) {
                std::cerr << "Error: El número de imágenes debe ser mayor a 0\n";
                return false;
            }
        }
        else if ((arg == "-w" || arg == "--width") && i + 1 < argc) {
            config.width = std::atoi(argv[++i]);
            if (config.width <= 0) {
                std::cerr << "Error: El ancho debe ser mayor a 0\n";
                return false;
            }
        }
        else if ((arg == "--height") && i + 1 < argc) {
            config.height = std::atoi(argv[++i]);
            if (config.height <= 0) {
                std::cerr << "Error: El alto debe ser mayor a 0\n";
                return false;
            }
        }
        else if ((arg == "-e" || arg == "--ext") && i + 1 < argc) {
            config.extension = argv[++i];
            if (!isValidImageExtension(config.extension)) {
                std::cerr << "Error: Extensión '" << config.extension << "' no soportada\n";
                return false;
            }
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            config.outputDir = argv[++i];
        }
        else if ((arg == "-p" || arg == "--prefix") && i + 1 < argc) {
            config.prefix = argv[++i];
        }
        else {
            std::cerr << "Error: Argumento desconocido '" << arg << "'\n";
            showHelp();
            return false;
        }
    }
    return true;
}

/**
 * @brief Crea el directorio de salida si no existe
 * 
 * @param outputDir Directorio a crear
 * @return true si se creó exitosamente o ya existía, false en caso contrario
 */
bool createOutputDirectory(const std::string& outputDir) {
    try {
        if (!std::filesystem::exists(outputDir)) {
            std::filesystem::create_directories(outputDir);
            std::cout << "Directorio creado: " << outputDir << std::endl;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creando directorio: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Función principal que genera y escribe las imágenes
 */
int main(int argc, char* argv[]) {
    Config config;
    
    // Parsear argumentos
    if (!parseArguments(argc, argv, config)) {
        return 1;
    }
    
    // Crear directorio de salida
    if (!createOutputDirectory(config.outputDir)) {
        return 1;
    }
    
    // Mostrar configuración
    std::cout << "=== Configuración ===" << std::endl;
    std::cout << "Imágenes a generar: " << config.imageCount << std::endl;
    std::cout << "Resolución: " << config.width << "x" << config.height << std::endl;
    std::cout << "Extensión: " << config.extension << std::endl;
    std::cout << "Directorio: " << config.outputDir << std::endl;
    std::cout << "Prefijo: " << config.prefix << std::endl;
    std::cout << "===================" << std::endl << std::endl;
    
    // Variables para estadísticas
    std::vector<double> writeTimes;
    writeTimes.reserve(config.imageCount);
    
    auto totalStartTime = std::chrono::high_resolution_clock::now();
    
    // Generar y escribir imágenes
    for (int i = 0; i < config.imageCount; i++) {
        // Generar imagen
        cv::Mat image = generateRandomImage(config.width, config.height);
        
        // Crear nombre de archivo
        std::string filename = config.outputDir + "/" + config.prefix + "_" + 
                              std::to_string(i + 1) + "." + config.extension;
        
        // Medir tiempo de escritura
        auto writeStartTime = std::chrono::high_resolution_clock::now();
        
        bool success = cv::imwrite(filename, image);
        
        auto writeEndTime = std::chrono::high_resolution_clock::now();
        
        if (!success) {
            std::cerr << "Error escribiendo imagen: " << filename << std::endl;
            continue;
        }
        
        // Calcular tiempo de escritura en milisegundos
        auto writeTime = std::chrono::duration<double, std::milli>(writeEndTime - writeStartTime).count();
        writeTimes.push_back(writeTime);
        
        // Mostrar progreso cada 100 imágenes o en múltiplos de 10% del total
        int progressInterval = std::max(100, config.imageCount / 10);
        if ((i + 1) % progressInterval == 0 || i == config.imageCount - 1) {
            double currentAvg = 0.0;
            for (double time : writeTimes) {
                currentAvg += time;
            }
            currentAvg /= writeTimes.size();
            
            std::cout << "Progreso: " << (i + 1) << "/" << config.imageCount 
                      << " (" << std::fixed << std::setprecision(1) 
                      << (100.0 * (i + 1) / config.imageCount) << "%) - "
                      << "Tiempo promedio actual: " << std::setprecision(3) 
                      << currentAvg << " ms" << std::endl;
        }
    }
    
    auto totalEndTime = std::chrono::high_resolution_clock::now();
    
    // Calcular estadísticas
    if (writeTimes.empty()) {
        std::cerr << "No se pudo escribir ninguna imagen." << std::endl;
        return 1;
    }
    
    double totalTime = std::chrono::duration<double>(totalEndTime - totalStartTime).count();
    double avgWriteTime = 0.0;
    double minWriteTime = writeTimes[0];
    double maxWriteTime = writeTimes[0];
    
    for (double time : writeTimes) {
        avgWriteTime += time;
        minWriteTime = std::min(minWriteTime, time);
        maxWriteTime = std::max(maxWriteTime, time);
    }
    avgWriteTime /= writeTimes.size();
    
    // Mostrar resultados
    std::cout << "\n=== RESULTADOS ===" << std::endl;
    std::cout << "Imágenes escritas exitosamente: " << writeTimes.size() << std::endl;
    std::cout << "Tiempo total: " << std::fixed << std::setprecision(2) << totalTime << " segundos" << std::endl;
    std::cout << "Tiempo promedio por imagen: " << std::setprecision(3) << avgWriteTime << " ms" << std::endl;
    std::cout << "Tiempo mínimo: " << std::setprecision(3) << minWriteTime << " ms" << std::endl;
    std::cout << "Tiempo máximo: " << std::setprecision(3) << maxWriteTime << " ms" << std::endl;
    std::cout << "Throughput: " << std::setprecision(2) << (writeTimes.size() / totalTime) << " imágenes/segundo" << std::endl;
    
    // Calcular tamaño aproximado por imagen
    std::filesystem::path firstImage = config.outputDir + "/" + config.prefix + "_1." + config.extension;
    if (std::filesystem::exists(firstImage)) {
        auto fileSize = std::filesystem::file_size(firstImage);
        double totalSizeMB = (fileSize * writeTimes.size()) / (1024.0 * 1024.0);
        std::cout << "Tamaño aproximado por imagen: " << std::setprecision(2) << (fileSize / 1024.0) << " KB" << std::endl;
        std::cout << "Tamaño total aproximado: " << std::setprecision(2) << totalSizeMB << " MB" << std::endl;
    }
    
    std::cout << "==================" << std::endl;
    
    return 0;
}