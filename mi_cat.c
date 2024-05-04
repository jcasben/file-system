/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

int main(int argc, char **args)
{

    if (argc < 3)
    {
        fprintf(
                stderr,
                RED
                "Invalid syntax. Usage: ./mi_cat <disco> </ruta_fichero>\n"
                RESET
        );
        return FALLO;
    }



    // Mount the device.
    if (bmount(args[1]) < 0) return FALLO;

    int tamBuffer = 1500;
    char buffer[tamBuffer];
    strcpy(buffer, args[3]);

    printf("Bytes leidos: %d", mi_read(args[2], buffer, offset, nbytes));

    if (bumount() < 0) return FALLO;

    return 0;
}