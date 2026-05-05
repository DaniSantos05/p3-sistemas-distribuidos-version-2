#include "clavesRPC.h"
#include "claves.h" // Incluimos la lógica real del servidor (la base de datos local)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Las funciones generadas por rpcgen para el servidor terminan en '_svc'.
 * Tienen que devolver bool_t (TRUE o FALSE) para indicar si RPC pudo procesar la llamada,
 * NO es el valor de retorno de tu función (ese se guarda en el puntero *result).
 */

/*
 * Operación 1: DESTROY
 */
bool_t
rpc_destroy_1_svc(int *result, struct svc_req *rqstp)
{
    (void)rqstp; // rqstp tiene datos de la petición (IP del cliente, etc). No lo usamos.

    fprintf(stderr, "[servidor] destroy()\n"); // Log por pantalla para ver qué pasa
    
    // Llamamos a la función destroy() real que tienes programada en el servidor.
    // Su retorno lo guardamos en la variable a la que apunta *result para que rpcgen lo envíe de vuelta.
    *result = destroy(); 
    
    fprintf(stderr, "[servidor] destroy() -> %d\n", *result);
    return TRUE; // Le decimos a RPC: "Hecho, puedes enviar la respuesta"
}

/*
 * Operación 2: SET_VALUE
 */
bool_t
rpc_set_value_1_svc(t_clave key, t_valor1 value1, int N_value2,
                    t_vector_value2 V_value2, paquete_rpc value3,
                    int *result, struct svc_req *rqstp)
{
    struct Paquete value3_local;

    (void)rqstp;

    fprintf(stderr, "[servidor] set_value(key='%s', value1='%s', N=%d)\n",
            key, value1, N_value2);

    // 1. Validamos que el número N coincida con el tamaño real del array que nos mandó rpcgen.
    // Esto previene que si un cliente malicioso miente, intentemos leer fuera de la memoria.
    if (N_value2 != (int)V_value2.t_vector_value2_len) {
        *result = -1;
        fprintf(stderr, "[servidor] set_value -> -1 (N_value2 incoherente)\n");
        return TRUE;
    }

    // 2. Convertimos el struct generado por rpcgen al struct nativo de C que espera nuestro set_value real.
    value3_local.x = value3.x;
    value3_local.y = value3.y;
    value3_local.z = value3.z;

    // 3. Llamamos a nuestra base de datos local y guardamos el resultado
    *result = set_value(key, value1, N_value2, V_value2.t_vector_value2_val, value3_local);
    fprintf(stderr, "[servidor] set_value(key='%s') -> %d\n", key, *result);
    return TRUE;
}

/*
 * Operación 3: GET_VALUE
 */
bool_t
rpc_get_value_1_svc(t_clave key, rpc_get_value_res *result, struct svc_req *rqstp)
{
    // Variables locales para recibir los datos desde nuestra base de datos
    char value1_local[256];
    int n_local = 0;
    float v2_local[32];
    struct Paquete value3_local;

    (void)rqstp;

    fprintf(stderr, "[servidor] get_value(key='%s')\n", key);
    memset(result, 0, sizeof(*result)); // Limpiamos la estructura de respuesta

    // 1. Preguntamos a nuestra base de datos local por los datos de esa 'key'
    result->status = get_value(key, value1_local, &n_local, v2_local, &value3_local);

    /*
     * Si la clave existía (status == 0), tenemos que meter los datos en '*result'.
     * PROBLEMA: XDR (el traductor de rpcgen a red) necesita que los strings y arrays 
     * sean punteros que sigan existiendo después de que esta función termine. 
     * Las variables locales se borran al hacer 'return', así que TENEMOS que usar malloc().
     */
    if (result->status == 0) {
        
        // Reservamos memoria para el string (texto + el caracter nulo \0)
        result->value1 = (char *)malloc(strlen(value1_local) + 1);
        
        if (result->value1 == NULL) { // Control de fallo de RAM
            result->status = -1;
            result->N_value2 = 0;
            result->V_value2.t_vector_value2_len = 0;
            result->V_value2.t_vector_value2_val = NULL;
            return TRUE;
        }

        // Copiamos el texto desde nuestra variable local al bloque reservado
        strcpy(result->value1, value1_local);

        result->N_value2 = n_local;
        result->V_value2.t_vector_value2_len = (u_int)n_local;

        // Si hay floats, reservamos memoria para el array
        if (n_local > 0) {
            result->V_value2.t_vector_value2_val =
                (float *)malloc((size_t)n_local * sizeof(float));

            if (result->V_value2.t_vector_value2_val == NULL) {
                // Si falla, liberamos lo que ya habíamos reservado arriba (el string) para no dejar basura
                free(result->value1);
                result->value1 = NULL;
                result->status = -1;
                result->N_value2 = 0;
                result->V_value2.t_vector_value2_len = 0;
                result->V_value2.t_vector_value2_val = NULL;
                return TRUE;
            }

            // Copiamos los floats desde el array local al bloque reservado
            memcpy(result->V_value2.t_vector_value2_val,
                   v2_local,
                   (size_t)n_local * sizeof(float));
        } else {
            result->V_value2.t_vector_value2_val = NULL;
        }

        // Los tipos básicos (ints) se asignan directamente por valor, no necesitan malloc
        result->value3.x = value3_local.x;
        result->value3.y = value3_local.y;
        result->value3.z = value3_local.z;
    }

    fprintf(stderr, "[servidor] get_value(key='%s') -> %d\n", key, result->status);
    
    // Le decimos a rpcgen: "Terminé. Coge *result, serialízalo (pásalo a bytes) y mándalo por red".
    return TRUE; 
}

