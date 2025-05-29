/**
 * @file Utils.cpp
 * @brief Funciones utilitarias para mostrar ayuda, crear directorios y formatear tamaños.
 */

#include "Utils.h"
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>

/**
 * @brief Muestra la ayuda y uso del programa con sus opciones.
 * 
 * @param programName Nombre del programa (usualmente argv[0]).
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
 * @brief Crea un directorio si no existe.
 * 
 * Si la ruta existe pero no es un directorio, retorna error.
 * 
 * @param path Ruta del directorio a crear.
 * @return true si el directorio existe o fue creado exitosamente, false en caso contrario.
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
 * @brief Formatea un tamaño en bytes a una cadena legible (B, KB, MB, GB, TB).
 * 
 * @param bytes Tamaño en bytes.
 * @return std::string Cadena con tamaño formateado.
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
