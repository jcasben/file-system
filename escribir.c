#include "ficheros.h"

int main(int argc, char **args)
{
    // TODO: escribir.c implementation, not finished.
    int noffsets = 5;
    unsigned int offsets[] = {9000, 209000, 30725000, 409605000, 480000000};
    unsigned int n_first_inode;

    if (argc < 3)
    {
        fprintf(
            stderr, 
            RED 
            "Too few arguments. Usage: ./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n" 
            RESET
        );
        return FALLO;
    }

    // Mount the device.
    if (bmount(args[1]) == FALLO) return FALLO;
    
    // Reserve only one inode for all the offsets.
    if (args[3] == 0)
    {
        n_first_inode = reservar_inodo('f', 6);
    }
    // Reserve one inode for each offset.
    else if (args[3] == 1)
    {
        for (size_t i = 0; i < noffsets; i++)
        {
            n_first_inode = reservar_inodo('f', 6);
        }
    }
    else
    {
        fprintf(
            stderr,
            RED 
            "Invalid argument. The third argument must be 0 or 1\n"
            RESET
        );
        return FALLO;
    }

    // Write the content of args[2] to the specified inodes.
    if (args[3] == 0)
    {
        for (size_t i = 0; i < noffsets; i++)
        {
            mi_write_f(n_first_inode, args[2], offsets[i], strlen(args[2]));
        }
    }
    // args[3] == 1
    else
    {
        for (size_t i = 0; i < noffsets; i++)
        {
            mi_write_f(n_first_inode + i, *args[2], offsets[i], strlen(args[2]));
        }
    }



    if (bumount() == FALLO) return FALLO;
}