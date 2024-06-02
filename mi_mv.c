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
            "ERROR: invalid syntax. Usage: ./mi_rn <disco> </origen/nombre> </destino/>\n"
            RESET
        );
        exit(FALLO);
    }

    if (args[3][strlen(args[3]) - 1] != '/')
    {
        fprintf(
            stderr,
            RED
            "ERROR: destination must be a directory\n"
            RESET
        );
        exit(FALLO);
    }

    if (bmount(args[1]) < 0) return FALLO;

    unsigned int p_inodo_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    int error = buscar_entrada(args[2], &p_inodo_dir, &n_inode, &p_entry, 0, 6);
    if(error < 0)
    {
        mostrar_error_buscar_entrada(error);
        bumount();
        exit(FALLO);
    }

    char path[strlen(args[2])];
    memset(path, 0, sizeof(path));
    int is_directory = args[2][strlen(args[2]) - 1] == '/';
    if (is_directory) strncpy(path, args[2], strlen(args[2]) - 1);
    else strcpy(path, args[2]);

    char *file_name = strrchr(path, '/') + 1;
    char dest_path[strlen(args[3]) + TAMNOMBRE];
    memset(dest_path, 0, sizeof(dest_path));
    sprintf(dest_path, "%s%s", args[3], file_name);
    if (is_directory) strcat(dest_path, "/");
//    printf("%s\n", dest_path);

    unsigned p_inodo_dir2 = 0;
    unsigned int n_inode2 = 0;
    unsigned int p_entry2 = 0;

    error = buscar_entrada(dest_path, &p_inodo_dir2, &n_inode2, &p_entry2, 1, 6);
    if(error < 0)
    {
        mostrar_error_buscar_entrada(error);
        bumount();
        exit(FALLO);
    }

    struct entrada entrada;
    if(mi_read_f(p_inodo_dir, &entrada, p_entry * sizeof(struct entrada),
                 sizeof(struct entrada)) == FALLO) return FALLO;

    struct entrada entrada2;
    if(mi_read_f(p_inodo_dir2, &entrada2, p_entry2 * sizeof(struct entrada),
                 sizeof(struct entrada)) == FALLO) return FALLO;

    liberar_inodo(entrada2.ninodo);
    entrada2.ninodo = entrada.ninodo;

    mi_unlink(args[2]);

    if(mi_write_f(p_inodo_dir2, &entrada2, p_entry2 * sizeof(struct entrada),
                  sizeof(struct entrada)) == FALLO) return FALLO;

    if (bumount() < 0) return FALLO;

    return EXITO;
}