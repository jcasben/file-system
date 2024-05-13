/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "ficheros.h"

int main(int argc, char **args)
{
    int noffsets = 5;
    unsigned int offsets[] = {9000, 209000, 30725000, 409605000, 480000000};
    int n_inode;
    int b_writed;

    if (argc < 4)
    {
        fprintf(
            stderr, 
            RED 
            "ERROR: invalid syntax. Usage: ./escribir <nombre_dispositivo> <\"$(cat fichero\">"
            "<diferentes_inodos>\n" 
            RESET
        );
        return FALLO;
    }

    // Mount the device.
    if (bmount(args[1]) == FALLO) return FALLO;
    
    int dif_inodos = atoi(args[3]);

    printf("text length: %ld", strlen(args[2]));
    char buffer[strlen(args[2])];
    strcpy(buffer, args[2]);

    // Reserve only one inode for all the offsets.
    if (dif_inodos == 0)
    {
        if((n_inode = reservar_inodo('f', 6)) == FALLO) return FALLO;

        
        for (size_t i = 0; i < noffsets; i++)
        {
            printf ("\nNº de inodo reservado: %d\n", n_inode);
            printf ("offset: %d\n", offsets[i]);

            if((b_writed = mi_write_f(n_inode, buffer, offsets[i], strlen(args[2]))) == FALLO) return FALLO;

            printf ("Bytes escritos: %d\n", b_writed);
            struct STAT stat;
            mi_stat_f(n_inode, &stat);
            printf ("stat.tamEnBytesLOG = %d\n", stat.tamEnBytesLog);
            printf ("stat.numBloquesOcupados = %d\n", stat.numBloquesOcupados);

        }
    }
    // Reserve one inode for each offset.
    else if (dif_inodos == 1)
    {
        for (size_t i = 0; i < noffsets; i++)
        {
            if ((n_inode = reservar_inodo('f',6)) == FALLO) return FALLO;
            printf ("\nNº de inodo reservado: %d\n", n_inode);
            printf ("offset: %d\n", offsets[i]);

            if((b_writed = mi_write_f(n_inode, buffer, offsets[i], strlen(args[2]))) == FALLO) return FALLO;
            
            printf ("Bytes escritos: %d\n", b_writed);
            struct STAT stat;
            mi_stat_f(n_inode, &stat);
            printf ("stat.tamEnBytesLOG = %d\n", stat.tamEnBytesLog);
            printf ("stat.numBloquesOcupados = %d\n", stat.numBloquesOcupados);
        }
    }
    else
    {
        fprintf(
            stderr,
            RED 
            "ERROR: invalid argument. The third argument must be 0 or 1\n"
            RESET
        );
        return FALLO;
    }

    if (bumount() == FALLO) return FALLO;
}