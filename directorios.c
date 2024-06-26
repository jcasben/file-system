/*
    Autores:
        - Marc Link Cladera
        - Carlos Gálvez Mena
        - Jesús Castillo Benito
*/

#include "directorios.h"

#if USARCACHE==1
    static struct UltimaEntrada UltimaEntradaEscritura;
    static struct UltimaEntrada UltimaEntradaLectura;
#endif

#if USARCACHE==2
    static struct CacheFIFO writeCache;
    static struct CacheFIFO readCache;
    static char initWriteCache = 0;
    static char initReadCache = 0;
#endif
#if USARCACHE==3
    static struct CacheLRU writeCache;
    static struct CacheLRU readCache;
    static char initWriteCache = 0;
    static char initReadCache = 0;
#endif

//----------------------------- NIVEL 7 (11/04/2023 - 01/05/2024) -----------------------------

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    //If the path is null or if path doesn't start with '/'
    if (camino == NULL || inicial == NULL || final == NULL || tipo == NULL || *camino != '/') 
    {
        return FALLO;
    }

    char *aux = (char *)camino + 1;
    char *barPos = strchr(aux, '/');

    if(barPos == NULL)
    {
        strcpy(inicial, aux);
        memset(final, 0, strlen(final));
        *tipo = 'f';
    }
    else
    {
        strcpy(final, barPos);
        const size_t aux_len = strlen(aux);
        const size_t final_len = strlen(final);
        strncpy(inicial, aux, aux_len - final_len);
        inicial[aux_len - final_len] = '\0';
        *tipo = 'd';
    }

    return EXITO;
}

