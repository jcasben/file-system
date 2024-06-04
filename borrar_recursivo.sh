# Adelaida
# borrar_recursivo.sh

#ejecutar primero el script estructura.sh para montar el árbol de directorios de ejmplo

echo -e "\x1B[38;2;17;245;120m$ ./mi_rm_r disco /dir3/\x1b[0m"
./mi_rm_r disco /dir3/
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco
echo -e "\x1B[38;2;17;245;120m$ ./mi_rm_r disco /dir2/\x1b[0m"
./mi_rm_r disco /dir2/
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco
echo -e "\x1B[38;2;17;245;120m$ ./mi_rm_r disco /dir1/\x1b[0m"
./mi_rm_r disco /dir1/
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco