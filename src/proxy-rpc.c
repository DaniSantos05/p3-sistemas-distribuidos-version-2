#include "claves.h"
#include "clavesRPC.h" // Archivo generado por rpcgen con las estructuras y prototipos

#include <stdlib.h>
#include <string.h>

/*
 * Función auxiliar para crear la conexión con el servidor RPC.
 * Evita repetir el mismo código de conexión en todas las funciones.
 */
static CLIENT *crear_cliente(void)
{
    char *host; // Almacena la IP del servidor

    // 1. Obtiene la dirección IP del servidor desde la variable de entorno
    host = getenv("IP_TUPLAS");
    if (host == NULL) {
        // Si el usuario no definió la variable, fallamos antes de intentar conectar
        return NULL; 
    }

    // 2. Crea el cliente RPC. 
    // Argumentos: IP, Número de Programa, Versión, Protocolo (TCP)
    return clnt_create(host, CLAVESRPC_PROG, CLAVESRPC_VERS, "tcp");
}

/*
 * Operación 1: DESTROY
 * Pide al servidor que borre toda su base de datos.
 */
int destroy(void)
{
    CLIENT *clnt;
    enum clnt_stat estado_rpc; // Guarda si la comunicación por red falló o no (Estado RPC)
    int resultado;             // Guarda lo que devolvió la función destroy() real en el servidor

    // 1. Establecemos conexión
    clnt = crear_cliente();
    if (clnt == NULL) {
        return -1; // Fallo al conectar
    }

    // 2. Llamamos a la función generada por rpcgen.
    // Le pasamos dónde guardar la respuesta (&resultado) y el manejador de conexión (clnt)
    estado_rpc = rpc_destroy_1(&resultado, clnt);
    
    // 3. Cerramos la conexión para no dejar sockets colgados
    clnt_destroy(clnt);

    // 4. Comprobamos si hubo un error a nivel de RED (ej. se cayó internet)
    if (estado_rpc != RPC_SUCCESS) {
        return -1;
    }

    // 5. Devolvemos el resultado lógico (ej. 0 si borró bien, -1 si el servidor falló internamente)
    return resultado;
}

/*
 * Operación 2: SET_VALUE
 * Envía una clave y sus valores al servidor para que los guarde.
 */
int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3)
{
    CLIENT *clnt;
    enum clnt_stat estado_rpc;
    int resultado;
    
    // Variables temporales para adaptar los tipos de C normales a los tipos que exige rpcgen
    t_vector_value2 vector_rpc; 
    paquete_rpc paquete;

    // 1. Validación de seguridad en el cliente antes de molestar al servidor
    if (N_value2 < 1 || N_value2 > 32 || V_value2 == NULL) {
        return -1;
    }

    clnt = crear_cliente();
    if (clnt == NULL) {
        return -1;
    }

    // 2. Adaptación de datos (Marshalling manual para las estructuras complejas)
    // rpcgen usa un struct especial para arrays de longitud variable (len y val)
    vector_rpc.t_vector_value2_len = (u_int) N_value2;
    vector_rpc.t_vector_value2_val = V_value2;

    // Copiamos los datos de nuestro struct normal al struct generado por rpcgen
    paquete.x = value3.x;
    paquete.y = value3.y;
    paquete.z = value3.z;

    // 3. Llamada RPC. Rpcgen convierte esto en bytes y lo manda por red.
    estado_rpc = rpc_set_value_1(key, value1, N_value2, vector_rpc, paquete, &resultado, clnt);
    clnt_destroy(clnt);

    if (estado_rpc != RPC_SUCCESS) {
        return -1;
    }

    return resultado;
}

/*
 * Operación 3: GET_VALUE
 * Pide al servidor los valores asociados a una clave
 * Es la más compleja porque el servidor tiene que devolvernos memoria dinámica     
 */
