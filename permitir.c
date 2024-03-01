#include "ficheros.h"

int main(int argc, char **args)
{
    // TODO: permitir.c implementation
    if (argc != 4)
    {
        fprintf(
            stderr,
            RED
            "Syntax error: ./permitir <nombre_dispositivo> <ninodo> <permisos>\n"
            RESET
        );
        return FALLO;  
    }

    if (bmount(args[1]) == FALLO)
    {
        fprintf(
            stderr,
            RED
            "Error: unable to mount the file system\n"
            RESET
        );
        return FALLO;
    }

    int ninode = atoi(args[2]);
    int permissions = atoi(args[3]);
    mi_chmod_f(ninode, permissions);

    if (bumount() == FALLO)
    {
        fprintf(
            stderr,
            RED
            "Error: unable to unmount the file system\n"
            RESET
        );
        return FALLO;
    }
    
}