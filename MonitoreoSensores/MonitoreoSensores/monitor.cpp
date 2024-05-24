/**
 * @file monitor.cpp
 * @autores Juan Pablo Hernández Ceballos
 * Implementa el monitoreo de los sensores y la escritura de los datos en los archivos correspondientes.
 * 
 * @detalles
 * Este archivo contiene las siguientes funciones y módulos:
 * - getCurrentTime: Obtiene la hora actual en formato HH:MM:SS.
 * - is_float: Verifica si una cadena representa un número flotante.
 * - is_integer: Verifica si una cadena representa un número entero.
 * - reco_hilo: Función del hilo recolector de datos de sensores.
 * - pH_hilo: Función del hilo que maneja los datos de pH.
 * - temperatura_hilo: Función del hilo que maneja los datos de temperatura.
 * - main: Función principal que inicia el programa y gestiona la creación y sincronización de hilos.
 * 
 * @fecha 23/05/2024
 */
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include "buffer.cpp"

/**
 * Estructura para almacenar los argumentos que se pasarán a los hilos.
 * 
 * @param pH_buffer Puntero al buffer que almacena los datos de pH.
 * @param temp_buffer Puntero al buffer que almacena los datos de temperatura.
 * @param pipeName Nombre del pipe que se utilizará para la comunicación.
 * @param semaphore Semáforo para la sincronización entre hilos.
 */
struct ThreadArgs {
    Buffer* pH_buffer;    ///< Buffer para los datos de pH
    Buffer* temp_buffer;  ///< Buffer para los datos de temperatura
    char* pipeName;       ///< Nombre del pipe para la comunicación entre procesos
    sem_t semaphore;      ///< Semáforo para la sincronización entre hilos
};


/**
 * Obtiene la hora actual en formato HH:MM:SS.
 * 
 * @return Una cadena que representa la hora actual.
 */
std::string getCurrentTime() {
    std::time_t currentTime = std::time(nullptr); // Obtener la hora actual en segundos desde el epoch
    std::tm* localTime = std::localtime(&currentTime); // Convertir la hora actual en una estructura de tiempo local
    char timeString[100]; // Buffer para almacenar la hora formateada como cadena
    std::strftime(timeString, sizeof(timeString), "%H:%M:%S", localTime); // Formatear la hora en formato HH:MM:SS
    return std::string(timeString); // Devolver la hora formateada como una cadena
}

/**
 * Verifica si una cadena representa un número flotante.
 * 
 * @param str La cadena que se va a verificar.
 * @return true si la cadena representa un número flotante, false en caso contrario.
 */
bool is_float(const std::string& str) {
    try {
        std::size_t pos; // Variable para almacenar la posición del primer carácter no convertido
        std::stof(str, &pos); // Convertir la cadena a flotante
        // Tiene éxito si no hay caracteres restantes en la cadena
        return pos == str.size(); // Devolver true si no hay caracteres restantes, indicando que la conversión fue exitosa
    } catch (...) {
        return false; // Capturar cualquier excepción y devolver false si ocurre un error durante la conversión
    }
}

/**
 * Verifica si una cadena representa un número entero.
 * 
 * @param str La cadena que se va a verificar.
 * @return true si la cadena representa un número entero, false en caso contrario.
 */
bool is_integer(const std::string& str) {
    try {
        std::size_t pos; // Variable para almacenar la posición del primer carácter no convertido
        std::stoi(str, &pos); // Convertir la cadena a entero
        // La conversión tiene éxito si no hay caracteres restantes en la cadena
        return pos == str.size(); // Devolver true si no hay caracteres restantes, indicando que la conversión fue exitosa
    } catch (...) {
        return false; // Capturar cualquier excepción y devolver false si ocurre un error durante la conversión
    }
}



