#include "directorios.h"

void recursive_copy(char *path, char *dest);
void copy_file(char *path, char *dest, char permisos);

int main(int argc, char **argv)
{
    if(argc != 4)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: ./mi_cp <disco> </origen/nombre> </destino/>\n" RESET);
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
    int is_directory = argv[2][strlen(argv[2]) - 1] == '/';
    if (is_directory) strncpy(path, argv[2], strlen(argv[2]) - 1);
    else strcpy(path, argv[2]);

    char *file_name = strrchr(path, '/') + 1;
    char dest_path[strlen(argv[3]) + TAMNOMBRE];
    memset(dest_path, 0, sizeof(dest_path));
    sprintf(dest_path, "%s%s", argv[3], file_name);
    if (is_directory) strcat(dest_path, "/");

    recursive_copy(argv[2], dest_path);

    if (bumount() < 0) exit(FALLO);
}

void recursive_copy(char *path, char *dest)
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

    if (inode.tipo == 'f')
    {
        copy_file(path, dest, inode.permisos);
        return;
    }

    unsigned int p_inode_dir2 = 0;
    unsigned int n_inode2 = 0;
    unsigned int p_entry2 = 0;
    buscar_entrada(dest, &p_inode_dir2, &n_inode2, &p_entry2, 1, inode.permisos);

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

        char new_dest[strlen(buffer_lec[index].nombre) + TAMNOMBRE + 1];
        memset(new_dest, 0, sizeof(child_path));
        strcat(new_dest, dest);
        strcat(new_dest, buffer_lec[index].nombre);
        if (inode.tipo == 'd') strcat(new_dest, "/");

        recursive_copy(child_path, new_dest);

        index++;
        if (index == BLOCKSIZE / sizeof(struct entrada))
        {
            offset += mi_read_f(n_inode, buffer_lec, offset, BLOCKSIZE);
            index = 0;
        }
        entry_inode_number++;
    }
}

void copy_file(char *path, char *dest, char permisos)
{
    unsigned p_inodo_dir2 = 0;
    unsigned int n_inode2 = 0;
    unsigned int p_entry2 = 0;

    int error = buscar_entrada(dest, &p_inodo_dir2, &n_inode2, &p_entry2, 1, permisos);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        bumount();
        return;
    }

    char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);
    int read_bytes;
    int i = 0;
    char zero_buffer[BLOCKSIZE];
    memset(zero_buffer, 0, BLOCKSIZE);

    if ((read_bytes = mi_read(path, buffer, i * BLOCKSIZE, BLOCKSIZE)) == FALLO) return;
    while (read_bytes > 0) {
        if (memcmp(buffer, zero_buffer, BLOCKSIZE) != 0) {
            mi_write(dest, buffer, i * BLOCKSIZE, read_bytes);
            memset(buffer, 0, BLOCKSIZE);
        }
        i++;
        if ((read_bytes = mi_read(path, buffer, i * BLOCKSIZE, BLOCKSIZE)) == FALLO) return;
    }
}