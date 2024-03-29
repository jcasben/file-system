#include "ficheros.h"

int main(int argc, char **argsv)
{
    if (argc < 4)
    {
        fprintf(
            stderr, 
            RED 
            "Invalid syntax. Usage: ./truncar <nombre_dispositivo> <ninodo>"
            "<nbytes>\n" 
            RESET
        );
        return FALLO;
    }

    if (bmount(argsv[1]) == FALLO)
    {
        fprintf(
            stderr,
            RED
            "Error: unable to mount the file system\n"
            RESET
        );
        return FALLO;
    }
    
    if (atoi(argsv[3]) == 0)
    {
        liberar_inodo(atoi(argsv[2]));
    }
    else 
    {
       mi_truncar_f(atoi(argsv[2]), atoi(argsv[3]));
    }

    struct STAT stats;
    mi_stat_f(atoi(argsv[2]), &stats);

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    
    ts = localtime(&stats.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stats.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stats.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    printf("DATOS INODO %s:\n", argsv[2]);
    printf("tipo = %c\n", stats.tipo);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nlinks: %d\n", stats.nlinks);
    printf("tamEnBytLog=%d\n", stats.tamEnBytesLog);
    printf("numBloquesOcupados=%d\n", stats.numBloquesOcupados);

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