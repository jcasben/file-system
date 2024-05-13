/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "ficheros.h"

int main(int argc, char **args)
{
    if (argc < 4)
    {
        fprintf(
            stderr,
            RED
            "ERROR: invalid syntax. Usage: ./permitir <nombre_dispositivo> <ninodo> <permisos>\n"
            RESET
        );
        return FALLO;  
    }

    if (bmount(args[1]) == FALLO) return FALLO;

    int ninode = atoi(args[2]);
    int permissions = atoi(args[3]);
    mi_chmod_f(ninode, permissions);

    if (bumount() == FALLO) return FALLO;
}