int buscar_entrada(
    const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo,
    unsigned int *p_entrada, char reservar, unsigned char permisos
)
{
    struct entrada entry;
    struct inodo inode;
    char beginning[sizeof(entry.nombre)];
    char end[strlen(camino_parcial)];
    char type;

    if (strcmp("/", camino_parcial) == 0)
    {
        struct superbloque SB;
        if (bread(posSB, &SB) == FALLO) return FALLO;
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;

        return 0;
    }

    if (extraer_camino(camino_parcial, beginning, end, &type) == FALLO)
        return ERROR_CAMINO_INCORRECTO;
#if DEBUGN7
    fprintf(
            stderr,
            GRAY
            "[buscar_entrada() -> inicial: %s, final: %s, reservar: %d]\n"
            RESET,
            beginning, end, reservar
    );
#endif

    if (leer_inodo(*p_inodo_dir, &inode) == FALLO) return FALLO;
    if ((inode.permisos & 4) != 4) return ERROR_PERMISO_LECTURA;

    //Calculate the number of entrys in the inode
    int cant_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada);

    int offset = 0;
    if (cant_entries_inode > 0)
    {
        struct entrada buffer_lec[BLOCKSIZE/sizeof(struct entrada)];

        memset(buffer_lec, 0, BLOCKSIZE);

        offset = mi_read_f(*p_inodo_dir, &buffer_lec, 0, BLOCKSIZE);
        while (*p_entrada < cant_entries_inode) {
            entry = buffer_lec[*p_entrada % (BLOCKSIZE / sizeof(struct entrada))];

            if (strcmp(beginning, entry.nombre) == 0) break;

            *p_entrada = *p_entrada + 1;

            if (*p_entrada % (BLOCKSIZE / sizeof(struct entrada)) == 0) {
                memset(buffer_lec, 0, BLOCKSIZE);
                offset += mi_read_f(*p_inodo_dir, &buffer_lec, offset, BLOCKSIZE);

            }
        }
    }

    if (strcmp(beginning, entry.nombre) != 0 && *p_entrada == cant_entries_inode)
    {
        switch (reservar)
        {
            case 0:
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

            case 1:

                //Create entry in the directory refered by *p_inodo_dir
                //If it's a file, don't allow writing
                if(inode.tipo == 'f')
                {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }

                if((inode.permisos & 2) == 0)
                {
                    return ERROR_PERMISO_ESCRITURA;
                }

                strcpy(entry.nombre, beginning);
                if(type == 'd')
                {
                    if (strcmp(end, "/") == 0)
                    {
                        if ((entry.ninodo = reservar_inodo('d', permisos)) == FALLO) return FALLO;
#if DEBUGN7
                        fprintf(
                            stderr,
                            GRAY
                            "[buscar_entrada() -> reservado inodo %d tipo %c con permisos %d para %s]\n"
                            RESET,
                            buffer_lec[index].ninodo, type, permisos, beginning
                    );
#endif
                    }
                    else
                    {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                {
                    if ((entry.ninodo = reservar_inodo('f', permisos)) == FALLO) return FALLO;
#if DEBUGN7
                    fprintf(
                        stderr,
                        GRAY
                        "[buscar_entrada() -> reservado inodo %d tipo %c con permisos %d para %s]\n"
                        RESET,
                        buffer_lec[index].ninodo, type, permisos, beginning
                );
#endif
                }
                //Write entry in the father directory
                if(mi_write_f(
                        *p_inodo_dir,
                        &entry,
                        offset,
                        sizeof(struct entrada)
                ) == FALLO)
                {
                    if (entry.ninodo != FALLO)
                    {
                        if (liberar_inodo(entry.ninodo) == FALLO) return FALLO;
                    }
                    return FALLO;
                }
#if DEBUGN7
                fprintf(
                        stderr,
                        GRAY
                        "[buscar_entrada() -> creada entrada: %s, %d]\n"
                        RESET,
                        buffer_lec[index].nombre, buffer_lec[index].ninodo
                );
#endif

                break;
            default:
                return FALLO;
        }
    }

    if(strcmp(end, "/") == 0 || strcmp(end, "") == 0)
    {
        if((*p_entrada < cant_entries_inode) && (reservar == 1)) return ERROR_ENTRADA_YA_EXISTENTE;

        *p_inodo = entry.ninodo;
        return EXITO;
    }
    *p_inodo_dir = entry.ninodo;
    *p_inodo = 0;
    *p_entrada = 0;

    return buscar_entrada(end, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}

void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
        case -2: fprintf(stderr, RED "ERROR: incorrect path\n" RESET); break;
        case -3: fprintf(stderr, RED "ERROR: denied reading permission\n" RESET); break;
        case -4: fprintf(stderr, RED "ERROR: no such file or directory\n" RESET); break;
        case -5: fprintf(stderr, RED "ERROR: no such intermediate directory\n" RESET); break;
        case -6: fprintf(stderr, RED "ERROR: denied writing permission\n" RESET); break;
        case -7: fprintf(stderr, RED "ERROR: file already exists\n" RESET); break;
        case -8: fprintf(stderr, RED "ERROR: it's not a direcotory\n" RESET); break;
    }
}

//----------------------------- NIVEL 8 (25/04/2023 - 05/05/2024) -----------------------------

int mi_creat(const char *camino, unsigned char permisos)
{
    mi_waitSem();
    struct superbloque SB;
    bread(posSB, &SB);
    unsigned int p_inode_dir = SB.posInodoRaiz;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;

    int error = buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 1, permisos);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();

        return FALLO;
    }
    mi_signalSem();

    return EXITO;
}

