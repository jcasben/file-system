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
        fprintf(
                stderr,
                RED
                "Invalid syntax. Usage: ./mi_cat <disco> </ruta_fichero>\n"
                RESET
        );
        return FALLO;
    }

    if(args[2][strlen(args[2])-1]==('/'))
    {
        fprintf(stderr, RED "Error: la ruta no se corresponde a un fichero" RESET);
        return FALLO;
    }

    // Mount the device.
    if (bmount(args[1]) < 0) return FALLO;

    int tamBuffer = 1500;
    char buffer[tamBuffer];
    strcpy(buffer, args[3]);

    //STAT stats = le

    int read_bytes = mi_read(args[2], buffer, 0, tamBuffer);

    printf("Bytes leidos: %d", read_bytes);
    printf("%s\n", buffer);
    // Deben coincidir 

    if (bumount() < 0) return FALLO;

    return 0;
}