#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "ImageData.h"

/**
 * @class ThreadSafeQueue
 * @brief Cola de imágenes segura para múltiples hilos (thread-safe).
 *
 * Esta clase permite que múltiples hilos productores y consumidores accedan
 * concurrentemente a una cola de objetos `ImageData` sin condiciones de carrera.
 * 
 * Utiliza `std::mutex` para garantizar la exclusión mutua, y `std::condition_variable`
 * para la sincronización entre productores y consumidores.
 */
class ThreadSafeQueue {
private:
    std::queue<ImageData> queue;                    ///< Cola interna de imágenes.
    std::mutex mutex;                               ///< Mutex para proteger el acceso concurrente a la cola.
    std::condition_variable cv;                     ///< Variable de condición para notificar cuando la cola no está vacía.
    std::condition_variable cv_full;                ///< Variable de condición para notificar cuando hay espacio disponible.
    std::atomic<bool> done{false};                  ///< Indica si se ha terminado de generar imágenes.
    size_t max_size;                                ///< Tamaño máximo permitido para la cola.

public:
    /**
     * @brief Constructor de la cola segura.
     * 
     * @param max_size Tamaño máximo permitido para la cola.
     */
    ThreadSafeQueue(size_t max_size = 100);

    /**
     * @brief Inserta un nuevo dato en la cola.
     * 
     * Bloquea si la cola está llena hasta que haya espacio disponible o se haya terminado.
     * 
     * @param data Imagen a insertar.
     */
    void push(const ImageData& data);

    /**
     * @brief Extrae un dato de la cola.
     * 
     * Bloquea si la cola está vacía hasta que haya datos o se haya terminado.
     * 
     * @param result Referencia donde se almacenará el dato extraído.
     * @return true si se extrajo un dato, false si la cola está terminada y vacía.
     */
    bool pop(ImageData& result);

    /**
     * @brief Marca la cola como terminada, notificando a todos los hilos bloqueados.
     */
    void finish();

    /**
     * @brief Indica si la cola está terminada.
     * 
     * @return true si se llamó a finish(), false en caso contrario.
     */
    bool isDone() const;

    /**
     * @brief Obtiene el tamaño actual de la cola.
     * 
     * @return Número de elementos en la cola.
     */
    size_t size();
};

#endif // THREADSAFEQUEUE_H
