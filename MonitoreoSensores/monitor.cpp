#include <iostream>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include "buffer.cpp"


struct ThreadArgs {
    Buffer* pH_buffer;
    Buffer* temp_buffer;
    char* pipeName;
    sem_t semaphore;
};

// Función para obtener la hora actual
std::string getCurrentTime() {
    std::time_t currentTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&currentTime);
    char timeString[100];
    std::strftime(timeString, sizeof(timeString), "%H:%M:%S", localTime);
    return std::string(timeString);
}

// Función para verificar si una cadena representa un número flotante
bool is_float(const std::string& str) {
    try {
        std::size_t pos;
        std::stof(str, &pos);
        //tiene éxito si no hay caracteres restantes en la cadena
        return pos == str.size();
    } catch (...) {
        return false;
    }
}

// Función para verificar si una cadena representa un número entero
bool is_integer(const std::string& str) {
    try {
        std::size_t pos;
        std::stoi(str, &pos);
        // La conversión tiene éxito si no hay caracteres restantes en la cadena
        return pos == str.size();
    } catch (...) {
        return false;
    }
}

// Funcion para recolectar datos de los sensores y manejarlos entre Hilos
void* reco_hilo(void* arg) {
    ThreadArgs* args = reinterpret_cast<ThreadArgs*>(arg);
    Buffer* bufferPh = args->pH_buffer;
    Buffer* bufferTemp = args->temp_buffer;
    const char* pipeName = args->pipeName;

    // Abrir el Pipe
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd < 0) {
        sem_post(&args->semaphore);
        std::cerr << "Failed to open pipe: " << pipeName << std::endl;
        return nullptr;
    }
    // Leer datos del pipe
    std::string line;
    while (true) {
        char buffer[256];
        int bytesRead = read(pipeFd, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            // El sensor no está conectado; esperando 10 segundos
            sleep(10);
            // Enviando mensajes a los otros hilos para terminar
            bufferPh->add("-1");
            bufferTemp->add("-1");
            // Borrando el pipe y terminando el proceso
            unlink(pipeName);
            std::cout << "Finalizado el procesamiento de mediciones" << std::endl;
            break;
        }
        // Procesar la línea y agregar a los buffers
        buffer[bytesRead] = '\0';
        line = buffer;
        // Comprobar si la línea es un entero
        if (is_integer(line)) {
            int value = std::stoi(line);
            if (value >= 0) {
                bufferTemp->add(line);
            } else {
                std::cerr << "Error: valor negativo recibido del sensor" << std::endl;
            }
        }
        // Comprobar si la línea es un flotante
        else if (is_float(line)) {
            float value = std::stof(line);
            if (value >= 0.0f) {
                bufferPh->add(line);
            } else {
                std::cerr << "Error: valor negativo recibido del sensor" << std::endl;
            }
        }
        else {
            std::cerr << "Error: valor no válido recibido del sensor" << std::endl;
        }
    }
    // Cerrar el pipe
    close(pipeFd);

    return nullptr;
}


void* pH_hilo(void* arg) {
    ThreadArgs* thread_args = reinterpret_cast<ThreadArgs*>(arg);
    Buffer* pH_buffer = thread_args->pH_buffer;
    int semaphore_value;
    sem_getvalue(&thread_args->semaphore, &semaphore_value);
    // Abrir el archivo
    std::ofstream pH_file("pH-data.txt");
    if (!pH_file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo: pH-data.txt" << std::endl;
        return nullptr;
    }

    if (semaphore_value > 0){
        pH_file.close();
        pH_buffer->~Buffer();
        return nullptr;
    }

    // Escribir datos del buffer al archivo
    std::string data;
    while ((data = pH_buffer->remove()) != "-1") {
        float value = std::stof(data);
        if(value >=8.0 || value <= 6.0){
            std::cout << "¡Alerta! Valor de pH fuera del rango normal: " << value << std::endl;
        }
        pH_file << value << " " << getCurrentTime() << std::endl;
    }

    // Cerrar archivo
    pH_file.close();
    pH_buffer->~Buffer();

    return nullptr;
}


void* temperatura_hilo(void* arg) {
    ThreadArgs* thread_args = reinterpret_cast<ThreadArgs*>(arg);
    Buffer* temperature_buffer = thread_args->temp_buffer;
    int semaphore_value;
    sem_getvalue(&thread_args->semaphore, &semaphore_value);
    // Abrir el archivo
    std::ofstream temperature_file("temperature-data.txt");
    if (!temperature_file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo: temperature-data.txt" << std::endl;
        return nullptr;
    }
    if (semaphore_value > 0){
        temperature_file.close();
        temperature_buffer->~Buffer();
        return nullptr;
    }
    // Escribir datos del buffer al archivo
    std::string data;
    while ((data = temperature_buffer->remove()) != "-1") {
        int value = std::stoi(data);
        if(value >=31.6 || value <= 20){
            std::cout << "¡Alerta! Valor de temperatura fuera del rango normal: " << value << std::endl;
        }
        temperature_file << value << " " << getCurrentTime() << std::endl;
    }
    // Cerrar archivo
    temperature_file.close();
    temperature_buffer->~Buffer();
    return nullptr;
}
int main(int argc, char *argv[]) {
  // Iniciando variables
  int option;
  int bufferSize = 0;
  char* temperatureFile = nullptr;
  char* pHFile = nullptr;
  char* pipeName = nullptr;
  // Revisando argumentos y asignándolos
  while ((option = getopt(argc, argv, "b:t:h:p:")) != -1) {
      switch (option) {
          case 'b':
              bufferSize = atoi(optarg);
              break;
          case 't':
              temperatureFile = optarg;
              break;
          case 'h':
              pHFile = optarg;
              break;
          case 'p':
              pipeName = optarg;
              break;
          default:
              std::cerr << "Uso: " << argv[0] << " -b tamañoBuffer -t archivoTemperatura -h archivoPh -p nombrePipe" << std::endl;
              return 1;
      }
  }

    // Creando Pipe
    if (mkfifo(pipeName, 0666) < 0) {
        std::cerr << "Failed to create pipe: " << pipeName << std::endl;
        return 1;
    }

    // Abriendo pipe
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd < 0) {
        std::cerr << "Failed to open pipe: " << pipeName << std::endl;
        return 1;
    }

    // Creando buffers
    Buffer bufferPh(bufferSize);
    Buffer bufferTemp(bufferSize);

    ThreadArgs args;
    args.pH_buffer = &bufferPh;
    args.temp_buffer = &bufferTemp;
    args.pipeName = pipeName;
    sem_init(&args.semaphore, 0, 0);
    // Creando Hilos
    pthread_t threadRecolector, threadPh, threadTemp;
    pthread_create(&threadRecolector, NULL, reco_hilo, &args);
    pthread_create(&threadPh, NULL, pH_hilo, &args);
    pthread_create(&threadTemp, NULL, temperatura_hilo, &args);
    // Uniendo Hilos
    pthread_join(threadRecolector, NULL);
    pthread_join(threadPh, NULL);
    pthread_join(threadTemp, NULL);

  // Cerrando pipe y destruyendo semáforo
    close(pipeFd);
    sem_destroy(&args.semaphore);
    return 0;
}