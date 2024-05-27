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
    if(mi_stat(args[2], &stat)< 0 ) return FALLO;

    int num_entradas = stat.tamEnBytesLog/ sizeof(struct entrada);
    if(num_entradas != NUMPROCESOS) return FALLO;

    char ruta[128];
    sprintf(ruta, "%sinforme.txt", args[2]);
    if((mi_creat(ruta, 7)) < 0) return FALLO;

    unsigned int escrituras_validadas = 0;
    struct entrada entradas[num_entradas];
    mi_read(ruta, entradas, 0, num_entradas * sizeof (struct entrada));

    unsigned int index = 0;
    struct INFORMACION info;
    struct entrada directorio;

    int cant_registros_buffer_escrituras = 256;
    struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
    memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
    struct REGISTRO registro_actual;
    char fichero[TAMNOMBRE+20];
    while (index < num_entradas)
    {
        memset(fichero, 0, strlen(fichero));
        directorio = entradas[index];
        strcpy(fichero, directorio.nombre);
        info.pid = atoi(strchr(directorio.nombre, '_'));

        strcat(fichero, "prueba.dat");
        while (mi_read(fichero, buffer_escrituras, 0, sizeof(buffer_escrituras)) > 0)
        {
            for (int i = 0; i < cant_registros_buffer_escrituras; ++i) {
                memcpy(&registro_actual, &buffer_escrituras[i], sizeof(struct REGISTRO));
                if(registro_actual.pid == info.pid)
                {
                    if(escrituras_validadas == 0)
                    {
                        // Actualizar info por primera escritura
                        info.PrimeraEscritura;
                    }
                    else
                    {
                        //Comparar nº de escritura (para obtener primera y última)
                        // y actualizarlas si es preciso
                    }
                    escrituras_validadas++;
                }
            }
        }
        // Obtener la escritura de la última posición.
        // Añadir la información del struct info al fichero informe.txt por el final.

        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        index++;
    }
    

    if(bumount() < 0) return FALLO;

    return EXITO;
}