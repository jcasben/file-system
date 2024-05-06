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
        fprintf(stderr, RED "Syntax error: ./mi_chmod <disco> <permisos> </ruta>\n" RESET);
        return FALLO;
    }
    int permisos = atoi(argv[2]);
    if(permisos < 0 || permisos > 7)
    {
        fprintf(stderr, RED "ERROR: Los permisos deben ser un número entre 0 y 7\n" RESET);
        return FALLO;
    }
    if (bmount(argv[1]) < 0)
    {
        fprintf(stderr, RED "Error mounting the device\n" RESET);
        return FALLO;
    }
    
    if (mi_chmod(argv[3], permisos) < 0)
    {
        fprintf(stderr, RED "ERROR: No se ha podido cambiar los permisos\n" RESET);
        return FALLO;
    }

    if (bumount() < 0)
    {
        fprintf(stderr, RED "Error unmounting the device\n" RESET);
        return FALLO;
    }
}