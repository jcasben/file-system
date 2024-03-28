#include "ficheros.h"

#define TAMBUFFER (BLOCKSIZE * 4 + 150)
//#define TAMBUFFER (BLOCKSIZE * 4);

int main(int argc, char **args)
{
    unsigned int ninodo;
    int read_bytes;
    int total_read_bytes = 0;

    if (argc < 3)
    {
        fprintf(
            stderr, 
            RED 
            "Invalid syntax. Usage: ./leer <nombre_dispositivo> <ninodo>\n" 
            RESET
        );
        return FALLO;
    }
    // Mounting the device
    if (bmount(args[1]) == FALLO)
    {
        fprintf(
            stderr, 
            RED 
            "Error: unable to mount the file system\n" 
            RESET
        );
        return FALLO;
    }

    if ((ninodo = atoi(args[2])) < 0)
    {
        fprintf(
            stderr, 
            RED 
            "Error: invalid inode number\n" 
            RESET
        );
        return FALLO;
    }

    char text_buffer[TAMBUFFER];

    memset(text_buffer, 0, TAMBUFFER);

    int offset = 0;
    if ((read_bytes = mi_read_f(ninodo, text_buffer, offset, TAMBUFFER)) == FALLO) return FALLO;
    while (read_bytes > 0){
        total_read_bytes += read_bytes;
        write(1, text_buffer, read_bytes);
        memset(text_buffer, 0, TAMBUFFER);
        offset += TAMBUFFER;
        if ((read_bytes = mi_read_f(ninodo, text_buffer, offset, TAMBUFFER)) == FALLO) return FALLO;
    }
    
    struct STAT stat;
    mi_stat_f(ninodo, &stat);
    if (total_read_bytes != stat.tamEnBytesLog)
    {
        fprintf(
            stderr, 
            RED 
            "Error: the number of bytes read does not match the file size\n" 
            RESET
        );
        return FALLO;
    }
    else
    {
        fprintf(
            stderr,
            "\nNumber of bytes read = %d\n"
            "Size in logical bytes = %d\n", 
            total_read_bytes,
            stat.tamEnBytesLog
        );
    }

    // Unmounting the device
    if (bumount() == FALLO)
    {
        fprintf(
            stderr, 
            RED 
            "Error: unable to unmount the file system\n" 
            RESET
        );
        return FALLO;
    }
}