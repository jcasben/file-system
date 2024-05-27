#include "simulacion.h"

struct INFORMACION {
    int pid;
    unsigned int nEscrituras; //validadas
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};

int read_subdirectories(const char *camino, struct entrada *entradas, int *num_entradas);