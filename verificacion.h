/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "simulacion.h"

#define LINESIZE 120
#define BUFFERSIZE (6*LINESIZE)

struct INFORMACION {
    int pid;
    unsigned int nEscrituras; //validadas
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};

int read_subdirectories(const char *camino, struct entrada *entradas, int *num_entradas);