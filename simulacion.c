/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "simulacion.h"

static int acabados = 0;
char nombre_directorio[88];

int main(int argc, char **args) {
    if (argc != 2)
    {
        fprintf(
                stderr,
                RED
                "ERROR: invalid syntax. Usage: ./simulacion <disco>\n"
                RESET
        );
        return FALLO;
    }

    if (bmount(args[1]) < 0) return FALLO;

    signal(SIGCHLD, reaper);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *t = localtime(&tv.tv_sec);

    sprintf(nombre_directorio, "/simul_%04d%02d%02d%02d%02d%02d/",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    if (mi_creat(nombre_directorio, 7) < 0) return FALLO;

    for (int i = 0; i < NUMPROCESOS; i++) {
        pid_t pid = fork();
        if (pid == 0)
        {
            if(bmount(args[1]) < 0) return FALLO;

            char nombre_directorio_hijo[128];
            memset(nombre_directorio_hijo, 0, sizeof(nombre_directorio_hijo));
            sprintf(nombre_directorio_hijo, "%sproceso_%d/", nombre_directorio, getpid());
            if(mi_creat(nombre_directorio_hijo, 7) < 0) return FALLO;

            char ruta_fichero[256];
            memset(ruta_fichero, 0, sizeof(ruta_fichero));
            sprintf(ruta_fichero, "%sprueba.dat", nombre_directorio_hijo);
            if(mi_creat(ruta_fichero, 6) < 0) return FALLO;

            srand(time(NULL) + getpid());
            for (int nscritura = 1; nscritura <= NUMESCRITURAS; nscritura++)
            {
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                fprintf(stdin, BLUE"%s"RESET, asctime((const struct tm *) registro.fecha));
                //printf("%s\n", asctime((const struct tm *) registro.fecha));

                registro.pid = getpid();
                registro.nEscritura = nscritura;
                registro.nRegistro = rand() % REGMAX;

                mi_write(ruta_fichero, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                usleep(50000);
            }
            if(bumount() < 0) return FALLO;
            exit(0);
        }
        usleep(150000);
    }

    while (acabados < NUMPROCESOS) pause();

    if (bumount() < 0) return FALLO;
    return EXITO;
}

void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG) > 0))
    {
        acabados++;
        #if DEBUGN12
            fprintf(
                stderr,
                GRAY
                "[Proceso %d: Completadas 50 escrituras en %sproceso_%d/prueba.dat]\n"
                RESET,
                acabados, nombre_directorio, ended
            );
        #endif
    }
}