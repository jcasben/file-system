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
    strcat(path, "/");
    strcat(path, args[3]);
    if (args[2][strlen(args[2])] == '/') strcat(path, "/");

    unsigned p_inodo_dir2 = p_inodo_dir;
    unsigned int n_inode2 = n_inode;
    unsigned int p_entry2 = p_entry;

    error = buscar_entrada(path, &p_inodo_dir2, &n_inode2, &p_entry2, 0, 6);
    if(error >= 0)
    {
        fprintf(stderr, RED "ERROR: the file/directory with name %s already exists\n" RESET, args[3]);
        return FALLO;
    }

    struct entrada entrada;
    if(mi_read_f(p_inodo_dir, &entrada, p_entry * sizeof(struct entrada),
            sizeof(struct entrada)) == FALLO) return FALLO;
    strcpy(entrada.nombre, args[3]);
    if(mi_write_f(p_inodo_dir, &entrada, p_entry * sizeof(struct entrada),
            sizeof(struct entrada)) == FALLO) return FALLO;

    if (bumount() < 0) return FALLO;

    return EXITO;
}