#include "claves.h"
#include "clavesRPC.h"

#include <stdlib.h>
#include <string.h>

static CLIENT *crear_cliente(void)
{
	char *host;

	host = getenv("IP_TUPLAS");
	if (host == NULL) {
		return NULL;
	}

	return clnt_create(host, CLAVESRPC_PROG, CLAVESRPC_VERS, "tcp");
}

int destroy(void)
{
	CLIENT *clnt;
	enum clnt_stat estado_rpc;
	int resultado;

	clnt = crear_cliente();
	if (clnt == NULL) {
		return -1;
	}

	estado_rpc = rpc_destroy_1(&resultado, clnt);
	clnt_destroy(clnt);

	if (estado_rpc != RPC_SUCCESS) {
		return -1;
	}

	return resultado;
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3)
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

	estado_rpc = rpc_set_value_1(key, value1, N_value2, vector_rpc, paquete, &resultado, clnt);
	clnt_destroy(clnt);

	if (estado_rpc != RPC_SUCCESS) {
		return -1;
	}

	return resultado;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3)
{
	CLIENT *clnt;
	enum clnt_stat estado_rpc;
	rpc_get_value_res resultado;

	if (value1 == NULL || N_value2 == NULL || V_value2 == NULL || value3 == NULL) {
		return -1;
	}

	memset(&resultado, 0, sizeof(resultado));

	clnt = crear_cliente();
	if (clnt == NULL) {
		return -1;
	}

	estado_rpc = rpc_get_value_1(key, &resultado, clnt);
	clnt_destroy(clnt);

	if (estado_rpc != RPC_SUCCESS) {
		xdr_free((xdrproc_t)xdr_rpc_get_value_res, (char *)&resultado);
		return -1;
	}

	if (resultado.status != 0) {
		xdr_free((xdrproc_t)xdr_rpc_get_value_res, (char *)&resultado);
		return -1;
	}

	if (resultado.N_value2 < 1 || resultado.N_value2 > 32 ||
	    resultado.V_value2.t_vector_value2_len != (u_int)resultado.N_value2 ||
	    (resultado.N_value2 > 0 && resultado.V_value2.t_vector_value2_val == NULL) ||
	    resultado.value1 == NULL) {
		xdr_free((xdrproc_t)xdr_rpc_get_value_res, (char *)&resultado);
		return -1;
	}

	strcpy(value1, resultado.value1);
	*N_value2 = resultado.N_value2;
	memcpy(V_value2,
	       resultado.V_value2.t_vector_value2_val,
	       (size_t)resultado.N_value2 * sizeof(float));

	value3->x = resultado.value3.x;
	value3->y = resultado.value3.y;
	value3->z = resultado.value3.z;

	xdr_free((xdrproc_t)xdr_rpc_get_value_res, (char *)&resultado);
	return 0;
}

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

int delete_key(char *key)
{
	CLIENT *clnt;
	enum clnt_stat estado_rpc;
	int resultado;

	clnt = crear_cliente();
	if (clnt == NULL) {
		return -1;
	}

	estado_rpc = rpc_delete_key_1(key, &resultado, clnt);
	clnt_destroy(clnt);

	if (estado_rpc != RPC_SUCCESS) {
		return -1;
	}

	return resultado;
}

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
