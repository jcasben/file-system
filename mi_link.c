/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

int main(int argc, char **argv)
{
    if(argc != 4)
    {
        fprintf(stderr,
                RED
                "ERROR: invalid syntax. Usage: ./mi_link disco /ruta_fichero_original /ruta_enlace\n"
                RESET);
        return FALLO;
    }

    if (bmount(argv[1]) < 0) return FALLO;


    if (argv[2][strlen(argv[2]) - 1] == '/')
    {
        fprintf(stderr,
                RED
                "ERROR: the original file cannot be a directory\n"
                RESET);
        return FALLO;
    }

    if (argv[3][strlen(argv[3]) - 1] == '/')
    {
        fprintf(stderr,
                RED
                "ERROR: the link route cannot be a directory route\n"
                RESET);
        return FALLO;
    }

    if (mi_link(argv[2], argv[3]) < 0)
    {
        return FALLO;
    }

    if (bumount() < 0) return FALLO;
}