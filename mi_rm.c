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
        fprintf(stderr, RED "ERROR: Incorrect number of arguments. Syntaxis: ./mi_rm disco /ruta \n" RESET);
        return FALLO;
    }

    if (args[2][strlen(args[2]) - 1] == '/')
    {
        fprintf(stderr, RED "ERROR: With mi_rm you can only delete files, if you want to delete a direcotry use mi_rmdir\n" RESET);
        return FALLO;
    }

    if (bmount(args[1]) < 0)
    {
        fprintf(stderr, RED "Error mounting the device\n" RESET);
        return FALLO;
    }

    if (mi_unlink(args[2]) < 0)
    {
        fprintf(stderr, RED "Error deleting the file\n" RESET);
        return FALLO;
    }

    if (bumount() < 0)
    {
        fprintf(stderr, RED "Error unmounting the device\n" RESET);
        return FALLO;
    }
    return EXITO;
}