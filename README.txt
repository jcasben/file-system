ENTREGA 1 - SISTEMAS OPERATIVOS II

* AUTORES *

- Carlos Gálvez Mena
- Marc Link Cladera
- Jesús Castillo Benito

* OBSERVACIONES *

Todos los programas y funciones siguen la sintaxis descrita en los documentos de la práctica. 
En un principio, la práctica pasa correctamente los tests de todos los niveles, incluido scripte1.sh

MEJORAS PARTE 1 (Se dejan anotada la versión de la entrega anterior, las mejoras realizadas en esta entrega se muestran
en el apartado MEJORAS ENTREGA 2):

La función liberar_bloques_inodo es la versión recursiva auxiliar de Adelaida, sobre la cual hemos hecho las siguientes
modificaciones:

- Hacer que el bucle de liberar_bloques_inodo no compruebe en cada iteración si se requiere liberar los bloques directos.
Esta comprobación se ha movido a antes de realizar el bucle y de esta manera sabemos que cada vez que se itere el bucle
tendremos que ejecutar liberar_indirectos_recursivo.

- Hemos querido implementar la mejora en la que se comprobaba si ya se podía liberar el bloque entero cada vez que se
ponía un puntero a 0, pero debido a que nos ha dado errores y que tenemos algunas dudas de como hacerlo, no se ha
implementado todavía. Tenemos la intención de aplicar esta mejora en el futuro.

Los prints de los saltos no se han implementado debido a algunas dudas que queremos resolver. Se implementarán lo más
pronto posible. De esta misma manera, algunos prints, como por ejemplo el que indica que se ha salvado un bloque, no da
exactamente el mismo resultado que el que da en el resultado de ejemplo. Esto también es algo que nos gustaría corregir
para asegurarnos de que la práctica funciona al 100%.

MEJORAS ENTREGA 2:

* NIVEL 7

- buscar_entrada(): hemos aplicado la mejora en la que a la hora de buscar la entrada correspondiente al path indicado,
leemos del disco un buffer de n entradas (BLOCKSIZE / sizeof(struct entrada)) y luego las vamos recorriendo en memoria.

* NIVEL 8

- mi_touch.c y mi_mkdir.c: se ha separado la funcionalidad de crear ficheros y directorios en mi_touch y mi_mkdir
respectivamente. Si se intenta crear un fichero con mi touch con un path con forma de directorio, se muestra un mensaje
de error que sugiera usar mi_mkdir para crear una carpeta con ese path. Esto ocurre de igual manera con mi_mkdir.

- mi_ls y mi_ls -l: se ha añadido la opción de poder usar el mi_ls en su versión extendida, marcando los nombres de los
directorios de color azul y los ficheros de color blanco tanto en la versión normal como en la versión extendida.
También se puede utilizar la versión extendida con un fichero para ver solo sus atributos en detalle.

- mi_dir(): en la función mi_dir también se ha aplicado la mejora de utilizar un buffer de n entradas para asi ahorrarse
lecturas a disco. Después, se itera sobre este buffer y se pasa la entrada obtenida a una función auxiliar dependiendo
de si se trata de la versión extendida o no. Esta función construye la linea con los datos de esa entrada. En la version
normal se trata de simplemente mostrar el nombre por pantalla. En la versión extendida se muestra lo siguiente:
    - El tipo de la entrada
    - Los permisos de la entrada
    - el mTime
    - El tamaño de la entrada
    - El nombre de la entrada