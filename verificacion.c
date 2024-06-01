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

    char file[strlen(args[2])];
    strcpy(file, args[2]);
    struct STAT stat;
    if(mi_stat(file, &stat) < 0) return FALLO;

    int num_entradas = stat.tamEnBytesLog / sizeof(struct entrada);
    if(num_entradas != NUMPROCESOS)
    {
        fprintf(
            stderr,
            RED
            "ERROR: incorrect number of subdirectories\n"
            RESET
        );
        bumount();
        exit(FALLO);
    }

    char informe[strlen(file) + strlen("informe.txt")];
    sprintf(informe, "%sinforme.txt", file);
    if((mi_creat(informe, 7)) < 0)
    {
        bumount();
        exit(FALLO);
    }

    struct entrada entradas[NUMPROCESOS];
    mi_read(file, entradas, 0, sizeof (entradas));

    struct INFORMACION info;
    int offset = 0;

    for (int i = 0; i < num_entradas; i++)
    {
        info.pid = atoi(strchr(entradas[i].nombre, '_') + 1);
        char test[strlen(file) + sizeof(entradas[i].nombre) + strlen("prueba.dat")];
        sprintf(test, "%s%s/%s", file, entradas[i].nombre, "prueba.dat");

        unsigned int inner_offset = 0;
        info.nEscrituras = 0;

        int cant_reg = 256;
        struct REGISTRO buffer_lec[cant_reg];
        memset(buffer_lec, 0, sizeof(buffer_lec));

        int nReg;
        while (mi_read(test, buffer_lec, inner_offset, sizeof(buffer_lec)) > 0)
        {
            for (nReg = 0; nReg < cant_reg && info.nEscrituras < NUMESCRITURAS; nReg++)
            {
                if (buffer_lec[nReg].pid != info.pid) continue;
                if (info.nEscrituras == 0) {
                    info.PrimeraEscritura = buffer_lec[nReg];
                    info.UltimaEscritura = buffer_lec[nReg];
                    info.MenorPosicion = buffer_lec[nReg];
                } else {
                    if (buffer_lec[nReg].nEscritura < info.PrimeraEscritura.nEscritura)
                        info.PrimeraEscritura = buffer_lec[nReg];
                    if (buffer_lec[nReg].nEscritura > info.UltimaEscritura.nEscritura)
                        info.UltimaEscritura = buffer_lec[nReg];
                }
                info.MayorPosicion = buffer_lec[nReg];
                info.nEscrituras++;
            }

            inner_offset += sizeof(buffer_lec);
            memset(buffer_lec, 0, sizeof(buffer_lec));
        }

        char line[LINESIZE];
        memset(line, 0, LINESIZE);
        char info_buffer[BUFFERSIZE];
        memset(info_buffer, 0, BUFFERSIZE);

        sprintf(line, "\nPID: %i\nNumero de escrituras: %d\n", info.pid, info.nEscrituras);
        strcat(info_buffer, line);

        struct tm *tm;
        char aux[80];
        tm = localtime(&info.PrimeraEscritura.fecha.tv_sec);
        strftime(aux, sizeof(aux), "%Y-%m-%d %H:%M:%S", tm);
        sprintf(
            line,
            "%s\t %i\t %i\t %s.%06ld\n",
            "Primera Escritura",
            info.PrimeraEscritura.nEscritura,
            info.PrimeraEscritura.nRegistro,
            aux,
            info.PrimeraEscritura.fecha.tv_usec
        );
        strcat(info_buffer, line);

        tm = localtime(&info.UltimaEscritura.fecha.tv_sec);
        strftime(aux, sizeof(aux), "%Y-%m-%d %H:%M:%S", tm);
        sprintf(line, "%s\t %i\t %i\t %s.%06ld\n",
                "Ultima Escritura",
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                aux, info.UltimaEscritura.fecha.tv_usec);
        strcat(info_buffer, line);

        tm = localtime(&info.MenorPosicion.fecha.tv_sec);
        strftime(aux, sizeof(aux), "%Y-%m-%d %H:%M:%S", tm);
        sprintf(
            line,
            "%s\t\t %i\t %i\t %s.%06ld\n",
            "Menor Posición",
            info.MenorPosicion.nEscritura,
            info.MenorPosicion.nRegistro,
            aux, info.MenorPosicion.fecha.tv_usec
        );
        strcat(info_buffer, line);

        tm = localtime(&info.MayorPosicion.fecha.tv_sec);
        strftime(aux, sizeof(aux), "%Y-%m-%d %H:%M:%S", tm);
        sprintf(
            line,
            "%s\t\t %i\t %i\t %s.%06ld\n",
            "Mayor Posición",
            info.MayorPosicion.nEscritura,
            info.MayorPosicion.nRegistro,
            aux, info.MayorPosicion.fecha.tv_usec
        );
        strcat(info_buffer, line);

        if (mi_write(informe, info_buffer, offset, strlen(info_buffer)) == FALLO) return FALLO;
        offset += strlen(info_buffer);
    }

    if(bumount() < 0) return FALLO;

    return EXITO;
}