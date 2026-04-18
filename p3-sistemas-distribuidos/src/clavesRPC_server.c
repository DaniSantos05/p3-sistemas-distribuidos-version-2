#include "clavesRPC.h"
#include "claves.h"

#include <stdlib.h>
#include <string.h>

bool_t
rpc_destroy_1_svc(int *result, struct svc_req *rqstp)
{
	(void)rqstp;

	*result = destroy();
	return TRUE;
}

bool_t
rpc_set_value_1_svc(t_clave key, t_valor1 value1, int N_value2,
                    t_vector_value2 V_value2, paquete_rpc value3,
                    int *result, struct svc_req *rqstp)
{
	struct Paquete value3_local;

	(void)rqstp;

	if (N_value2 != (int)V_value2.t_vector_value2_len) {
		*result = -1;
		return TRUE;
	}

	value3_local.x = value3.x;
	value3_local.y = value3.y;
	value3_local.z = value3.z;

	*result = set_value(key, value1, N_value2, V_value2.t_vector_value2_val, value3_local);
	return TRUE;
}

bool_t
rpc_get_value_1_svc(t_clave key, rpc_get_value_res *result, struct svc_req *rqstp)
{
	char value1_local[256];
	int n_local = 0;
	float v2_local[32];
	struct Paquete value3_local;

	(void)rqstp;

	memset(result, 0, sizeof(*result));

	result->status = get_value(key, value1_local, &n_local, v2_local, &value3_local);

	if (result->status == 0) {
		result->value1 = (char *)malloc(strlen(value1_local) + 1);
		if (result->value1 == NULL) {
			result->status = -1;
			result->N_value2 = 0;
			result->V_value2.t_vector_value2_len = 0;
			result->V_value2.t_vector_value2_val = NULL;
			return TRUE;
		}

		strcpy(result->value1, value1_local);

		result->N_value2 = n_local;
		result->V_value2.t_vector_value2_len = (u_int)n_local;

		if (n_local > 0) {
			result->V_value2.t_vector_value2_val =
				(float *)malloc((size_t)n_local * sizeof(float));

			if (result->V_value2.t_vector_value2_val == NULL) {
				free(result->value1);
				result->value1 = NULL;
				result->status = -1;
				result->N_value2 = 0;
				result->V_value2.t_vector_value2_len = 0;
				result->V_value2.t_vector_value2_val = NULL;
				return TRUE;
			}

			memcpy(result->V_value2.t_vector_value2_val,
			       v2_local,
			       (size_t)n_local * sizeof(float));
		} else {
			result->V_value2.t_vector_value2_val = NULL;
		}

		result->value3.x = value3_local.x;
		result->value3.y = value3_local.y;
		result->value3.z = value3_local.z;
	}

	return TRUE;
}

bool_t
rpc_modify_value_1_svc(t_clave key, t_valor1 value1, int N_value2,
                       t_vector_value2 V_value2, paquete_rpc value3,
                       int *result, struct svc_req *rqstp)
{
	struct Paquete value3_local;

	(void)rqstp;

	if (N_value2 != (int)V_value2.t_vector_value2_len) {
		*result = -1;
		return TRUE;
	}

	value3_local.x = value3.x;
	value3_local.y = value3.y;
	value3_local.z = value3.z;

	*result = modify_value(key, value1, N_value2, V_value2.t_vector_value2_val, value3_local);
	return TRUE;
}

bool_t
rpc_delete_key_1_svc(t_clave key, int *result, struct svc_req *rqstp)
{
	(void)rqstp;

	*result = delete_key(key);
	return TRUE;
}

bool_t
rpc_exist_1_svc(t_clave key, int *result, struct svc_req *rqstp)
{
	(void)rqstp;

	*result = exist(key);
	return TRUE;
}

int
clavesrpc_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	(void)transp;

	xdr_free(xdr_result, result);
	return 1;
}