/*
 * Operación 4: MODIFY_VALUE
 */
bool_t
rpc_modify_value_1_svc(t_clave key, t_valor1 value1, int N_value2,
                       t_vector_value2 V_value2, paquete_rpc value3,
                       int *result, struct svc_req *rqstp)
{
    // Idéntico a set_value, pero llama a la función real modify_value()
    struct Paquete value3_local;

    (void)rqstp;

    fprintf(stderr, "[servidor] modify_value(key='%s', value1='%s', N=%d)\n",
            key, value1, N_value2);

    if (N_value2 != (int)V_value2.t_vector_value2_len) {
        *result = -1;
        fprintf(stderr, "[servidor] modify_value -> -1 (N_value2 incoherente)\n");
        return TRUE;
    }

    value3_local.x = value3.x;
    value3_local.y = value3.y;
    value3_local.z = value3.z;

    *result = modify_value(key, value1, N_value2, V_value2.t_vector_value2_val, value3_local);
    fprintf(stderr, "[servidor] modify_value(key='%s') -> %d\n", key, *result);
    return TRUE;
}

/*
 * Operación 5: DELETE_KEY
 */
bool_t
rpc_delete_key_1_svc(t_clave key, int *result, struct svc_req *rqstp)
{
    (void)rqstp;

    fprintf(stderr, "[servidor] delete_key(key='%s')\n", key);
    *result = delete_key(key);
    fprintf(stderr, "[servidor] delete_key(key='%s') -> %d\n", key, *result);
    return TRUE;
}

/*
 * Operación 6: EXIST
 */
bool_t
rpc_exist_1_svc(t_clave key, int *result, struct svc_req *rqstp)
{
    (void)rqstp;

    fprintf(stderr, "[servidor] exist(key='%s')\n", key);
    *result = exist(key);
    fprintf(stderr, "[servidor] exist(key='%s') -> %d\n", key, *result);
    return TRUE;
}

/*
 * LA FUNCIÓN "ESCOBA" (Garbage Collector Manual)
 * ¿Recuerdas los malloc() que hicimos en rpc_get_value_1_svc?
 * Si no los liberamos, el servidor se quedaría sin memoria tras muchos GET_VALUE.
 */
int
clavesrpc_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
    (void)transp;

    /* 
     * Rpcgen llama a esta función automáticamente DESPUÉS de haber enviado
     * los datos al cliente por la red de forma exitosa.
     * xdr_free es una función de la librería RPC que sabe cómo recorrer 
     * el struct complejo 'result' y hacer free() de todos los punteros 
     * (strings y arrays) que haya dentro.
     */
    xdr_free(xdr_result, result);
    
    return 1; // 1 significa que la liberación fue exitosa
}