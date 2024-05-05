#include "directorios.h"

int main(int argc, char **args)
{
    if(argc != 3)
    {
        fprintf(stderr, "Sintaxis: ./mi_stat <disco> </ruta>\n");
        return FALLO;
    }

    if (bmount(args[1]) < 0) return FALLO;

    struct STAT p_stat;
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    if(mi_stat(args[2], &p_stat) < 0) return FALLO;

//    printf("Nº de inodo: %d\n", p_stat.);
    printf("tipo: %c\n", p_stat.tipo);
    printf("permisos: %d\n", p_stat.permisos);

    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);

    printf("nlinks: %d\n", p_stat.nlinks);
    printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);

    if (bumount() < 0) return FALLO;
}