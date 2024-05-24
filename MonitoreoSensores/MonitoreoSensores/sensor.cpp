#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    // Declaración de variables para los argumentos de línea de comandos
    int opcion;
    int tipoSensor = 0;
    int intervaloTiempo = 0;
    char* archivoDatosNombre = nullptr;
    char* pipeNombre = nullptr;

    // Procesamiento de argumentos de línea de comandos usando getopt
    while ((opcion = getopt(argc, argv, "s:t:f:p:")) != -1) {
        switch (opcion) {
            case 's':
                // Asigna el tipo de sensor basado en el argumento
                tipoSensor = atoi(optarg);
                break;
            case 't':
                // Asigna el intervalo de tiempo entre lecturas basado en el argumento
                intervaloTiempo = atoi(optarg);
                break;
            case 'f':
                // Asigna el nombre del archivo de datos basado en el argumento
                archivoDatosNombre = optarg;
                break;
            case 'p':
                // Asigna el nombre del pipe basado en el argumento
                pipeNombre = optarg;
                break;
            default:
                // Muestra el uso correcto del programa en caso de argumentos incorrectos
                std::cerr << "Uso: " << argv[0] << " -s tipoSensor -t intervaloTiempo -f archivoDatosNombre -p pipeNombre" << std::endl;
                return 1;
        }
    }

    // Abre el archivo de datos para lectura
    std::ifstream archivoDatos(archivoDatosNombre);
    if (!archivoDatos.is_open()) {
        // Muestra un mensaje de error si no se puede abrir el archivo
        std::cerr << "Error: No se pudo abrir el archivo de datos: " << archivoDatosNombre << std::endl;
        return 1;
    }

    // Intento de abrir el pipe en modo escritura no bloqueante
    int pipeFd;
    do {
        pipeFd = open(pipeNombre, O_WRONLY | O_NONBLOCK);
        if (pipeFd < 0) {
            // Muestra un mensaje de error si no se puede abrir el pipe y reintenta después de 1 segundo
            std::cerr << "Error: No se pudo abrir el pipe: " << pipeNombre << ", reintentando..." << std::endl;
            sleep(1);
        }
    } while (pipeFd < 0);

    // Lectura del archivo línea por línea y escritura en el pipe
    std::string linea;
    ssize_t bytesEscritos;
    while (std::getline(archivoDatos, linea)) {
        // Escribe la línea leída en el pipe
        bytesEscritos = write(pipeFd, linea.c_str(), linea.size() + 1);
        if (bytesEscritos == -1) {
            // Muestra un mensaje de error si falla la escritura en el pipe y cierra los recursos
            std::cerr << "Error: Falló la escritura en el pipe" << std::endl;
            close(pipeFd);
            archivoDatos.close();
            return 1;
        }
        // Muestra la línea escrita en la salida estándar
        std::cerr << linea << std::endl;
        // Espera el intervalo de tiempo especificado antes de leer la siguiente línea
        sleep(intervaloTiempo);
    }

    // Cierra el archivo de datos y el descriptor del pipe
    archivoDatos.close();
    close(pipeFd);

    return 0;
}