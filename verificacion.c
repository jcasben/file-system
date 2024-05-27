#include "verificacion.h"

int main(int argc, char **args)
{
    if (argc != 3)
    {
        fprintf(
            stderr,
            RED
            "ERROR: invalid syntax. Usage: ./verificacion <nombre_dispositivo> <directorio_simulación>\n"
            RESET
        );
    }

    if (bmount(args[1]) < 0) return FALLO;

    struct STAT stat;
    if(mi_stat(args[2], &stat)< 0 ) return FALLO;

    int num_entradas = stat.tamEnBytesLog/ sizeof(struct entrada);
    if(num_entradas != NUMPROCESOS) return FALLO;

    char ruta[128];
    sprintf(ruta, "%sinforme.txt", args[2]);
    if((mi_creat(ruta, 7)) < 0) return FALLO;

    struct entrada entradas[num_entradas];
    read_subdirectories(args[2], entradas, num_entradas);

    

    if(bumount() < 0) return FALLO;

    return EXITO;
}

int read_subdirectories(const char *camino, struct entrada *entradas, int *num_entradas)
{
    struct superbloque SB;
    bread(posSB, &SB);

    unsigned int p_inode_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    int error = buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    // Read inode with the assigned value of the function buscar_entrada()
    struct inodo inode;
    if (leer_inodo(n_inode, &inode) == FALLO) return FALLO;
    unsigned long cant_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada);
    if (inode.tipo != tipo)
    {
        fprintf(stderr, RED "ERROR: the type of the file doesn't correspond with the path\n" RESET);
        return FALLO;
    }
    
    if (tipo == 'd' && cant_entries_inode > 0)
    {
        printf("Total: %lu\n", cant_entries_inode);
        if (flag == 'l')
        {
            printf("\nTYPE\tPERMISSIONS\tmTIME\t\t\tSIZE\tNAME\n"
                       "---------------------------------------------------------------------\n");
        }
        int nbloc = 0;
        int entry_inode_number = 0;

        struct entrada buffer_lec[BLOCKSIZE/sizeof(struct entrada)];
        if (mi_read_f(n_inode, buffer_lec, nbloc * BLOCKSIZE, BLOCKSIZE) == FALLO) return FALLO;

        while(entry_inode_number < cant_entries_inode)
        {

            for (size_t j = 0; j < cant_entries_inode; j++)
            {
                if (flag == 'l') build_extended_buffer(buffer_lec[j], buffer);
                else build_buffer(buffer_lec[j], buffer);
                entry_inode_number++;
                if (entry_inode_number == BLOCKSIZE / sizeof(struct entrada)) break;
            }
            nbloc++;
            memset(buffer_lec, 0, BLOCKSIZE);
            if (mi_read_f(n_inode, buffer_lec, nbloc * BLOCKSIZE, BLOCKSIZE) == FALLO) return FALLO;
        }
    } else if (tipo == 'f')
    {
        struct entrada entry;
        if (mi_read_f(p_inode_dir,&entry,p_entry * sizeof(struct entrada),sizeof(struct entrada)) == FALLO) return FALLO;

        if (flag == 'l')
        {
            printf("\nTYPE\tPERMISSIONS\tmTIME\t\t\tSIZE\tNAME\n"
                   "---------------------------------------------------------------------\n");
            build_extended_buffer(entry, buffer);
        }
        else build_buffer(entry, buffer);
    }

    return EXITO;
}