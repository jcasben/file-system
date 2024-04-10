ENTREGA 1 - SISTEMAS OPERATIVOS II

* AUTORES *

- Carlos Gálvez Mena
- Marc Link Cladera
- Jesús Castillo Benito

* OBSERVACIONES *

Todos los programas y funciones siguen la sintaxis descrita en los documentos de la práctica. 
En un principio, la práctica pasa correctamente los tests de todos los niveles, incluido scripte1.sh

La función liberar_bloques_inodo es la versión recursiva auxiliar de Adelaida, sobre la cual hemos hecho las siguientes modificaciones:
- Hacer que el bucle de liberar_bloques_inodo no compruebe en cada iteración si se requiere liberar los bloques directos. Esta comprobación
se ha movido a antes de realizar el bucle y de esta manera sabemos que cada vez que se itere el bucle tendremos que ejecutar 
liberar_indirectos_recursivo.
- Hemos querido implementar la mejora en la que se comprobaba si ya se podía liberar el bloque entero cada vez que se ponía un puntero a 0, pero
debido a que nos ha dado errores y que tenemos algunas dudas de como hacerlo, no se ha implementado todavía. Tenemos la intención de aplicar esta
mejora en el futuro.

Los prints de los saltos no se han implementado debido a algunas dudas que queremos resolver. Se implementarán lo más pronto posible. De esta misma manera,
algunos prints, como por ejemplo el que indica que se ha salvado un bloque, no da exactamente el mismo resultado que el que da en el resultado de ejemplo.
Esto también es algo que nos gustaría corregir para asegurarnos de que la práctica funciona al 100%.