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
    char buffer[TAMBUFFER];
    if (argc == 3)
    {
        //mi_dir(args[2], buffer, _, 0);
    } 
    else if (argc == 4 && strcmp(args[1], "-l"))
    {
        //mi_dir(args[2], buffer, _, 1);
    }
}