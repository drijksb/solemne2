#ifndef UTILS_H
#define UTILS_H

#include <string>

/**
 * @brief Muestra el uso del programa
 * @param programName Nombre del programa
 */
void showUsage(const std::string& programName);

/**
 * @brief Crea un directorio si no existe
 * @param path Ruta del directorio a crear
 * @return true si el directorio existe o fue creado exitosamente
 */
bool createDirectoryIfNotExists(const std::string& path);

/**
 * @brief Formatea un tamaño en bytes a una representación legible
 * @param bytes Cantidad de bytes
 * @return String formateado (ej: "1.23 MB")
 */
std::string formatByteSize(size_t bytes);

#endif // UTILS_H