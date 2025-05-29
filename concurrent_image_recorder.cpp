#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <vector>
#include <algorithm>

/**
 * @brief Estructura para almacenar una imagen y su número de secuencia
 */
struct ImageData {
    cv::Mat image;
    size_t sequenceNumber;
    
    ImageData(cv::Mat img, size_t seq) : image(img), sequenceNumber(seq) {}
};

/**
 * @brief Cola de imágenes segura para hilos (thread-safe)
 */
class ThreadSafeQueue {
private:
    std::queue<ImageData> queue;
    std::mutex mutex;
    std::condition_variable cv;
    std::condition_variable cv_full; // <-- nueva
    std::atomic<bool> done{false};
    size_t max_size; // <-- nueva
    
public:
    ThreadSafeQueue(size_t max_size = 100) : max_size(max_size) {}
    
    void push(const ImageData& data) {
        std::unique_lock<std::mutex> lock(mutex);
        cv_full.wait(lock, [this] { return queue.size() < max_size || done; }); // <-- espera si está llena
        if (done) return; // si está terminado, no agregar más
        queue.push(data);
        cv.notify_one();
    }
    
    bool pop(ImageData& result) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !queue.empty() || done; });
        if (queue.empty() && done) {
            return false;
        }
        result = queue.front();
        queue.pop();
        cv_full.notify_one(); // <-- avisa que hay espacio disponible
        return true;
    }
    
    /**
     * @brief Finaliza la cola y despierta a todos los hilos que esperan
     */
    void finish() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            done = true;
        }
        cv.notify_all();
    }
    
    /**
     * @brief Verifica si la cola está terminada
     * @return true si la cola está marcada como terminada
     */
    bool isDone() const {
        return done;
    }
    
    /**
     * @brief Obtiene el tamaño actual de la cola
     * @return Número de elementos en la cola
     */
    size_t size() {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.size();
    }
};

/**
 * @brief Genera una imagen con ruido con las dimensiones especificadas.
 * @param width El ancho deseado de la imagen en píxeles.
 * @param height La altura deseada de la imagen en píxeles.
 * @return cv::Mat Una matriz OpenCV que representa la imagen aleatoria generada.
 */
cv::Mat generateRandomImage(int width, int height) {
    cv::Mat randomImage(height, width, CV_8UC3);
    cv::randu(randomImage, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
    return randomImage;
}

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

/**
 * @brief Hilo escritor de imágenes
 * @param queue Cola de donde se obtendrán las imágenes a escribir
 * @param outputDir Directorio donde se escribirán las imágenes
 * @param statsBytesWritten Contador atómico de bytes escritos
 * @param threadId Identificador del hilo escritor
 */
void imageWriterThread(
    ThreadSafeQueue& queue, 
    const std::string& outputDir,
    std::atomic<size_t>& statsBytesWritten,
    int threadId) {
    
    size_t imagesWritten = 0;
    ImageData data(cv::Mat(), 0);
    
    std::cout << "Iniciando hilo escritor #" << threadId << std::endl;
    
    while (queue.pop(data)) {
        // Crear nombre de archivo
        std::ostringstream filename;
        filename << outputDir << "/img_" << std::setw(8) << std::setfill('0') 
                << data.sequenceNumber << "_t" << threadId << ".jpg"; // <-- CAMBIA la extensión

        // Escribir imagen JPG
        std::vector<int> compressionParams;
        compressionParams.push_back(cv::IMWRITE_JPEG_QUALITY);
        compressionParams.push_back(90); // Calidad 0-100 (más alto = mejor calidad/archivo más grande)
        bool success = cv::imwrite(filename.str(), data.image, compressionParams);
        
        if (success) {
            // Actualizar estadísticas
            imagesWritten++;
            
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

/**
 * @brief Muestra el uso del programa
 * @param programName Nombre del programa
 */
void showUsage(const std::string& programName) {
    std::cout << "Uso: " << programName << " [opciones]" << std::endl;
    std::cout << "Opciones:" << std::endl;
    std::cout << "  -fps N      Velocidad de generación en fotogramas por segundo (por defecto: 50)" << std::endl;
    std::cout << "  -time N     Tiempo de ejecución en segundos (por defecto: 300 = 5 minutos)" << std::endl;
    std::cout << "  -writers N  Número de hilos escritores (por defecto: 4, máximo: 7)" << std::endl;
    std::cout << "  -dir PATH   Directorio de salida para las imágenes (por defecto: 'output')" << std::endl;
    std::cout << "  -width N    Ancho de las imágenes en píxeles (por defecto: 1920)" << std::endl;
    std::cout << "  -height N   Alto de las imágenes en píxeles (por defecto: 1280)" << std::endl;
    std::cout << "  -h          Muestra esta ayuda" << std::endl;
}

/**
 * @brief Crea un directorio si no existe
 * @param path Ruta del directorio a crear
 * @return true si el directorio existe o fue creado exitosamente
 */
bool createDirectoryIfNotExists(const std::string& path) {
    std::filesystem::path dir(path);
    if (std::filesystem::exists(dir)) {
        if (!std::filesystem::is_directory(dir)) {
            std::cerr << "Error: " << path << " existe pero no es un directorio." << std::endl;
            return false;
        }
    } else {
        try {
            if (!std::filesystem::create_directories(dir)) {
                std::cerr << "Error: No se pudo crear el directorio " << path << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error al crear directorio: " << e.what() << std::endl;
            return false;
        }
    }
    return true;
}

/**
 * @brief Formatea un tamaño en bytes a una representación legible
 * @param bytes Cantidad de bytes
 * @return String formateado (ej: "1.23 MB")
 */
std::string formatByteSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return oss.str();
}

/**
 * @brief Función principal
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
    
    // Vector de hilos
    std::vector<std::thread> threads;
    
    // Tiempo de ejecución
    auto runDuration = std::chrono::seconds(runTime);
    
    // Iniciar hilos
    threads.emplace_back(
        imageGeneratorThread, 
        std::ref(imageQueue), 
        imageWidth, 
        imageHeight, 
        targetFPS, 
        runDuration, 
        std::ref(statsImageCount)
    );
    
    // Iniciar hilos escritores
    for (int i = 0; i < numWriterThreads; i++) {
        threads.emplace_back(
            imageWriterThread, 
            std::ref(imageQueue), 
            outputDir, 
            std::ref(statsBytesWritten), 
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
    std::cout << "Velocidad promedio: " << std::fixed << std::setprecision(2) 
              << (totalImages / elapsedSeconds) << " FPS" << std::endl;
    std::cout << "Datos grabados: " << formatByteSize(totalBytes) << std::endl;
    std::cout << "Velocidad de escritura: " << formatByteSize(static_cast<size_t>(totalBytes / elapsedSeconds)) << "/s" << std::endl;
    std::cout << "=========================" << std::endl;
    
    return 0;
}