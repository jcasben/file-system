#include "directorios.h"

int main(int argc, char **args)
{
    if (argc < 4)
    {
        fprintf(
            stderr,
            RED
            "Syntax error: ./mi_mkdir <disco> <permisos> </ruta/>\n"
            RESET
        );
        return FALLO;
    }

    if (args[3][strlen(args[3]) - 1] != '/')
    {
        fprintf(
            stderr,
            RED
            "Unable to create directory with specified path (path has to end with '/'). Try to use ./mi_touch instead\n"
            RESET
        );
        return FALLO;
    }

    int permisos = atoi(args[2]);

    if (permisos < 3 || permisos > 7)
    {
        fprintf(stderr, RED "ERROR: the permissions must be in between 3 and 7\n" RESET);
    }

    if (bmount(args[1]) < 0) return FALLO;

    mi_creat(args[3], permisos);

    if (bumount(args[1]) < 0) return FALLO;

    return EXITO;
}