#include "ficheros.h"

int main(int argc, char **args)
{
    // TODO: leer.c implementation
    unsigned int ninodo;

    if (argc != 2)
    {
        fprintf(
            stderr, 
            RED 
            "Usage: ./leer <nombre_dispositivo> <ninodo>\n" 
            RESET
        );
        return FALLO;
    }
    
    ninodo = args[1];

}