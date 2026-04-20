# P3-SISTEMAS-DISTRIBUIDOS (ejercicio_evaluable3)

Práctica de Sistemas Distribuidos que implementa un servicio de almacenamiento de tuplas de la forma `<key, value1, value2, value3>` utilizando **ONC RPC** para la comunicación entre cliente y servidor. El sistema permite insertar, consultar, modificar, eliminar y comprobar la existencia de claves mediante una API en C

La práctica se desarrolla en una versión distribuida mediante **ONC RPC (Sun RPC)**, con un servidor generado a partir de stubs RPC y una biblioteca proxy para el cliente (`libclaves.so`)

Cada tupla almacenada contiene:
- `key`: cadena de hasta 255 caracteres
- `value1`: cadena de hasta 255 caracteres
- `value2`: vector de `float` con tamaño entre 1 y 32
- `value3`: estructura con tres valores enteros `(x, y, z)`

Requisitos:
- Sistema operativo Linux
- gcc
- make
- rpcgen
- librería tirpc o nsl

Variables de entorno (cliente):
- `IP_TUPLAS`: dirección IP o nombre de la máquina donde ejecuta el servidor RPC

Instalación:
1. Clonar o descargar el repositorio del proyecto
2. Generar los ficheros RPC con rpcgen:
   - make rpcgen
3. Compilar el proyecto:
   - make
4. Limpiar archivos compilados (opcional):
   - make clean
5. Limpiar también los ficheros generados por rpcgen (opcional):
   - make distclean

Ejecución:
- Iniciar el servidor (en una terminal):
  ./clavesRPC_server
- Definir la variable de entorno e iniciar el cliente (en otra terminal):
  export IP_TUPLAS=localhost
  ./app-cliente
  O de forma alternativa en una sola línea:
  env IP_TUPLAS=localhost ./app-cliente

El cliente utiliza la biblioteca `libclaves.so`, que se encarga de realizar las llamadas RPC al servidor. Se pueden ejecutar varios clientes simultáneamente para probar el funcionamiento concurrente del servidor.

Estructura y archivos:
- Código principal:
- src/
  - `app-cliente.c` — aplicación cliente de prueba del servicio
  - `claves.c` — implementación del servicio de almacenamiento de tuplas
  - `claves.h` — definición de la API del servicio
  - `clavesRPC.x` — definición de la interfaz RPC en lenguaje XDR
  - `proxy-rpc.c` — implementación del proxy cliente (implementa claves.h mediante RPC)
  - `clavesRPC_server.c` — implementación de los procedimientos remotos en el servidor
- Ficheros generados automáticamente por rpcgen (no incluidos en el repositorio):
  - `clavesRPC.h` — cabecera con tipos y prototipos generados
  - `clavesRPC_clnt.c` — stub del cliente
  - `clavesRPC_svc.c` — stub del servidor
  - `clavesRPC_xdr.c` — filtros de serialización XDR
- Bibliotecas generadas:
  - `libclaves.so` — biblioteca cliente para la versión RPC
- Ejecutables generados:
  - `clavesRPC_server` — servidor del sistema distribuido
  - `app-cliente` — cliente de prueba del servicio
- Otros archivos:
  - `.gitignore` — excluye archivos objeto, bibliotecas, ejecutables y ficheros de rpcgen
  - `Makefile` — compilación automática del proyecto
  - `memoria.pdf` — documentación del diseño y estructura de archivos

Notas:
- Para las llamadas RPC no es necesario conocer el puerto del servidor, solo su IP
- El binding entre cliente y servidor se realiza automáticamente a través del proceso rpcbind (puerto 111)
- El servidor no necesita ningún argumento en la línea de comandos
- Las funciones de la API devuelven:
  - `0` si la operación se realiza correctamente
  - `-1` si ocurre un error (clave inexistente, duplicada, fuera de rango, o error RPC)
- Se recomienda probar el sistema ejecutando varios clientes simultáneamente para verificar el funcionamiento concurrente del servidor
