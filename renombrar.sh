# Adelaida
# renombrar.sh

#ejecutar primero el script estructura.sh para montar el árbol de directorios de ejmplo

echo -e "\x1B[38;2;17;245;120m$ ./mi_rn disco /dir1/dir12/fic122 fic121 #error\x1b[0m"
./mi_rn disco /dir1/dir12/fic122 fic121
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir2/dir22/fic221\x1b[0m"
./mi_cat disco /dir2/dir22/fic221
echo -e "\x1B[38;2;17;245;120m$ ./mi_rn disco /dir2/dir22/fic221 fic222\x1b[0m"
./mi_rn disco /dir2/dir22/fic221 fic222
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir2/dir22/fic221\x1b[0m"
./mi_cat disco /dir2/dir22/fic221
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir2/dir22/fic222\x1b[0m"
./mi_cat disco /dir2/dir22/fic222
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/dir22/\x1b[0m"
./mi_ls -l disco /dir2/dir22/
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir1/dir12/fic122\x1b[0m"
./mi_cat disco /dir1/dir12/fic122
echo -e "\x1B[38;2;17;245;120m$ ./mi_rn disco /dir1/dir12/fic122 fic123\x1b[0m"
./mi_rn disco /dir1/dir12/fic122 fic123
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir1/dir12/fic122\x1b[0m"
./mi_cat disco /dir1/dir12/fic122
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir1/dir12/fic123\x1b[0m"
./mi_cat disco /dir1/dir12/fic123
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir1/dir12/\x1b[0m"
./mi_ls -l disco /dir1/dir12/
echo -e "\x1B[38;2;17;245;120m$ ./mi_rn disco /dir2/dir23/ dir24\x1b[0m"
./mi_rn disco /dir2/dir23/ dir24
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/\x1b[0m"
./mi_ls -l disco /dir2/

echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco
echo
echo -e "\x1B[38;2;17;245;120m###lo dejamos todo como antes!\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_rn disco /dir1/dir12/fic123 fic122\x1b[0m"
./mi_rn disco /dir1/dir12/fic123 fic122
echo -e "\x1B[38;2;17;245;120m$ ./mi_rn disco /dir2/dir22/fic222 fic221\x1b[0m"
./mi_rn disco /dir2/dir22/fic222 fic221
echo -e "\x1B[38;2;17;245;120m$ ./mi_rn disco /dir2/dir24/ dir23\x1b[0m"
./mi_rn disco /dir2/dir24/ dir23
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir1/dir12/\x1b[0m"
./mi_ls -l disco /dir1/dir12/
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/dir22/\x1b[0m"
./mi_ls -l disco /dir2/dir22/
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/\x1b[0m"
./mi_ls -l disco /dir2/

echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco