#include "directorios.h"

int main(int argc, char **args)
{
    if (argc < 3)
    {
        fprintf(
            stderr,
            RED
            "Syntax error: ./mi_ls <disco> </ruta>\n"
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
    }

    char type;
    if (args[3][strlen(args[3]) - 1] != '/')

    char buffer[TAMBUFFER];
    if (argc == 3)
    {

        //mi_dir(args[2], buffer, , ' ');
    } 
    else if (argc == 4 && strcmp(args[1], "-l"))
    {
        //mi_dir(args[2], buffer, '', 'l');
    }
}