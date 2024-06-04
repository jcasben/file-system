/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

void recursive_delete(char *path);

int main(int argc, char **args)
{
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: Incorrect number of arguments. Syntaxis: ./mi_rm disco /ruta \n" RESET);
        return FALLO;
    }

    if (bmount(args[1]) < 0) return FALLO;

    recursive_delete(args[2]);

    if (bumount() < 0) return FALLO;
}

void recursive_delete(char *path)
{
    unsigned int p_inode_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    int error = buscar_entrada(path, &p_inode_dir, &n_inode, &p_entry, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return;
    }

    struct inodo inode;
    leer_inodo(n_inode, &inode);

    if (inode.tamEnBytesLog == 0 || inode.tipo == 'f')
    {
        mi_unlink(path);
        return;
    }

    unsigned long cant_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada);
    int entry_inode_number = 0;

    struct entrada buffer_lec[BLOCKSIZE/sizeof(struct entrada)];
    memset(buffer_lec, 0, sizeof(struct entrada));
    int offset = mi_read_f(n_inode, buffer_lec, 0, BLOCKSIZE);
    if(offset == FALLO) return;
    int index = 0;

    while (entry_inode_number < cant_entries_inode) {
        //Leer el inodo correspndiente
        if (leer_inodo(buffer_lec[index].ninodo, &inode) < 0) return;

        char child_path[strlen(buffer_lec[index].nombre) + TAMNOMBRE + 1];
        memset(child_path, 0, sizeof(child_path));
        strcat(child_path, path);
        strcat(child_path, buffer_lec[index].nombre);
        if (inode.tipo == 'd') strcat(child_path, "/");

        recursive_delete(child_path);

        index++;
        if (index == BLOCKSIZE / sizeof(struct entrada))
        {
            offset += mi_read_f(n_inode, buffer_lec, offset, BLOCKSIZE);
            index = 0;
        }
        entry_inode_number++;
    }

    mi_unlink(path);
}
