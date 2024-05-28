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
    }

    if (bmount(args[1]) < 0) return FALLO;

    char file[TAMNOMBRE+20];
    strcpy(file, args[2]);
    struct STAT stat;
    if(mi_stat(file, &stat)< 0 ) return FALLO;

    int num_entradas = stat.tamEnBytesLog/ sizeof(struct entrada);
    if(num_entradas != NUMPROCESOS) return FALLO;

    char ruta[128];
    sprintf(ruta, "%sinforme.txt", file);
    if((mi_creat(ruta, 7)) < 0) return FALLO;

    struct entrada entradas[num_entradas];
    mi_read(ruta, entradas, 0, num_entradas * sizeof (struct entrada));

    unsigned int index = 0;
    struct INFORMACION info;
    struct entrada directorio;

    unsigned int escrituras_validadas = 0;
    int cant_registros_buffer_escrituras = 256;
    struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
    memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
    struct REGISTRO registro_actual;
    while (index < num_entradas)
    {
        directorio = entradas[index];
        sprintf(file, "%s%sprueba.dat", file, directorio.nombre);
        info.pid = atoi(strchr(directorio.nombre, '_') + 1);

        unsigned int blocks = 0;
        while (mi_read(file, buffer_escrituras, blocks * sizeof(buffer_escrituras), sizeof(buffer_escrituras)) > 0)
        {
            for (int i = 0; i < cant_registros_buffer_escrituras; ++i) {
                memcpy(&registro_actual, &buffer_escrituras[i], sizeof(struct REGISTRO));
                if(registro_actual.pid == info.pid)
                {
                    if(escrituras_validadas == 0)
                    {
                        // Actualizar info por primera escritura
                        info.PrimeraEscritura = registro_actual;
                        info.UltimaEscritura = registro_actual;
                        info.MenorPosicion = registro_actual;
                        info.MayorPosicion = registro_actual;
                    }
                    else
                    {
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
            info.nEscrituras = info.nEscrituras + escrituras_validadas;
            
            escrituras_validadas = 0;
            blocks++;
        }
        // Obtener la escritura de la última posición.
        // Añadir la información del struct info al fichero informe.txt por el final.
        char buffer_info[BLOCKSIZE];
        sprintf(buffer_info,
            "PID: %d\n"
            "Numero de escrituras: %d\n"
            "Primera escirtura \t%d\t%d\t%s\n"
            "Ultima escritura \t%d\t%d\t%s\n"
            "Menor posicion \t%d\t%d\t%s\n"
            "Mayor posicion \t%d\t%d\t%s\n",
            info.pid,
            escrituras_validadas,
            info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, asctime(localtime(&info.PrimeraEscritura.fecha)),
            info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, asctime(localtime(&info.UltimaEscritura.fecha)),
            info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, asctime(localtime(&info.MenorPosicion.fecha)),
            info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, asctime(localtime(&info.MayorPosicion.fecha)));

        // Escribir buffer_info en el fichero informe.txt
        mi_write(ruta, buffer_info, index * sizeof(buffer_info), sizeof(buffer_info));
        
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        #if DEBUGN13
        fprintf(stderr,
                "[%d) %d escrituras validadas en %s",
                index,
                escrituras_validadas,
                file
                );
        #endif
        index++;
    }

    if(bumount() < 0) return FALLO;

    return EXITO;
}