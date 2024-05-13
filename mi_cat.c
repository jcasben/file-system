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
                "ERROR: invalid syntax. Usage: ./mi_cat <disco> </ruta_fichero>\n"
                RESET
        );
        return FALLO;
    }

    if(args[2][strlen(args[2])-1]==('/'))
    {
        fprintf(stderr, RED "ERROR: the path does not belong to a file\n" RESET);
        return FALLO;
    }

    // Mount the device.
    if (bmount(args[1]) < 0) return FALLO;

    int tamBuffer = BLOCKSIZE * 4;
    char buffer[tamBuffer];
    memset(buffer, 0, tamBuffer);
    int read_bytes;
    int total_bytes = 0;
    int i = 0;

    if ((read_bytes = mi_read(args[2], buffer, i * tamBuffer, tamBuffer)) == FALLO) return FALLO;
    while(read_bytes > 0)
    {
        total_bytes += read_bytes;
        write(1, buffer, read_bytes);
        //printf("%s", buffer);
        memset(buffer, 0, tamBuffer);
        i++;
        if ((read_bytes = mi_read(args[2], buffer, i * tamBuffer, tamBuffer)) == FALLO) return FALLO;
    }


    struct STAT stat;
    mi_stat(args[2], &stat);
    if (total_bytes != stat.tamEnBytesLog)
    {
        fprintf(
           stderr,
           RED
           "ERROR: the number of bytes read does not match the file size\n"
           RESET
       );
        return FALLO;
    }

    fprintf(stderr, "\nRead bytes: %d\n", total_bytes);

    if (bumount() < 0) return FALLO;

    return 0;
}