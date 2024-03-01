#include "ficheros.h"

int main(int argc, char **args)
{
    // TODO: escribir.c implementation
    unsigned int offsets = {9000, 209000, 30725000, 409605000, 480000000};
    unsigned int ninodes;

    if (argc < 3)
    {
        fprintf(
            stderr, 
            RED 
            "Too few arguments. Usage: ./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n" 
            RESET
        );

        if (bmount(args[1]) == FALLO) return FALLO;
        
        if (args[3] == 0)
        {
            reservar_inodo('f', 6);
        }

        if (bumount() == FALLO) return FALLO;
    }
}