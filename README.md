# MoniSenso

# Proyecto de Monitoreo de Parámetros de una Reserva de Agua

Este proyecto emula la medición de dos parámetros vitales en una reserva de agua: el pH y la temperatura, mediante el uso de sensores. Los datos recopilados se transmiten a un proceso de monitoreo que los almacena y notifica al usuario si se detectan valores anómalos. A continuación, se describen los componentes del repositorio y su operación.

## Presentación
La presentación del proyecto se puede encontrar en el siguiente enlace:
[Presentación del Proyecto](https://drive.google.com/file/d/1KujNr6IprCh4j0Jq_AWek3IGJZyReA2_/view?usp=sharing)

En este documento, se describirá el sistema integral diseñado para simular y monitorear los datos de los sensores. La atención se centrará en los elementos fundamentales del sistema, abarcando su operatividad, configuración y posibles aplicaciones. Además, se exhibirán ejemplos prácticos que ilustrarán cómo emplear el sistema tanto para la simulación como para el monitoreo de datos de pH y temperatura.

## Contenido del Repositorio

### Código
- **buffer.cpp - buffer.h**: Componentes que ejecutan la función de búferes para temporalmente guardar las mediciones de los sensores.
- **datos.txt**: Archivo de datos destinado a propósitos de prueba.
- **pH-data.txt - temperature-data.txt**: Documentos de salida designados para almacenar las mediciones de pH y temperatura respectivamente.
- **monitor.cpp**: Desarrollo del proceso monitor encargado de administrar los hilos recolector, H-pH y H-temperatura.
- **sensor.cpp**: Implementación de los procesos simuladores de sensores que transmiten datos al monitor.
- **makefile**: Herramienta de automatización para compilar y ejecutar el proyecto.

## Ejecución

### Compilación
En la terminal, diríjase al directorio del proyecto y ejecute el comando:
```bash
make
```

### Inicio del Monitor
Desde la terminal, active el proceso del monitor de la siguiente manera:
```bash
./monitor -b tamBúfer -t datosTemperatura -h datosPH -p nombrePipe
```
Donde:
- `tamBúfer`: Capacidad de los búferes donde se registrarán las mediciones.
- `datosTemperatura`: Nombre del archivo de texto donde se almacenarán las mediciones de temperatura.
- `datosPH`: Nombre del archivo de texto donde se guardarán las mediciones de pH.
- `nombrePipe`: Nombre del conducto utilizado para la comunicación con el sensor.

### Inicio de los Sensores
En la terminal, ejecute los procesos de los sensores de esta manera:
```bash
./sensor -s tipo -t intervalo -f archivoConfig -p nombrePipe
```
Donde:
- `tipo`: Define el tipo de sensor; puede ser [1,2] `PH` o `temperatura`.
- `intervalo`: Indica el intervalo de tiempo entre las mediciones.
- `archivoConfig`: Nombre del archivo de configuración para el sensor.
- `nombrePipe`: Nombre del conducto utilizado para la comunicación con el monitor.
  
### Ejemplo Práctico
Para compilar el proyecto, utilice el siguiente comando:
```bash
make
```

Inicie el monitor con los siguientes parámetros:
```bash
./monitor -b 10 -t datosTemperatura.txt -h datosPH.txt -p pipe1
```

Ejecute el sensor (debe ser activado en menos de 10 segundos tras el inicio del monitor):
```bash
./sensor -s 2 -t 3 -f datos.txt -p pipe1
```

## Detalles del Proyecto
Para conocer más sobre el desarrollo del proyecto y sus especificaciones, acceda al siguiente enlace:
[Enunciado del Proyecto](MonitoreoSensores/MonitoreoSensores/ProyectosMoniSenso.pdf)
[Documentación del Proyecto](https://drive.google.com/file/d/1SjNGoGDtmeVB9IbDvwwBlRSYugdWSRcL/view?usp=drive_link)