/**
 * Función para recolectar datos de los sensores y manejarlos entre hilos.
 * 
 * Esta función se ejecuta en un hilo separado. Abre un pipe para leer los datos
 * de los sensores y los procesa, distribuyéndolos entre los buffers correspondientes.
 * Si el sensor no está conectado, espera 10 segundos antes de terminar el proceso y 
 * enviar mensajes a los otros hilos para que también terminen.
 * 
 * @param arg Puntero a una estructura `ThreadArgs` que contiene los argumentos necesarios 
 *            para la función, incluyendo los buffers para pH y temperatura, el nombre del pipe
 *            y un semáforo.
 * @return void* Siempre devuelve nullptr.
 */
void* reco_hilo(void* arg) {
    // Convertir el argumento a un puntero ThreadArgs
    ThreadArgs* args = reinterpret_cast<ThreadArgs*>(arg);

    // Obtener los buffers y el nombre del pipe del argumento
    Buffer* bufferPh = args->pH_buffer;
    Buffer* bufferTemp = args->temp_buffer;
    const char* pipeName = args->pipeName;

    // Abrir el Pipe
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd < 0) {
        sem_post(&args->semaphore); // Incrementar el semáforo
        std::cerr << "Error: No se pudo abrir el pipe: " << pipeName << std::endl;
        return nullptr; // Salir de la función si hay un error
    }

    // Leer datos del pipe
    std::string line; // Variable para almacenar la línea leída del pipe
    while (true) { // Bucle infinito para leer continuamente del pipe
        char buffer[256]; // Buffer para almacenar los datos leídos
        int bytesRead = read(pipeFd, buffer, sizeof(buffer) - 1); // Leer datos del pipe
        if (bytesRead <= 0) { // Verificar si no se han leído bytes
            // El sensor no está conectado; esperando 10 segundos
            sleep(10); // Esperar 10 segundos
            // Enviar mensajes a los otros hilos para terminar
            bufferPh->add("-1"); // Agregar mensaje de terminación al buffer de pH
            bufferTemp->add("-1"); // Agregar mensaje de terminación al buffer de temperatura
            // Borrar el pipe y terminar el proceso
            unlink(pipeName); // Borrar el pipe
            std::cout << "Finalizado el procesamiento de mediciones" << std::endl; // Mensaje de finalización
            break; // Salir del bucle
        }

        // Procesar la línea y agregar a los buffers
        buffer[bytesRead] = '\0'; // Terminar la cadena con un carácter nulo
        line = buffer; // Convertir el buffer a un string
        if (is_integer(line)) { // Verificar si la línea es un entero
            int value = std::stoi(line); // Convertir la línea a entero
            if (value >= 0) { // Verificar si el valor es positivo
                bufferTemp->add(line); // Agregar el valor al buffer de temperatura
            } else {
                std::cerr << "Error: valor negativo recibido del sensor" << std::endl; // Mensaje de error si el valor es negativo
            }
        } else if (is_float(line)) { // Verificar si la línea es un flotante
            float value = std::stof(line); // Convertir la línea a flotante
            if (value >= 0.0f) { // Verificar si el valor es positivo
                bufferPh->add(line); // Agregar el valor al buffer de pH
            } else {
                std::cerr << "Error: valor negativo recibido del sensor" << std::endl; // Mensaje de error si el valor es negativo
            }
        } else {
            std::cerr << "Error: valor no válido recibido del sensor" << std::endl; // Mensaje de error si la línea no es válida
        }
    }

    // Cerrar el pipe
    close(pipeFd); // Cerrar el descriptor de archivo del pipe

    return nullptr; // Devolver nullptr al finalizar la función
}



/**
 * Función que maneja el procesamiento de datos de pH en un hilo separado.
 * 
 * Esta función se ejecuta en un hilo dedicado a manejar los datos de pH recolectados
 * por otro hilo y almacenados en un buffer. Abre un archivo para escribir los datos,
 * lee del buffer y escribe los valores de pH junto con la hora actual en el archivo.
 * Si se detectan valores fuera del rango normal, emite una alerta en la consola.
 * 
 * @param arg Puntero a una estructura `ThreadArgs` que contiene los argumentos necesarios 
 *            para la función, incluyendo el buffer para pH, el nombre del pipe y un semáforo.
 * @return void* Siempre devuelve nullptr.
 */
