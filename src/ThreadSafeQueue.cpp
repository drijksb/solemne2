/**
 * @file ThreadSafeQueue.cpp
 * @brief Implementación de una cola segura para hilos que almacena objetos ImageData.
 */

#include "ThreadSafeQueue.h"

/**
 * @brief Constructor de la cola segura.
 * @param max_size Tamaño máximo de la cola.
 */
ThreadSafeQueue::ThreadSafeQueue(size_t max_size) : max_size(max_size) {}

/**
 * @brief Inserta un elemento en la cola.
 * 
 * Espera si la cola está llena hasta que haya espacio o se indique finalización.
 * 
 * @param data Objeto ImageData a insertar.
 */
void ThreadSafeQueue::push(const ImageData& data) {
    std::unique_lock<std::mutex> lock(mutex);
    cv_full.wait(lock, [this] { return queue.size() < max_size || done; });
    if (done) return;
    queue.push(data);
    cv.notify_one();
}

/**
 * @brief Extrae un elemento de la cola.
 * 
 * Espera si la cola está vacía hasta que haya un elemento o se indique finalización.
 * 
 * @param result Referencia donde se almacenará el elemento extraído.
 * @return true si se extrajo un elemento, false si la cola está vacía y se terminó.
 */
bool ThreadSafeQueue::pop(ImageData& result) {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return !queue.empty() || done; });
    if (queue.empty() && done) {
        return false;
    }
    result = queue.front();
    queue.pop();
    cv_full.notify_one();
    return true;
}

/**
 * @brief Marca la cola como finalizada.
 * 
 * Despierta a todos los hilos en espera para que terminen.
 */
void ThreadSafeQueue::finish() {
    {
        std::unique_lock<std::mutex> lock(mutex);
        done = true;
    }
    cv.notify_all();
}

/**
 * @brief Consulta si la cola está marcada como finalizada.
 * @return true si se ha llamado a finish(), false en caso contrario.
 */
bool ThreadSafeQueue::isDone() const {
    return done;
}

/**
 * @brief Obtiene el tamaño actual de la cola.
 * @return Número de elementos en la cola.
 */
size_t ThreadSafeQueue::size() {
    std::unique_lock<std::mutex> lock(mutex);
    return queue.size();
}
