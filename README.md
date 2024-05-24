# MoniSenso

# Proyecto de Monitoreo de Parámetros de una Reserva de Agua

Este proyecto simula la medición de dos parámetros críticos en una reserva de agua: el pH y la temperatura, utilizando sensores. Los datos medidos se envían a un proceso monitor que los almacena y genera alertas al usuario si se detectan valores fuera del rango normal. A continuación, se detallan los componentes del repositorio y su funcionamiento.

## Presentación
La presentación del proyecto está disponible en el siguiente enlace:
[Presentación del Proyecto](https://drive.google.com/file/d/1KujNr6IprCh4j0Jq_AWek3IGJZyReA2_/view?usp=sharing)

En esta presentación se explica el sistema completo para la simulación y monitoreo de datos de sensores. Se enfocará en los aspectos principales del sistema, incluyendo su funcionamiento, configuración y aplicaciones. También se mostrarán ejemplos de uso del sistema para simular y monitorear datos de pH y temperatura.

## Contenido del Repositorio

### Código
- **buffer.cpp - buffer.h**: Módulos que implementan los búferes para almacenar temporalmente las medidas de los sensores.
- **datos.txt**: Archivo de datos para pruebas.
- **file-ph.txt - file-temp.txt**: Archivos de salida donde se almacenarán las medidas de pH y temperatura respectivamente.
- **monitor.cpp**: Implementación del proceso monitor que gestiona los hilos recolector, H-pH y H-temperatura.
- **sensor.cpp**: Implementación de los procesos simuladores de sensores que envían datos al monitor.
- **makefile**: Script de automatización para compilar y ejecutar el proyecto.

## Ejecución

### Compilación
En la terminal, diríjase al directorio del proyecto y ejecute el comando:
```bash
make
```

### Ejecución del Monitor
Desde el shell, invoque el proceso del monitor de la siguiente manera:
```bash
./monitor -b tam_buffer -t file-temp -h file-ph -p pipe-nominal
```
Donde:
- `tam-buffer`: Tamaño de los búferes donde se almacenarán las medidas.
- `file-temp`: Nombre del archivo de texto donde el hilo de temperatura almacenará las mediciones de temperatura recibidas.
- `file-ph`: Nombre del archivo de texto donde el hilo de pH almacenará las mediciones de pH recibidas.
- `pipe-nominal`: Pipe nominal utilizado para la comunicación con el sensor.

### Ejecución de los Sensores
Desde el shell, invoque los procesos de los sensores de la siguiente manera:
```bash
./sensor -s tipo-sensor -t tiempo -f archivo -p pipe-nominal
```
Donde:
- `tipo-sensor`: Tipo de sensor, puede ser `PH` o `temperatura` (1 o 2).
- `tiempo`: Intervalo de tiempo entre las medidas.
- `archivo`: Archivo de configuración para el sensor.
- `pipe-nominal`: Pipe nominal utilizado para la comunicación con el monitor.

### Ejemplo
Compilación del proyecto:
```bash
make
```

Ejecución del monitor:
```bash
./monitor -b 10 -t file-temp.txt -h file-ph.txt -p pipe1
```

Ejecución del sensor (debe ejecutarse en menos de 10 segundos después del monitor):
```bash
./sensor -s 2 -t 3 -f datos.txt -p pipe1
```

## Documentación del Desarrollo
Puede acceder a la documentación completa del desarrollo del proyecto en el siguiente enlace:
[Documentación del Proyecto]()
```
