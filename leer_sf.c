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
    printf("\n\n RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    int i = SB.posPrimerBloqueAI;
    struct inodo inodo_aux;
    while (i != SB.posUltimoBloqueAI)
    {
        bread(i, &inodo_aux);
        printf("%d ", inodo_aux.punterosDirectos[0]);
        i = inodo_aux.punterosDirectos[0];
    }
    printf("\n");

    if (bumount() < 0)
    {
        return FALLO;
    }
    
    return 0;
}