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
    memset(buffer, 0, tamBuffer);
    strcpy(buffer, args[3]);
    int read_bytes = 1;
    int total_bytes = 0;
    int i = 0;

    while(read_bytes > 0)
    {
        read_bytes = mi_read(args[2], buffer, i * tamBuffer, tamBuffer);
        total_bytes += read_bytes;
        printf("%s", buffer);
        memset(buffer, 0, tamBuffer);
        i++;
    }

    printf("Bytes leidos: %d", read_bytes);
    // Deben coincidir 

    if (bumount() < 0) return FALLO;

    return 0;
}