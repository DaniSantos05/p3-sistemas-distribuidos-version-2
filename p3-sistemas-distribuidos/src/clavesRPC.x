const MAX_CADENA = 255;
const MAX_VECTOR = 32;

struct paquete_rpc {
    int x;
    int y;
    int z;
};

typedef string t_clave<MAX_CADENA>;
typedef string t_valor1<MAX_CADENA>;
typedef float t_vector_value2<MAX_VECTOR>;

struct rpc_get_value_res {
    int status;
    t_valor1 value1;
    int N_value2;
    t_vector_value2 V_value2;
    paquete_rpc value3;
};

program CLAVESRPC_PROG {
    version CLAVESRPC_VERS {
        int RPC_DESTROY(void) = 1;
        int RPC_SET_VALUE(t_clave key,
                          t_valor1 value1,
                          int N_value2,
                          t_vector_value2 V_value2,
                          paquete_rpc value3) = 2;
        rpc_get_value_res RPC_GET_VALUE(t_clave key) = 3;
        int RPC_MODIFY_VALUE(t_clave key,
                             t_valor1 value1,
                             int N_value2,
                             t_vector_value2 V_value2,
                             paquete_rpc value3) = 4;
        int RPC_DELETE_KEY(t_clave key) = 5;
        int RPC_EXIST(t_clave key) = 6;
    } = 1;
} = 0x31230001;
