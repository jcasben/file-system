#include "ficheros_basico.h"

#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 1

int main(int argc, char **argv)
{
    if (bmount(argv[1]) < 0)
    {
        return FALLO;
    }

    struct superbloque SB;
    bread(posSB, &SB);
    
    // Show all the information of the superblock
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    printf(BLUE "cantBloquesLibres = %d\n" RESET, SB.cantBloquesLibres);
    printf("cantInodosLibres = %d\n", SB.cantInodosLibres);
    printf("totBloques = %d\n", SB.totBloques);
    printf("totInodos = %d\n", SB.totInodos);

    #if DEBUGN2
        printf("\nsizeof struct superbloque: %ld\n", sizeof(struct superbloque));
        printf("sizeof struct inodo: %ld\n", sizeof(struct inodo));
        
        printf("\n\n RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
        
        struct inodo inodos[BLOCKSIZE/INODOSIZE];
        int i = SB.posPrimerBloqueAI;
        while (i <= SB.posUltimoBloqueAI)
        {
            if(bread(i, inodos) == FALLO)
            {
                printf(RED "ERROR: Error reading an inode" RESET);
            }
            for (size_t j = 0; j < BLOCKSIZE/INODOSIZE; j++)
            {
                printf("%d ", inodos[j].punterosDirectos[0]);
            }
            printf("\n");
            i++;
        }
    #endif

    #if DEBUGN3
        printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
        int nblock = reservar_bloque();
        bread(posSB, &SB);
        printf("Se ha reservado el bloque físico nº %d, que era el 1º libre indicado por el MB\n", nblock);
        printf("SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);
        liberar_bloque(nblock);
        bread(posSB, &SB);
        printf("Liberamos ese bloque y despues SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);
        printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
        int value = leer_bit(posSB);
        printf("posSB: %d -> leer_bit(%d) = %d\n",posSB, posSB, value);
        value = leer_bit(SB.posPrimerBloqueMB);
        printf("SB.posPrimerBloqueMB: %d -> leer_bit(%d) = %d\n",SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, value);
        value = leer_bit(SB.posUltimoBloqueMB);
        printf("SB.posUltimoBloqueMB: %d -> leer_bit(%d) = %d\n",SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, value);
        value = leer_bit(SB.posPrimerBloqueAI);
        printf("SB.posPrimerBloqueAI: %d -> leer_bit(%d) = %d\n",SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, value);
        value = leer_bit(SB.posUltimoBloqueAI);
        printf("SB.posUltimoBloqueAI: %d -> leer_bit(%d) = %d\n",SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, value);
        value = leer_bit(SB.posPrimerBloqueDatos);
        printf("SB.posPrimerBloqueDatos: %d -> leer_bit(%d) = %d\n",SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, value);
        value = leer_bit(SB.posUltimoBloqueDatos);
        printf("SB.posUltimoBloqueDatos: %d -> leer_bit(%d) = %d\n",SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, value);
        printf("\nDATOS DEL DIRECTORIO RAIZ\n");

        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];

        struct inodo inode;
        int ninode = 0;
        leer_inodo(ninode, &inode);
        ts = localtime(&inode.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inode.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inode.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf("tipo: %c\n", inode.tipo);
        printf("permisos: %d\n", inode.permisos);
        printf("atime: %s\n", atime);
        printf("ctime: %s\n", ctime);
        printf("mtime: %s\n", mtime);
        printf("nlinks: %d\n", inode.nlinks);
        printf("tamEnBytesLog: %d\n", inode.tamEnBytesLog);
        printf("numBloquesOcupados: %d\n", inode.numBloquesOcupados);
    #endif
    
    #if DEBUGN4
        int ind = reservar_inodo('f', 6);
        struct inodo inode;
        leer_inodo(ind, &inode);
        int lblocs[] = {8, 204, 30004, 400004, 468750};
        for (size_t i = 0; i < 5; i++)
        {
            traducir_bloque_inodo(&inode, lblocs[i], 1);
        }
        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80]; 
        ts = localtime(&inode.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inode.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inode.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf("\nDATOS DEL INODO RESERVADO 1\n");
        printf("tipo: %c\n", inode.tipo);
        printf("permisos: %d\n", inode.permisos);
        printf("atime: %s\n", atime);
        printf("ctime: %s\n", ctime);
        printf("mtime: %s\n", mtime);
        printf("nlinks: %d\n", inode.nlinks);
        printf("tamEnBytesLog: %d\n", inode.tamEnBytesLog);
        printf("numBloquesOcupados: %d\n", inode.numBloquesOcupados);
        bread(posSB, &SB);
        printf("SB.posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    #endif
    
    if (bumount() < 0)
    {
        return FALLO;
    }
    
    return 0;
}