int mi_dir(const char *camino, char *buffer, char tipo, char flag)
{
    struct superbloque SB;
    bread(posSB, &SB);

    unsigned int p_inode_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    int error = buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    // Read inode with the assigned value of the function buscar_entrada()
    struct inodo inode;
    if (leer_inodo(n_inode, &inode) == FALLO) return FALLO;
    unsigned long cant_entries_inode = inode.tamEnBytesLog / sizeof(struct entrada);
    if (inode.tipo != tipo)
    {
        fprintf(stderr, RED "ERROR: the type of the file doesn't correspond with the path\n" RESET);
        return FALLO;
    }

    if (tipo == 'd' && cant_entries_inode > 0)
    {
        printf("Total: %lu\n", cant_entries_inode);
        if (flag == 'l')
        {
            printf("\nTYPE\tPERMISSIONS\tmTIME\t\t\tSIZE\tNAME\n"
                       "---------------------------------------------------------------------\n");
        }
        int entry_inode_number = 0;

        struct entrada buffer_lec[BLOCKSIZE/sizeof(struct entrada)];
        int offset = mi_read_f(n_inode, buffer_lec, 0, BLOCKSIZE);
        if(offset == FALLO) return FALLO;
        int index = 0;

        while (entry_inode_number < cant_entries_inode) {
            //Leer el inodo correspndiente
            if (leer_inodo(buffer_lec[index].ninodo, &inode) < 0) return FALLO;

            // Load the line into the buffer
            if (flag == 'l') build_extended_buffer(buffer_lec[index], buffer);
            else build_buffer(buffer_lec[index], buffer);

            index++;
            if (index == BLOCKSIZE / sizeof(struct entrada)) {
                offset += mi_read_f(n_inode, buffer_lec, offset, BLOCKSIZE);
                index = 0;
            }
            entry_inode_number++;
        }
    } else if (tipo == 'f')
    {
        struct entrada entry;
        if (mi_read_f(p_inode_dir,&entry,p_entry * sizeof(struct entrada),sizeof(struct entrada)) == FALLO) return FALLO;

        if (flag == 'l')
        {
            printf("\nTYPE\tPERMISSIONS\tmTIME\t\t\tSIZE\tNAME\n"
                   "---------------------------------------------------------------------\n");
            build_extended_buffer(entry, buffer);
        }
        else build_buffer(entry, buffer);
    }

    return EXITO;
}

int build_buffer(struct entrada entry, char *buffer)
{
    struct inodo inode;
    leer_inodo(entry.ninodo, &inode);
    char tmp[sizeof(entry.nombre) + 20];
    char *aux;

    if (inode.tipo == 'd') sprintf(tmp, NEGRITA BLUE "%s\t" RESET, entry.nombre);
    else sprintf(tmp, "%s\t", entry.nombre);

    // Check if we have to create a new line
    size_t num_rows = strlen(buffer) / TAMFILA;
    aux = buffer + (num_rows * TAMFILA);
    if (strlen(aux + strlen(tmp)) >= TAMFILA + 20) strcat(buffer, "\n");

    strcat(buffer, tmp);

    return EXITO;
}

int build_extended_buffer(struct entrada entry, char *buffer)
{
    struct inodo inode;
    leer_inodo(entry.ninodo, &inode);
    char type[2];
    strcpy(type, (char[2]) { (char) inode.tipo, '\0' });

    // Entry type
    strcat(buffer, type);
    strcat(buffer, "\t");
    // Entry permissions
    if (inode.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
    if (inode.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
    if (inode.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
    strcat(buffer, "\t");
    strcat(buffer, "\t");
    // Entry mTIME
    char tmp[20];
    struct tm *tm; //ver info: struct tm
    tm = localtime(&inode.mtime);
    sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
    strcat(buffer, tmp);
    strcat(buffer, "\t");
    // Entry size
    char tmp2[20];
    sprintf(tmp2, "%u", inode.tamEnBytesLog);
    strcat(buffer, tmp2);
    strcat(buffer, "\t");
    // Entry name
    char name[80];
    if (inode.tipo == 'd') sprintf(name, NEGRITA BLUE "%s" RESET, entry.nombre);
    else sprintf(name, "%s", entry.nombre);
    strcat(buffer, name);
    strcat(buffer, "\n");

    return EXITO;
}

int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    return mi_chmod_f(p_inodo, permisos);
}

int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    if (mi_stat_f(p_inodo, p_stat) == FALLO)
    {
        fprintf(stderr, RED "ERROR: statistics could not be obtained" RESET);
        return FALLO;
    }

    return p_inodo;
}

//----------------------------- NIVEL 9 (02/05/2023 - 13/05/2024) -----------------------------

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    mi_waitSem();

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    #if USARCACHE==0
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
        if (error  < 0)
        {
            mostrar_error_buscar_entrada(error);
            mi_signalSem();
            return FALLO;
        }

    #endif
    #if USARCACHE==1
        if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
        {
            p_inodo = UltimaEntradaEscritura.p_inodo;
                #if DEBUGN9
                fprintf(
                    stderr, 
                    ORANGE 
                    "[mi_write() → Utilizamos última escritura: %s]\n" 
                    RESET, 
                    UltimaEntradaEscritura.camino
                );
            #endif

        } else
        {
            int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
            if (error  < 0) mostrar_error_buscar_entrada(error);
            strcpy(UltimaEntradaEscritura.camino, camino);
            UltimaEntradaEscritura.p_inodo = p_inodo;
             #if DEBUGN9
                fprintf(
                    stderr, 
                    ORANGE 
                    "[mi_write() → Actualizamos última escritura: %s]\n" 
                    RESET, 
                    UltimaEntradaEscritura.camino
                );
            #endif
        }
    #endif
    #if (USARCACHE==2 || USARCACHE==3)
        if (initWriteCache == 0) {
            initWriteCache = 1;
            #if USARCACHE==2
            writeCache.last = 0;
            #endif
            writeCache.size = 0;
        }
        int pos = searchEntry(camino, &writeCache);
        if(pos >= 0)
        {
            p_inodo = writeCache.lastEntries[pos].p_inodo;
            #if DEBUGN9
                fprintf(
                    stderr,
                    BLUE
                    "[mi_write() -> Utilizamos cache[%d]: %s]\n"
                    RESET,
                    pos, writeCache.lastEntries[pos].camino
                );
            #endif
        }
        else
        {
            int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
            if (error  < 0)
            {
                mostrar_error_buscar_entrada(error);
                mi_signalSem();
                return FALLO;
            }
            updateCache(&writeCache, camino, &p_inodo);
        }
    #endif

    int written_bytes;
    if ((written_bytes = mi_write_f(p_inodo, buf, offset, nbytes)) == FALLO)
    {
        mi_signalSem();
        return  0;
    }

    mi_signalSem();

    return written_bytes;
}