int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3)
{
    CLIENT *clnt;
    enum clnt_stat estado_rpc;
    rpc_get_value_res resultado; // Struct complejo generado por rpcgen que agrupa toda la respuesta

    // 1. Comprobamos que se nos ha pasado punteros válidos para rellenarlos
    if (value1 == NULL || N_value2 == NULL || V_value2 == NULL || value3 == NULL) {
        return -1;
    }

    // Limpiamos la estructura donde rpcgen meterá los datos recibidos
    memset(&resultado, 0, sizeof(resultado));

    clnt = crear_cliente();
    if (clnt == NULL) {
        return -1;
    }

    // 2. Llamada RPC. El servidor rellena '&resultado'
    estado_rpc = rpc_get_value_1(key, &resultado, clnt);
    clnt_destroy(clnt);

    // 3. Control de errores. Si la red falló, liberamos la memoria que XDR (rpcgen) 
    // haya podido reservar a medias y salimos
    if (estado_rpc != RPC_SUCCESS) {
        xdr_free((xdrproc_t)xdr_rpc_get_value_res, (char *)&resultado);
        return -1;
    }

    // Si el servidor encontró un error lógico (ej. la clave no existe)
    if (resultado.status != 0) {
        xdr_free((xdrproc_t)xdr_rpc_get_value_res, (char *)&resultado);
        return -1;
    }

    // 4. Validamos que los datos que nos mandó el servidor son coherentes (anti-corrupción)
    if (resultado.N_value2 < 1 || resultado.N_value2 > 32 ||
        resultado.V_value2.t_vector_value2_len != (u_int)resultado.N_value2 ||
        (resultado.N_value2 > 0 && resultado.V_value2.t_vector_value2_val == NULL) ||
        resultado.value1 == NULL) {
        xdr_free((xdrproc_t)xdr_rpc_get_value_res, (char *)&resultado);
        return -1;
    }

    // 5. Desempaquetamos. Copiamos los datos del struct de rpcgen 
    // a las variables que el usuario de nuestra API nos pasó por referencia.
    strcpy(value1, resultado.value1);
    *N_value2 = resultado.N_value2;
    memcpy(V_value2,
           resultado.V_value2.t_vector_value2_val,
           (size_t)resultado.N_value2 * sizeof(float));

    value3->x = resultado.value3.x;
    value3->y = resultado.value3.y;
    value3->z = resultado.value3.z;

    // 6. Liberamos la memoria dinámica que rpcgen reservó internamente 
    // al recibir los arrays y strings desde la red. Si no, habría "memory leaks".
    xdr_free((xdrproc_t)xdr_rpc_get_value_res, (char *)&resultado);
    return 0;
}

/*
 * Operación 4: MODIFY_VALUE
 * Casi idéntica a SET_VALUE, pero llama a rpc_modify_value_1
 */
int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3)
{
    CLIENT *clnt;
    enum clnt_stat estado_rpc;
    int resultado;
    t_vector_value2 vector_rpc;
    paquete_rpc paquete;

    if (N_value2 < 1 || N_value2 > 32 || V_value2 == NULL) {
        return -1;
    }

    clnt = crear_cliente();
    if (clnt == NULL) {
        return -1;
    }

    vector_rpc.t_vector_value2_len = (u_int) N_value2;
    vector_rpc.t_vector_value2_val = V_value2;

    paquete.x = value3.x;
    paquete.y = value3.y;
    paquete.z = value3.z;

    estado_rpc = rpc_modify_value_1(key, value1, N_value2, vector_rpc, paquete, &resultado, clnt);
    clnt_destroy(clnt);

    if (estado_rpc != RPC_SUCCESS) {
        return -1;
    }

    return resultado;
}

/*
 * Operación 5: DELETE_KEY
 * Borra una clave específica.
 */
int delete_key(char *key)
{
    CLIENT *clnt;
    enum clnt_stat estado_rpc;
    int resultado;

    clnt = crear_cliente();
    if (clnt == NULL) {
        return -1;
    }

    // Solo pasamos la clave, no hace falta marshalling de structs complejos
    estado_rpc = rpc_delete_key_1(key, &resultado, clnt);
    clnt_destroy(clnt);

    if (estado_rpc != RPC_SUCCESS) {
        return -1;
    }

    return resultado;
}

/*
 * Operación 6: EXIST
 * Comprueba si una clave existe (devuelve 1 o 0, o -1 en error).
 */
int exist(char *key)
{
    CLIENT *clnt;
    enum clnt_stat estado_rpc;
    int resultado;

    clnt = crear_cliente();
    if (clnt == NULL) {
        return -1;
    }

    estado_rpc = rpc_exist_1(key, &resultado, clnt);
    clnt_destroy(clnt);

    if (estado_rpc != RPC_SUCCESS) {
        return -1;
    }

    return resultado;
}