# Random Image Generator

Un generador de imágenes aleatorias multihilo de alto rendimiento que utiliza OpenCV para la generación y TurboJPEG para la compresión y escritura optimizada de archivos JPEG.

## Descripción

Este programa genera imágenes aleatorias con ruido de colores a una tasa de FPS configurable, utilizando un sistema de colas thread-safe donde un hilo generador produce las imágenes y múltiples hilos escritores las procesan y guardan en disco de forma paralela. Está diseñado para evaluar el rendimiento de escritura de imágenes en sistemas de almacenamiento.

## Características

- **Generación multihilo**: Un hilo generador y múltiples hilos escritores (hasta 7)
- **Cola thread-safe**: Sincronización segura entre productores y consumidores
- **Compresión optimizada**: Utiliza TurboJPEG para máxima velocidad de compresión
- **Control de FPS**: Mantiene una tasa constante de generación de fotogramas
- **Estadísticas en tiempo real**: Monitoreo de rendimiento y throughput
- **Configuración flexible**: Múltiples parámetros ajustables por línea de comandos

## Dependencias

### Requerimientos del sistema

- **CMake** >= 3.10
- **Compilador C++** con soporte para C++17 o superior (GCC, Clang)
- **OpenCV** >= 3.0 (preferiblemente 4.x)
- **TurboJPEG** (libjpeg-turbo)

### Instalación de dependencias

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install cmake build-essential
sudo apt install libopencv-dev
sudo apt install libturbojpeg0-dev
```

#### CentOS/RHEL/Fedora
```bash
# CentOS/RHEL
sudo yum install cmake gcc-c++
sudo yum install opencv-devel
sudo yum install turbojpeg-devel

# Fedora
sudo dnf install cmake gcc-c++
sudo dnf install opencv-devel
sudo dnf install turbojpeg-devel
```

#### macOS
```bash
# Usando Homebrew
brew install cmake opencv jpeg-turbo
```

#### Arch Linux
```bash
sudo pacman -S cmake gcc opencv libjpeg-turbo
```

## Compilación

1. **Clonar o descargar el proyecto**
```bash
# Si tienes el código fuente
cd RandomImageGenerator
```

2. **Crear directorio de compilación**
```bash
mkdir build
cd build
```

3. **Configurar con CMake**
```bash
cmake ..
```

4. **Compilar**
```bash
make
```

Si la compilación es exitosa, se generará el ejecutable `random_image_generator` en el directorio `build`.

## Uso

### Ejecución básica
```bash
./random_image_generator
```

### Parámetros disponibles
```bash
./random_image_generator [opciones]
```

| Parámetro | Descripción | Valor por defecto |
|-----------|-------------|-------------------|
| `-fps N` | Velocidad de generación (fotogramas por segundo) | 50 |
| `-time N` | Tiempo de ejecución en segundos | 300 (5 minutos) |
| `-writers N` | Número de hilos escritores (máximo: 7) | 4 |
| `-dir PATH` | Directorio de salida para las imágenes | `output` |
| `-width N` | Ancho de las imágenes en píxeles | 1920 |
| `-height N` | Alto de las imágenes en píxeles | 1280 |
| `-h` | Muestra la ayuda | - |

### Ejemplos de uso

**Generar a 30 FPS durante 2 minutos con 6 hilos escritores:**
```bash
./random_image_generator -fps 30 -time 120 -writers 6
```

**Generar imágenes 4K en directorio personalizado:**
```bash
./random_image_generator -width 3840 -height 2160 -dir /tmp/imagenes
```

**Prueba rápida de 30 segundos:**
```bash
./random_image_generator -time 30 -fps 60
```

## Estructura del proyecto

```
RandomImageGenerator/
├── README.md
├── CMakeLists.txt
├── include/
│   ├── ImageData.h
│   ├── ImageGenerator.h
│   ├── ImageWriter.h
│   ├── ThreadSafeQueue.h
│   ├── TurboJPEGWriter.h
│   └── Utils.h
├── src/
│   ├── main.cpp
│   ├── ImageGenerator.cpp
│   ├── ImageWriter.cpp
│   ├── ThreadSafeQueue.cpp
│   ├── TurboJPEGWriter.cpp
│   └── Utils.cpp
└── build/           (creado durante la compilación)
```

## Salida

El programa genera:

1. **Archivos JPEG**: Las imágenes se guardan con el formato `img_XXXXXXXX_tN.jpg` donde:
   - `XXXXXXXX`: Número de secuencia con padding de ceros
   - `N`: ID del hilo escritor que procesó la imagen

2. **Estadísticas en consola**: Información en tiempo real sobre:
   - FPS actual de generación
   - Tamaño de la cola de procesamiento
   - Progreso de cada hilo escritor
   - Estadísticas finales (imágenes totales, velocidad promedio, datos escritos)

## Especificaciones técnicas

- **Formato de imagen**: BGR de 8 bits por canal (OpenCV estándar)
- **Compresión JPEG**: Usando TurboJPEG
- **Submuestreo cromático**: 4:2:0 para balance entre calidad y tamaño
- **Sincronización**: Mutex y variables de condición para thread-safety
- **Control de flujo**: Cola con tamaño máximo de 100 elementos para evitar consumo excesivo de memoria

## Resolución de problemas

### Error: "OpenCV not found"
- Verificar que OpenCV esté instalado correctamente
- En algunos sistemas, puede ser necesario especificar: `cmake -DOpenCV_DIR=/path/to/opencv ..`

### Error: "TurboJPEG not found"
- Instalar el paquete de desarrollo: `sudo apt install libturbojpeg0-dev`
- Verificar que los headers estén en `/usr/include/turbojpeg.h`

### Error: "Permission denied" al crear directorio
- Verificar permisos de escritura en el directorio de destino
- Usar un directorio alternativo: `./random_image_generator -dir /tmp/output`

### Rendimiento bajo
- Reducir el número de hilos escritores si el disco es lento
- Ajustar los FPS según la capacidad del sistema
- Verificar que no se esté llenando el disco de destino

## Autores

- [Karina](https://github.com/ninaaaa3)
- [Demis](https://github.com/drijksb)

