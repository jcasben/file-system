#include "directorios.h"

int main(int argc, char **args)
{
    if (argc < 4)
    {
        fprintf(
            stderr,
            RED
            "Syntax error: ./mi_mkdir <disco> <permisos> </ruta/>\n"
            RESET
        );
        return FALLO;
    }

    
}