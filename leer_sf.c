#include "ficheros_basico.h"

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
    printf("\nsizeof struct superbloque: %ld\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %ld\n", sizeof(struct inodo));
    /*printf("\n\n RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    
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
    }*/
    printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int nblock = reservar_bloque();
    printf("Se ha reservado el bloque físico nº %d, que era el 1º libre indicado por el MB\n", nblock);
    printf("SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);
    liberar_bloque(nblock);
    printf("Liberamos ese bloque y despues SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);
    printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    int value = leer_bit(0);
    printf("posSb: 0 -> leer_bit(0) = %d\n", value);
    value = leer_bit(1);
    printf("SB.PrimerBloqueMB: 1 -> leer_bit(1) = %d\n", value);
    value = leer_bit(13);
    printf("SB.UltimoBloqueMB: 13 -> leer_bit(13) = %d\n", value);
    value = leer_bit(14);
    printf("SB.PrimerBloqueAI: 14 -> leer_bit(14) = %d\n", value);
    value = leer_bit(3138);
    printf("SB.UltimoBloqueAI: 3138 -> leer_bit(3138) = %d\n", value);
    value = leer_bit(3139);
    printf("SB.PrimerBloqueDatos: 3139 -> leer_bit(3139) = %d\n", value);
    value = leer_bit(99999);
    printf("SB.UltimoBloqueDatos: 99999 -> leer_bit(99999) = %d\n", value);
    printf("DATOS DEL DIRECTORIO RAIZ\n");

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
    printf("tipo: %d\n", inode.tipo);
    printf("permisos: %d\n", inode.permisos);
    printf("atime: %ld\n", inode.atime);
    printf("ctime: %ld\n", inode.ctime);
    printf("mtime: %ld\n", inode.mtime);
    printf("nlinks: %d\n", inode.nlinks);
    printf("tamEnBytesLog: %d\n", inode.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inode.numBloquesOcupados);

    
    if (bumount() < 0)
    {
        return FALLO;
    }
    
    return 0;
}