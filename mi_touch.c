#include "directorios.h"

int main(int argc, char **args)
{
    if (argc < 4)
    {
        fprintf(
            stderr,
            RED
            "Syntax error: ./mi_touch <disco> <permisos> </ruta>\n"
            RESET
        );
        return FALLO;  
    }

    int permisos = atoi(args[2]);
    if (permisos < 0 || permisos > 7)
    {
        fprintf(
            stderr,
            RED
            "Error: permissions must be in between 0 and 7\n"
            RESET
        );
        return FALLO;
    }

    if (args[3][strlen(args[3]) - 1] == '/')
    {
        fprintf(
            stderr,
            RED
            "ERROR: path can not end with '/'. Try to use ./mi_mkdir instead\n"
            RESET
        );
        return FALLO;
    }

    if (bmount(args[1]) < 0) return FALLO;

    mi_creat(args[3], permisos);

    if (bumount() < 0) return FALLO;

    return EXITO;
}