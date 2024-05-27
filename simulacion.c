#include "simulacion.h"

int acabados = 0;

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
    char nombre_directorio[32];
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

            char nombre_directorio_hijo[64];
            memset(nombre_directorio_hijo, 0, sizeof(nombre_directorio_hijo));
            sprintf(nombre_directorio_hijo, "%sproceso_%d/", nombre_directorio, getpid());
            printf("primer %s\n", nombre_directorio_hijo);
            if(mi_creat(nombre_directorio_hijo, 7) < 0) return FALLO;

            char ruta_fichero[128];
            memset(ruta_fichero, 0, sizeof(ruta_fichero));
            sprintf(ruta_fichero, "%sprueba.dat", nombre_directorio_hijo);
            printf("segundo %s\n", ruta_fichero);
            if(mi_creat(ruta_fichero, 6) < 0) return FALLO;

            srand(time(NULL) + getpid());
            for (int nscritura = 1; nscritura < NUMESCRITURAS; nscritura++) {
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nscritura;
                registro.nRegistro = rand() % REGMAX;

                if(bwrite(registro.nRegistro, &registro) < 0) return FALLO;
            }
            my_sleep(50);

            if(bumount() < 0) return FALLO;
            exit(0);
        }
        my_sleep(150);
    }

    while (acabados < NUMPROCESOS) {
        pause();
    }

    if (bumount() < 0) return FALLO;
    return EXITO;
}

void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}

void my_sleep(unsigned msec) { //recibe tiempo en milisegundos
    struct timespec req, rem;
    int err;
    req.tv_sec = msec / 1000; //conversión a segundos
    req.tv_nsec = (msec % 1000) * 1000000; //conversión a nanosegundos
    while ((req.tv_sec != 0) || (req.tv_nsec != 0)) {
        if (nanosleep(&req, &rem) == 0)
            // rem almacena el tiempo restante si una llamada al sistema
            // ha sido interrumpida por una señal
            break;
        err = errno;
        // Interrupted; continue
        if (err == EINTR) {
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
    }
}
