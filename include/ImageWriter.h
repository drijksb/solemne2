#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

#include <string>
#include <atomic>
#include "ThreadSafeQueue.h"

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
    std::atomic<size_t>& imagesSaved,
    int threadId);

#endif // IMAGEWRITER_H