int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    mi_waitSem();

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    #if USARCACHE==0
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
        if (error  < 0)
        {
            mostrar_error_buscar_entrada(error);
            mi_signalSem();
            return FALLO;
        }
    #endif
    #if USARCACHE==1
        if (strcmp(UltimaEntradaLectura.camino, camino) == 0)
        {
            p_inodo = UltimaEntradaLectura.p_inodo;
            #if DEBUGN9
                    fprintf(
                        stderr,
                        BLUE
                        "\n[mi_read() -> Utilizamos última lectura: %s]\n"
                        RESET,
                        UltimaEntradaLectura.camino
                    );
            #endif
        } else {
            int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
            if (error  < 0) mostrar_error_buscar_entrada(error);
            strcpy(UltimaEntradaLectura.camino, camino);
            UltimaEntradaLectura.p_inodo = p_inodo;
            #if DEBUGN9
                fprintf(
                    stderr,
                    ORANGE
                    "[mi_read() → Actualizamos última lectura: %s]\n"
                    RESET,
                    UltimaEntradaLectura.camino
                );
            #endif
        }
    #endif
    #if (USARCACHE==2 || USARCACHE==3)
        if (initReadCache == 0) {
            initReadCache = 1;
            #if USARCACHE==2
                readCache.last = 0;
            #endif
            readCache.size = 0;
        }
        int pos = searchEntry(camino, &readCache);
        if(pos >= 0)
        {
            p_inodo = readCache.lastEntries[pos].p_inodo;
            #if DEBUGN9
                fprintf(
                    stderr,
                    BLUE
                    "[mi_read() -> Utilizamos cache[%d]: %s]\n"
                    RESET,
                    pos, readCache.lastEntries[pos].camino
                );
            #endif
        }
        else
        {
            int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
            if (error < 0)
            {
                mostrar_error_buscar_entrada(error);
                mi_signalSem();
                return FALLO;
            }
            updateCache(&readCache, camino, &p_inodo);
        }
    #endif

    int read_bytes;
    if ((read_bytes = mi_read_f(p_inodo, buf, offset, nbytes)) == FALLO)
    {
        mi_signalSem();
        return 0;
    }

    mi_signalSem();

    return read_bytes;
}

#if USARCACHE==2
    int searchEntry(const char *camino, struct CacheFIFO *cache)
    {
        for (int i = 0; i < cache->size; i++) {
            if(strcmp(cache->lastEntries[i].camino, camino) == 0)
            {
                cache->lastEntries[i].a = 1;
                return i;
            }
        }

        return -1;
    }
