#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    // Inicialización de variables
    int option;
    int tipoSensor = 0;
    int intervaloTiempo = 0;
    char* nombreArchivo = nullptr;
    char* nombrePipe = nullptr;
    // Revisando argumentos y asignándolos
    while ((option = getopt(argc, argv, "s:t:f:p:")) != -1) {
        switch (option) {
            case 's':
                tipoSensor = atoi(optarg);
                break;
            case 't':
                intervaloTiempo = atoi(optarg);
                break;
            case 'f':
                nombreArchivo = optarg;
                break;
            case 'p':
                nombrePipe = optarg;
                break;
            default:
                std::cerr << "Uso: " << argv[0] << " -s tipoSensor -t intervaloTiempo -f nombreArchivo -p nombrePipe" << std::endl;
                return 1;
        }
    }

    // Abriendo archivo para lectura
    std::ifstream archivoDatos(nombreArchivo);
    if (!archivoDatos.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo de datos: " << nombreArchivo << std::endl;
        return 1;
    }

    // Abriendo el pipe
    int pipeFd;
    do {
        pipeFd = open(nombrePipe, O_WRONLY | O_NONBLOCK);
        if (pipeFd < 0) {
            std::cerr << "Error: No se pudo abrir el pipe: " << nombrePipe << ", reintentando..." << std::endl;
            sleep(1);
        }
    } while (pipeFd < 0);

    // Leyendo el archivo y escribiendo en el pipe
    std::string linea;
    ssize_t bytesEscritos;
    while (std::getline(archivoDatos, linea)) {
        bytesEscritos = write(pipeFd, linea.c_str(), linea.size() + 1);
        if (bytesEscritos == -1) {
            std::cerr << "Error: Falló la escritura en el pipe" << std::endl;
            close(pipeFd);
            archivoDatos.close();
            return 1;
        }
        std::cerr << linea << std::endl;
        sleep(intervaloTiempo);
    }

    // Cerrando el archivo y el pipe
    archivoDatos.close();
    close(pipeFd);

    return 0;
}