void* pH_hilo(void* arg) {
    // Convertir el argumento a un puntero ThreadArgs
    ThreadArgs* thread_args = reinterpret_cast<ThreadArgs*>(arg);

    // Obtener el buffer de pH y el valor del semáforo
    Buffer* pH_buffer = thread_args->pH_buffer;
    int semaphore_value;
    sem_getvalue(&thread_args->semaphore, &semaphore_value); // Obtener el valor del semáforo

    // Abrir el archivo para escribir los datos de pH
    std::ofstream pH_file("pH-data.txt"); // Abrir el archivo de datos de pH
    if (!pH_file.is_open()) { // Verificar si el archivo se abrió correctamente
        std::cerr << "Error: No se pudo abrir el archivo: pH-data.txt" << std::endl;
        return nullptr;
    }

    // Verificar el valor del semáforo antes de procesar los datos
    if (semaphore_value > 0) { // Si el semáforo es mayor que cero, se detiene el proceso
        pH_file.close(); // Cerrar el archivo
        pH_buffer->~Buffer(); // Destruir el buffer
        return nullptr;
    }

    // Leer datos del buffer y escribir en el archivo
    std::string data; // Variable para almacenar los datos leídos del buffer
    while ((data = pH_buffer->remove()) != "-1") { // Bucle para leer los datos del buffer
        float value = std::stof(data); // Convertir el dato a flotante
        if (value >= 8.0 || value <= 6.0) { // Verificar si el valor está fuera del rango normal
            std::cout << "¡Alerta! Valor de pH fuera del rango normal: " << value << std::endl;
        }
        pH_file << value << " " << getCurrentTime() << std::endl; // Escribir el valor de pH en el archivo
    }

    // Cerrar el archivo y destruir el buffer
    pH_file.close(); // Cerrar el archivo
    pH_buffer->~Buffer(); // Destruir el buffer

    return nullptr;
}

/**
 * Función que maneja el procesamiento de datos de temperatura en un hilo separado.
 * 
 * Esta función se ejecuta en un hilo dedicado a manejar los datos de temperatura recolectados
 * por otro hilo y almacenados en un buffer. Abre un archivo para escribir los datos, lee del buffer
 * y escribe los valores de temperatura junto con la hora actual en el archivo. Si se detectan valores
 * fuera del rango normal, emite una alerta en la consola.
 * 
 * @param arg Puntero a una estructura `ThreadArgs` que contiene los argumentos necesarios 
 *            para la función, incluyendo el buffer para temperatura, el nombre del pipe y un semáforo.
 * @return void* Siempre devuelve nullptr.
 */
void* temperatura_hilo(void* arg) {
    // Convertir el argumento a un puntero ThreadArgs
    ThreadArgs* thread_args = reinterpret_cast<ThreadArgs*>(arg);

    // Obtener el buffer de temperatura y el valor del semáforo
    Buffer* temperature_buffer = thread_args->temp_buffer;
    int semaphore_value;
    sem_getvalue(&thread_args->semaphore, &semaphore_value); // Obtener el valor del semáforo

    // Abrir el archivo para escribir los datos de temperatura
    std::ofstream temperature_file("temperature-data.txt"); // Abrir el archivo de datos de temperatura
    if (!temperature_file.is_open()) { // Verificar si el archivo se abrió correctamente
        std::cerr << "Error: No se pudo abrir el archivo: temperature-data.txt" << std::endl;
        return nullptr;
    }

    // Verificar el valor del semáforo antes de procesar los datos
    if (semaphore_value > 0) { // Si el semáforo es mayor que cero, se detiene el proceso
        temperature_file.close(); // Cerrar el archivo
        temperature_buffer->~Buffer(); // Destruir el buffer
        return nullptr;
    }

    // Leer datos del buffer y escribir en el archivo
    std::string data; // Variable para almacenar los datos leídos del buffer
    while ((data = temperature_buffer->remove()) != "-1") { // Bucle para leer los datos del buffer
        int value = std::stoi(data); // Convertir el dato a entero
        if (value >= 31.6 || value <= 20) { // Verificar si el valor está fuera del rango normal
            std::cout << "¡Alerta! Valor de temperatura fuera del rango normal: " << value << std::endl;
        }
        temperature_file << value << " " << getCurrentTime() << std::endl; // Escribir el valor de temperatura en el archivo
    }

    // Cerrar el archivo y destruir el buffer
    temperature_file.close(); // Cerrar el archivo
    temperature_buffer->~Buffer(); // Destruir el buffer

    return nullptr; // Devolver nullptr
}



