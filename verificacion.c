#include "verificacion.h"

int main(int argc, char **args)
{
    if (argc != 3)
    {
        fprintf(
            stderr,
            RED
            "ERROR: invalid syntax. Usage: ./verificacion <nombre_dispositivo> <directorio_simulación>\n"
            RESET
        );
    }

    if (bmount(args[1]) < 0) return FALLO;

    struct STAT stat;
    mi_stat(args[2], &stat);

    int num_entradas = stat.tamEnBytesLog/ sizeof(struct entrada);
    if(num_entradas != NUMPROCESOS) return FALLO;

    char ruta[128];
    sprintf(ruta, "%sinforme.txt", args[2]);
    if((mi_creat(ruta, 7)) < 0) return FALLO;


    if(bumount() < 0) return FALLO;

    return EXITO;
}