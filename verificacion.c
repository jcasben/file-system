/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

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
        return FALLO;
    }

    if (bmount(args[1]) < 0) return FALLO;

    char file[TAMNOMBRE+20];
    strcpy(file, args[2]);
    struct STAT stat;
    if(mi_stat(file, &stat) < 0) return FALLO;

    int num_entradas = stat.tamEnBytesLog / sizeof(struct entrada);
    if(num_entradas != NUMPROCESOS) return FALLO;

    char ruta[128];
    sprintf(ruta, "%sinforme.txt", file);
    if((mi_creat(ruta, 7)) < 0) return FALLO;

    struct entrada entradas[num_entradas];
    mi_read(file, entradas, 0, num_entradas * sizeof (struct entrada));

    unsigned int index = 0;
    struct entrada directorio;

    unsigned int escrituras_validadas;
    int cant_registros_buffer_escrituras = 256;
    struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];

    struct REGISTRO registro_actual;
    while (index < num_entradas)
    {
        struct INFORMACION info;
        memset(&info, 0, sizeof(struct INFORMACION));
        char file_buffer[256];
        memset(file_buffer, 0, sizeof(file_buffer));

        directorio = entradas[index];
        sprintf(file_buffer, "%s%s/prueba.dat", file, directorio.nombre);
        info.pid = (pid_t) atoi(strchr(directorio.nombre, '_') + 1);
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));

        escrituras_validadas = 0;
        unsigned int blocks = 0;
        while (mi_read(file_buffer, buffer_escrituras, blocks * sizeof(buffer_escrituras), sizeof(buffer_escrituras)) > 0)
        {
            for (int i = 0; i < cant_registros_buffer_escrituras; ++i) {
                memcpy(&registro_actual, &buffer_escrituras[i], sizeof(struct REGISTRO));
                if(registro_actual.pid == info.pid)
                {
                    if(escrituras_validadas == 0)
                    {
                        // Actualizar info la primera vez
                        info.PrimeraEscritura = registro_actual;
                        info.UltimaEscritura = registro_actual;
                        info.MenorPosicion = registro_actual;
                        info.MayorPosicion = registro_actual;
                    }
                    else
                    {
                        if(info.PrimeraEscritura.nEscritura > registro_actual.nEscritura)
                        {
                            info.PrimeraEscritura = registro_actual;
                        }
                        //Comparar nº de escritura (para obtener primera y última)
                        // y actualizarlas si es preciso
                        if (info.UltimaEscritura.nEscritura < registro_actual.nEscritura)
                        {
                            info.UltimaEscritura = registro_actual;
                        }
                        if (info.MayorPosicion.nRegistro < registro_actual.nRegistro)
                        {
                            info.MayorPosicion = registro_actual;
                        }
                    }
                    escrituras_validadas++;
                }
            }
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
            blocks++;
        }

        // Obtener la escritura de la última posición.
        // Añadir la información del struct info al fichero informe.txt por el final.
        char buffer_info[490];
        memset(buffer_info, 0, sizeof(buffer_info));

        // Buscado en ChatGPT como poner la fecha con milisegundos
        char aux1[30];
        char aux2[30];
        char aux3[30];
        char aux4[30];
        struct tm *tm_info1;
        struct tm *tm_info2;
        struct tm *tm_info3;
        struct tm *tm_info4;

        tm_info1 = localtime(&info.PrimeraEscritura.fecha.tv_sec);
        tm_info2 = localtime(&info.UltimaEscritura.fecha.tv_sec);
        tm_info3 = localtime(&info.MenorPosicion.fecha.tv_sec);
        tm_info4 = localtime(&info.MayorPosicion.fecha.tv_sec);

        strftime(aux1, 30, "%Y-%m-%d %H:%M:%S", tm_info1);
        strftime(aux2, 30, "%Y-%m-%d %H:%M:%S", tm_info2);
        strftime(aux3, 30, "%Y-%m-%d %H:%M:%S", tm_info3);
        strftime(aux4, 30, "%Y-%m-%d %H:%M:%S", tm_info4);


        sprintf(buffer_info,
            "PID: %d\n"
            "Numero de escrituras: %d\n"
            "Primera escritura \t%d\t%d\t%s.%06ld\n"
            "Ultima escritura  \t%d\t%d\t%s.%06ld\n"
            "Menor posicion    \t%d\t%d\t%s.%06ld\n"
            "Mayor posicion    \t%d\t%d\t%s.%06ld\n\n",
            info.pid,
            escrituras_validadas,
            info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, aux1, info.PrimeraEscritura.fecha.tv_usec,
            info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, aux2, info.UltimaEscritura.fecha.tv_usec,
            info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, aux3, info.MenorPosicion.fecha.tv_usec,
            info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, aux4, info.MayorPosicion.fecha.tv_usec);

        // Escribir buffer_info en el fichero informe.txt
        mi_write(ruta, buffer_info, index * sizeof(buffer_info), sizeof(buffer_info));

        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        #if DEBUGN13
        fprintf(
            stderr,
            "[%d) %d escrituras validadas en %s\n",
            index,
            escrituras_validadas,
            file_buffer
        );
        #endif

        index++;
    }

    if(bumount() < 0) return FALLO;

    return EXITO;
}