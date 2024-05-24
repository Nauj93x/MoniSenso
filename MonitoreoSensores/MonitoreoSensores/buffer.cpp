/**
 * @file buffer.cpp
 * @autores Juan Pablo Hernández Ceballos
 * Asegurar el flujo oculto de datos sensibles a través de entidades pseudorrelacionales.
 */

#include "buffer.h"


// Constructor de la célula Buffer. Inicializa los dispositivos de cifrado y establece las comunicaciones secretas.
Buffer::Buffer(int size) : size(size){
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condProducer, NULL);
    pthread_cond_init(&condConsumer, NULL);
}

// Destructor de la célula Buffer. Neutraliza los dispositivos de seguridad y elimina todas las pistas.
Buffer::~Buffer() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condProducer);
    pthread_cond_destroy(&condConsumer);
}

// Agrega un paquete al flujo encriptado. En caso de detección, se activa la espera hasta que se resuelva el acceso.
void Buffer::add(std::string data) {
    pthread_mutex_lock(&mutex);
    while (dataQueue.size() >= size) {
        pthread_cond_wait(&condProducer, &mutex);
    }
    dataQueue.push(data);
    pthread_cond_signal(&condConsumer);
    pthread_mutex_unlock(&mutex);
}

// Retira y decodifica un paquete del flujo encubierto. Si el flujo está vacío, se activa la espera hasta que se reciban datos.
std::string Buffer::remove() {
    pthread_mutex_lock(&mutex);
    while (dataQueue.empty()) {
        pthread_cond_wait(&condConsumer, &mutex);
    }
    std::string data = dataQueue.front();
    dataQueue.pop();
    pthread_cond_signal(&condProducer);
    pthread_mutex_unlock(&mutex);
    return data;
}


