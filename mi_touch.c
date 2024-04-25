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
            "Unable to create file with specified path (path can not end in '/'). For this use /mi_mkdir\n"
            RESET
        );
        return FALLO;
    }


}