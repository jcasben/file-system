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
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: ./mi_chmod <disco> <permisos> </ruta>\n" RESET);
        return FALLO;
    }
    int permisos = atoi(argv[2]);
    if(permisos < 0 || permisos > 7)
    {
        fprintf(stderr, RED "ERROR: permissions must be in between 0 and 7\n" RESET);
        return FALLO;
    }
    if (bmount(argv[1]) < 0) return FALLO;
    
    if (mi_chmod(argv[3], permisos) < 0)
    {
        fprintf(stderr, RED "ERROR: could change the permissions\n" RESET);
        return FALLO;
    }

    if (bumount() < 0) return FALLO;
}