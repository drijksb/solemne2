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
 * @param data Objeto ImageData a insertar.
 * @return true si se encoló el dato, false si la cola está llena o finalizada.
 */
bool ThreadSafeQueue::push(const ImageData& data) {
    std::unique_lock<std::mutex> lock(mutex); // Protege el acceso concurrente a la cola

    // Si la cola está llena y no se ha finalizado, elimina el primer dato de la cola
    // para generar un espacio para este nuevo dato
    if (queue.size() >= max_size && !done) {
        queue.pop();
    }

    // Si la cola ya fue finalizada no encola el dato
    if (done) return false;

    // Encola el dato
    queue.push(data);

    // Notifica a un consumidor de que hay un nuevo elemento
    cv.notify_one();

    return true; // El dato fue encolado correctamente
}

/**
 * @brief Extrae un elemento de la cola.
 * @param result Referencia donde se almacenará el elemento extraído.
 * @return true si se extrajo un elemento, false si la cola está vacía y finalizada.
 */
bool ThreadSafeQueue::pop(ImageData& result) {
    std::unique_lock<std::mutex> lock(mutex); // Protege el acceso concurrente a la cola

    // Espera hasta que haya un elemento en la cola o la cola esté finalizada
    cv.wait(lock, [this] { return !queue.empty() || done; });

    // Si la cola está vacía y finalizada, no hay más datos por consumir
    if (queue.empty() && done) {
        return false;
    }

    // Extrae el elemento del frente de la cola
    result = queue.front();
    queue.pop();

    // Notifica a un posible productor que hay espacio disponible en la cola
    cv_full.notify_one();

    return true; // Se extrajo un elemento correctamente
}

/**
 * @brief Marca la cola como finalizada.
 * Despierta a todos los hilos en espera para que terminen.
 */
void ThreadSafeQueue::finish() {
    {
        std::unique_lock<std::mutex> lock(mutex); // Protege el acceso concurrente
        done = true; // Marca la cola como finalizada
    }
    cv.notify_all(); // Despierta a todos los consumidores que puedan estar esperando
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
    std::unique_lock<std::mutex> lock(mutex); // Protege el acceso concurrente
    return queue.size();
}
