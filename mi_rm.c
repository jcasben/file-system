/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

int main(int argc, char **args)
{
    if (bmount(args[1]) < 0)
    {
        fprintf(stderr, RED "Error mounting the device\n" RESET);
        return FALLO;
    }


    if (bumount() < 0)
    {
        fprintf(stderr, RED "Error unmounting the device\n" RESET);
        return FALLO;
    }
    return EXITO;
}