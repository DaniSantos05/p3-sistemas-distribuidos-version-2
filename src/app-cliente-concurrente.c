#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "claves.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <id_cliente>\n", argv[0]);
        return 1;
    }

    int id = atoi(argv[1]);

    // Cada cliente usa su propia clave para evitar colisiones
    char key[16];
    char key_inexistente[16];
    snprintf(key, sizeof(key), "clave_%d", id);
    snprintf(key_inexistente, sizeof(key_inexistente), "Z_%d", id);

    struct Paquete v3 = {1, 2, 3};
    float v2[3] = {1.1f, 2.2f, 3.3f};

    struct Paquete v3_mod = {7, 8, 9};
    float v2_mod[2] = {4.4f, 5.5f};

    char value1_out[256];
    int n_out;
    float v2_out[32];
    struct Paquete v3_out;

    int fallos = 0;

    printf("[Cliente %d] Iniciando pruebas con clave '%s'\n", id, key);

    // Test 1: insertar
    if (set_value(key, "valor1", 3, v2, v3) == 0) {
        printf("[Cliente %d] set_value: OK\n", id);
    } else {
        printf("[Cliente %d] set_value: MAL\n", id);
        fallos++;
    }

    // Test 2: duplicar (debe fallar)
    if (set_value(key, "otro", 1, v2, v3) == -1) {
        printf("[Cliente %d] duplicado: OK (fallo esperado)\n", id);
    } else {
        printf("[Cliente %d] duplicado: MAL (deberia haber fallado)\n", id);
        fallos++;
    }

    // Test 3: existencia
    if (exist(key) == 1) {
        printf("[Cliente %d] exist: OK\n", id);
    } else {
        printf("[Cliente %d] exist: MAL\n", id);
        fallos++;
    }

    // Test 4: leer
    if (get_value(key, value1_out, &n_out, v2_out, &v3_out) == 0) {
        printf("[Cliente %d] get_value: OK (value1=%s, N=%d)\n", id, value1_out, n_out);
    } else {
        printf("[Cliente %d] get_value: MAL\n", id);
        fallos++;
    }

    // Test 5: leer clave inexistente
    if (get_value(key_inexistente, value1_out, &n_out, v2_out, &v3_out) == -1) {
        printf("[Cliente %d] get inexistente: OK (fallo esperado)\n", id);
    } else {
        printf("[Cliente %d] get inexistente: MAL\n", id);
        fallos++;
    }

    // Test 6: modificar
    if (modify_value(key, "nuevo_valor", 2, v2_mod, v3_mod) == 0) {
        printf("[Cliente %d] modify_value: OK\n", id);
    } else {
        printf("[Cliente %d] modify_value: MAL\n", id);
        fallos++;
    }

    // Test 7: leer tras modificar
    if (get_value(key, value1_out, &n_out, v2_out, &v3_out) == 0) {
        printf("[Cliente %d] get tras modify: OK (value1=%s, N=%d)\n", id, value1_out, n_out);
    } else {
        printf("[Cliente %d] get tras modify: MAL\n", id);
        fallos++;
    }

    // Test 8: borrar
    if (delete_key(key) == 0) {
        printf("[Cliente %d] delete_key: OK\n", id);
    } else {
        printf("[Cliente %d] delete_key: MAL\n", id);
        fallos++;
    }

    // Test 9: comprobar que ya no existe
    if (exist(key) == 0) {
        printf("[Cliente %d] exist tras borrar: OK\n", id);
    } else {
        printf("[Cliente %d] exist tras borrar: MAL\n", id);
        fallos++;
    }

    if (fallos == 0) {
        printf("[Cliente %d] TODOS LOS TESTS PASADOS\n", id);
    } else {
        printf("[Cliente %d] %d TEST(S) FALLARON\n", id, fallos);
    }

    return fallos;
}