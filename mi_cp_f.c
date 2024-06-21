#include "directorios.h"

int main(int argc, char **argv)
{
    if(argc != 4)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: ./mi_cp_f <disco> </origen/nombre> </destino/>\n" RESET);
        exit(FALLO);
    }

    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
        fprintf(stderr,
                RED
                "ERROR: you can not copy a directory with this function\n"
                RESET);
        exit(FALLO);
    }

    if (argv[3][strlen(argv[3]) - 1] != '/')
    {
        fprintf(stderr,
                RED
                "ERROR: the destination must be a directory\n"
                RESET);
        exit(FALLO);
    }

    if (bmount(argv[1]) < 0) exit(FALLO);

    unsigned int p_inodo_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    int error = buscar_entrada(argv[2], &p_inodo_dir, &n_inode, &p_entry, 0, 6);
    if(error < 0)
    {
        mostrar_error_buscar_entrada(error);
        bumount();
        exit(FALLO);
    }

    struct entrada entry;
    memset(&entry, 0, sizeof(entry));
    mi_read_f(p_inodo_dir, &entry, p_entry * sizeof(struct entrada), sizeof(struct entrada));

    struct inodo inode_org;
    memset(&inode_org, 0, sizeof(struct inodo));
    leer_inodo(entry.ninodo, &inode_org);

    char path[strlen(argv[2])];
    memset(path, 0, sizeof(path));
    strcpy(path, argv[2]);

    char *file_name = strrchr(path, '/') + 1;
    char dest_path[strlen(argv[3]) + TAMNOMBRE];
    memset(dest_path, 0, sizeof(dest_path));
    sprintf(dest_path, "%s%s", argv[3], file_name);

    unsigned p_inodo_dir2 = 0;
    unsigned int n_inode2 = 0;
    unsigned int p_entry2 = 0;

    error = buscar_entrada(dest_path, &p_inodo_dir2, &n_inode2, &p_entry2, 1, inode_org.permisos);
    if(error < 0)
    {
        mostrar_error_buscar_entrada(error);
        bumount();
        exit(FALLO);
    }

    char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);
    int read_bytes;
    int i = 0;
    char zero_buffer[BLOCKSIZE];
    memset(zero_buffer, 0, BLOCKSIZE);

    if ((read_bytes = mi_read(argv[2], buffer, i * BLOCKSIZE, BLOCKSIZE)) == FALLO) return FALLO;
    while(read_bytes > 0)
    {
        if (memcmp(buffer, zero_buffer, BLOCKSIZE) != 0)
        {
            mi_write(dest_path, buffer, i * BLOCKSIZE, read_bytes);
            memset(buffer, 0, BLOCKSIZE);
        }
        i++;
        if ((read_bytes = mi_read(argv[2], buffer, i * BLOCKSIZE, BLOCKSIZE)) == FALLO) return FALLO;
    }

    if (bumount() < 0) exit(FALLO);
}