int main(int argc, char *argv[]) {
    // Iniciando variables
    int option;  // Opción para getopt
    int bufferSize = 0;  // Tamaño del buffer para almacenar datos
    char* temperatureFile = nullptr;  // Nombre del archivo para datos de temperatura
    char* pHFile = nullptr;  // Nombre del archivo para datos de pH
    char* pipeName = nullptr;  // Nombre del pipe

    // Revisando argumentos y asignándolos
    while ((option = getopt(argc, argv, "b:t:h:p:")) != -1) {
        switch (option) {
            case 'b':
                bufferSize = atoi(optarg);  // Asignando el tamaño del buffer
                break;
            case 't':
                temperatureFile = optarg;  // Asignando el nombre del archivo de temperatura
                break;
            case 'h':
                pHFile = optarg;  // Asignando el nombre del archivo de pH
                break;
            case 'p':
                pipeName = optarg;  // Asignando el nombre del pipe
                break;
            default:
                std::cerr << "Uso: " << argv[0] << " -b tamañoBuffer -t archivoTemperatura -h archivoPh -p nombrePipe" << std::endl;
                return 1;
        }
    }

    // Creando Pipe
    if (mkfifo(pipeName, 0666) < 0) {  // Crea un pipe con permisos de lectura/escritura
        std::cerr << "Failed to create pipe: " << pipeName << std::endl;
        return 1;
    }

    // Abriendo pipe
    int pipeFd = open(pipeName, O_RDONLY);  // Abre el pipe para lectura
    if (pipeFd < 0) {
        std::cerr << "Failed to open pipe: " << pipeName << std::endl;
        return 1;
    }

    // Creando buffers
    Buffer bufferPh(bufferSize);  // Inicializa el buffer para datos de pH
    Buffer bufferTemp(bufferSize);  // Inicializa el buffer para datos de temperatura

    // Preparando los argumentos para los hilos
    ThreadArgs args;
    args.pH_buffer = &bufferPh;  // Asigna el buffer de pH
    args.temp_buffer = &bufferTemp;  // Asigna el buffer de temperatura
    args.pipeName = pipeName;  // Asigna el nombre del pipe
    sem_init(&args.semaphore, 0, 0);  // Inicializa el semáforo

    // Creando hilos
    pthread_t threadRecolector, threadPh, threadTemp;  // Identificadores para los hilos
    pthread_create(&threadRecolector, NULL, reco_hilo, &args);  // Crea el hilo recolector de datos
    pthread_create(&threadPh, NULL, pH_hilo, &args);  // Crea el hilo para manejar los datos de pH
    pthread_create(&threadTemp, NULL, temperatura_hilo, &args);  // Crea el hilo para manejar los datos de temperatura

    // Uniendo hilos
    pthread_join(threadRecolector, NULL);  // Espera a que el hilo recolector termine
    pthread_join(threadPh, NULL);  // Espera a que el hilo de pH termine
    pthread_join(threadTemp, NULL);  // Espera a que el hilo de temperatura termine

    // Cerrando pipe y destruyendo semáforo
    close(pipeFd);  // Cierra el file descriptor del pipe
    sem_destroy(&args.semaphore);  // Destruye el semáforo

    return 0;  // Finaliza el programa
}
