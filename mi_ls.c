#include "directorios.h"

int main(int argc, char **args)
{
    if (argc < 3 || argc > 4)
    {
        fprintf(
            stderr,
            RED
            "Syntax error: ./mi_ls <disco> </ruta>. Try ./mi_ls -l <disco> </ruta> to get an extended information\n"
            RESET
        );
        return FALLO;
    }

    if (argc == 4 && strcmp(args[1], "-l") != 0)
    {
        fprintf(
            stderr,
            RED
            "Flag %s not recognized. Try using flag -l to get an extended view\n"
            RESET,
            args[1]
        );
        return FALLO;
    }

    char type = ' ';
    char flag = ' ';
    char buffer[TAMBUFFER];
    char *path;
    char *disco;

    if (argc == 4)
    {
        path = args[3];
        disco = args[2];
        flag = 'l';
        if (args[3][strlen(args[3]) - 1] != '/') type = 'f';
        else type = 'd';
    } else if (argc == 3)
    {
        path = args[2];
        disco = args[1];
        if (args[2][strlen(args[2]) - 1] != '/') type = 'f';
        else type = 'd';
    }

    if (bmount(disco) < 0) return FALLO;

    mi_dir(path, buffer, type, flag);
    printf("%s\n", buffer);

    if (bumount()) return FALLO;
}