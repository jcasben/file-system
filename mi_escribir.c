/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

int main(int argc, char **args)
{

    if (argc != 5)
    {
        fprintf(
            stderr, 
            RED 
            "ERROR: invalid syntax. Usage: ./mi_escribir <disco> </ruta_fichero>"
            "<texto> <offset>\n" 
            RESET
        );
        return FALLO;
    }

    // Mount the device.
    if (bmount(args[1]) < 0) return FALLO;

    unsigned int offset = (unsigned int) atoi(args[4]);
    unsigned int nbytes = (unsigned int) strlen(args[3]);

    printf("Text length: %d\n", nbytes);
    char buffer[nbytes + 1];
    strcpy(buffer, args[3]);

    int writen_bytes;
    if ((writen_bytes = mi_write(args[2], buffer, offset, nbytes)) == FALLO) return FALLO;

    printf("Writen bytes: %d\n", writen_bytes);

    if (bumount() < 0) return FALLO;

    return 0;
}