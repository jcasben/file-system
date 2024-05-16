/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

int main(int argc, char **args)
{
    if (argc != 4)
    {
        fprintf(
                stderr,
                RED
                "ERROR: invalid syntax. Usage: ./mi_rn <disco> </ruta_antigua> <ruta_nueva>\n"
                RESET
        );
        return FALLO;
    }

    if (bmount(args[1]) < 0) return FALLO;

    unsigned int p_inodo_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    int error = buscar_entrada(args[2], &p_inodo_dir, &n_inode, &p_entry, 0, 6);
    if(error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    char path[TAMNOMBRE * PROFUNDIDAD];
    memset(path, 0, TAMNOMBRE * PROFUNDIDAD);

    if (args[2][strlen(args[2])] == '/') strncpy(path, args[2], strlen(args[2]) - 1);
    else strcpy(path, args[2]);

    char *last_dir = strrchr(path, '/');
    char new_path[TAMNOMBRE * PROFUNDIDAD];
    printf("%s\n", last_dir);
    strncpy(new_path, path, strlen(path) - strlen(last_dir));
    strcat(new_path, args[3]);
    if (args[2][strlen(args[2])] == '/') strcat(new_path, "/");

    printf("%s\n", new_path);

    struct entrada entrada;
    if(mi_read_f(p_inodo_dir, &entrada, p_entry * sizeof(struct entrada),
            sizeof(struct entrada)) == FALLO) return FALLO;
    strcpy(entrada.nombre, args[3]);
    if(mi_write_f(p_inodo_dir, &entrada, p_entry * sizeof(struct entrada),
            sizeof(struct entrada)) == FALLO) return FALLO;

    if (bumount() < 0) return FALLO;

    return EXITO;
}