/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

int main(int argc, char **args)
{
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: ./mi_rmdir disco /ruta/ \n" RESET);
        return FALLO;
    }

    if (strcmp(args[2], "/") == 0)
    {
        fprintf(stderr, RED "ERROR: Cannot delete the root directory\n" RESET);
        return FALLO;
    }

    if (args[2][strlen(args[2]) - 1] != '/')
    {
        fprintf(stderr, RED "ERROR: can not delete file with mi_rmdir. Try mi_rm instead\n" RESET);
        return FALLO;
    }

    if (bmount(args[1]) < 0) return FALLO;

    if (mi_unlink(args[2]) < 0) fprintf(stderr, RED "ERROR: couldn't dele the file\n" RESET);

    if (bumount() < 0) return FALLO;
}