#endif
#if USARCACHE==3
    int searchEntry(const char *camino, struct CacheLRU *cache)
    {
        for (int i = 0; i < cache->size; i++) {
            if(strcmp(cache->lastEntries[i].camino, camino) == 0)
            {
                gettimeofday(&cache->lastEntries[i].ultima_consulta, NULL);
                return i;
            }
        }
        return -1;
    }
#endif
#if USARCACHE==2
    unsigned int updateCache(struct CacheFIFO *cache, const char *camino, const unsigned int *p_inodo)
    {
        unsigned int pos;

        if(cache->size < CACHE_SIZE)
        {
            strcpy(cache->lastEntries[cache->size].camino, camino);
            cache->lastEntries[cache->size].p_inodo = *p_inodo;
            cache->lastEntries[cache->size].a = 1;
            pos = cache->size;
            cache->size++;
            cache->last = cache->size;
        }
        else
        {
            if(cache->lastEntries[cache->last % CACHE_SIZE].a == 0)
            {
                memset(cache->lastEntries[cache->last % CACHE_SIZE].camino, 0, TAMNOMBRE*PROFUNDIDAD);
                strcpy(cache->lastEntries[cache->last % CACHE_SIZE].camino, camino);
                cache->lastEntries[cache->last % CACHE_SIZE].p_inodo = *p_inodo;
                cache->lastEntries[cache->last % CACHE_SIZE].a = 1;
                pos = cache->last % CACHE_SIZE;
                cache->last++;
            }
            else
            {
                do
                {
                    cache->lastEntries[cache->last % CACHE_SIZE].a = 0;
                    cache->last++;
                } while(cache->lastEntries[cache->last % CACHE_SIZE].a == 1);

                memset(cache->lastEntries[cache->last % CACHE_SIZE].camino, 0, TAMNOMBRE*PROFUNDIDAD);
                strcpy(cache->lastEntries[cache->last % CACHE_SIZE].camino, camino);
                cache->lastEntries[cache->last % CACHE_SIZE].p_inodo = *p_inodo;
                cache->lastEntries[cache->last % CACHE_SIZE].a = 1;
                pos = cache->last % CACHE_SIZE;
                cache->last++;
            }
        }

            #if DEBUGN9
                    fprintf(
                        stderr,
                        ORANGE "[mi_read() → Reemplazamos cache[%d]: %s]\n"
                        RESET,
                        posc, camino
                    );
            #endif

        return pos;
    }
#endif
#if USARCACHE==3
    unsigned int updateCache(struct CacheLRU *cache, const char *camino, const unsigned int *p_inodo)
    {
        unsigned int pos;

        if(cache->size < CACHE_SIZE)
        {
            strcpy(cache->lastEntries[cache->size].camino, camino);
            cache->lastEntries[cache->size].p_inodo = *p_inodo;
            gettimeofday(&cache->lastEntries[cache->size].ultima_consulta, NULL);
            pos = cache->size;
            cache->size++;
        }
        else
        {
            unsigned int posMin = 0;
            for(int i = 1; i < cache->size; i++)
            {
                if(compareTimevals(&cache->lastEntries[i].ultima_consulta, &cache->lastEntries[posMin].ultima_consulta) < 0)
                {
                    posMin = i;
                }
            }
            strcpy(cache->lastEntries[posMin].camino, camino);
            cache->lastEntries[posMin].p_inodo = *p_inodo;
            gettimeofday(&cache->lastEntries[posMin].ultima_consulta, NULL);
            pos = posMin;
        }

        #if DEBUGN9
            fprintf(
                stderr,
                ORANGE "[mi_write() → Reemplazamos cache[%d]: %s]\n"
                RESET,
                posc, camino
            );
        #endif

        return pos;
    }

    int compareTimevals(const struct timeval *t1, const struct timeval *t2)
    {
        if(t1->tv_sec > t2->tv_sec) return 1;
        else if(t1->tv_sec < t2->tv_sec) return -1;
        else
        {
            if(t1->tv_usec > t2->tv_usec) return 1;
            else if(t1->tv_usec < t2->tv_usec) return -1;
        }
        return 0;
    }
#endif

//----------------------------- NIVEL 10 (06/05/2024 - 12/05/2024) -----------------------------

int mi_link(const char *camino1, const char *camino2)
{
    mi_waitSem();

    struct inodo inode1;
    unsigned int p_inode_dir1 = 0;
    unsigned int n_inode1 = 0;
    unsigned int p_entry1 = 0;
    int error = buscar_entrada(camino1, &p_inode_dir1, &n_inode1, &p_entry1, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }
    if (leer_inodo(n_inode1, &inode1) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    if((inode1.permisos & 4) != 4)
    {
        fprintf(
            stderr,
            RED
            "ERROR: the entry to be linked doesn't have read permissions\n"
            RESET
        );
        mi_signalSem();
        return FALLO;
    }

    unsigned int n_inode2 = 0;
    unsigned p_inode_dir2 = 0;
    unsigned p_entry2 = 0;
    error = buscar_entrada(camino2, &p_inode_dir2, &n_inode2, &p_entry2, 1, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }

    struct entrada entry;
    if (mi_read_f(p_inode_dir2, &entry, p_entry2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    unsigned int inode_to_delete = entry.ninodo;
    entry.ninodo = n_inode1;

    if (mi_write_f(p_inode_dir2, &entry, p_entry2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    
    if (liberar_inodo(inode_to_delete) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    
    inode1.nlinks = inode1.nlinks + 1;
    inode1.ctime = time(NULL);

    if (escribir_inodo(n_inode1, &inode1) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();

    return EXITO;
}

int mi_unlink(const char *camino)
{
    mi_waitSem();

    unsigned int p_inode_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;

    int error = buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }

    struct entrada entry;
    if (mi_read_f(p_inode_dir, &entry, p_entry * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    struct inodo inode_entry;
    if (leer_inodo(entry.ninodo, &inode_entry) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    if (inode_entry.tipo == 'd' && inode_entry.tamEnBytesLog > 0)
    {
        fprintf(
            stderr, 
            RED 
            "ERROR: can not delete not empty directory" 
            RESET
        );
        mi_signalSem();
        return FALLO;
    }

    struct inodo inode_dir;
    if (leer_inodo(p_inode_dir, &inode_dir) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }
    unsigned int cant_entries_dir = inode_dir.tamEnBytesLog / sizeof(struct entrada);
    if (p_entry == cant_entries_dir - 1)
    {
        if (mi_truncar_f(p_inode_dir, inode_dir.tamEnBytesLog - sizeof(struct entrada)) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
    }
    else
    {
        struct entrada last_entry;
        if (mi_read_f(p_inode_dir,&last_entry,(cant_entries_dir - 1) * sizeof(struct entrada),sizeof(struct entrada)) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
        if (mi_write_f(p_inode_dir,&last_entry,p_entry * sizeof(struct entrada),sizeof(struct entrada)) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
        if (mi_truncar_f(p_inode_dir,inode_dir.tamEnBytesLog - sizeof(struct entrada)) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
    }

    inode_entry.nlinks = inode_entry.nlinks - 1;
    if (inode_entry.nlinks == 0)
    {
        if (liberar_inodo(entry.ninodo) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
    }
    else
    {
        inode_entry.ctime = time(NULL);
        if (escribir_inodo(entry.ninodo, &inode_entry) == FALLO)
        {
            mi_signalSem();
            return FALLO;
        }
    }

    mi_signalSem();

    return EXITO;
}

int is_directory(const char *camino)
{
    unsigned int p_inode_dir = 0;
    unsigned int n_inode = 0;
    unsigned int p_entry = 0;
    int error = buscar_entrada(camino, &p_inode_dir, &n_inode, &p_entry, 0, 6);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    struct inodo inode;
    leer_inodo(n_inode, &inode);

    if (inode.tipo != 'd')
    {
        fprintf(
            stderr, 
            RED 
            "ERROR: the specified path doesn't refer to a directory\n" 
            RESET
        );
        return FALLO;
    }

    return EXITO;
}