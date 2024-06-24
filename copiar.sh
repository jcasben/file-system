# Adelaida
# copiar.sh

echo -e "\x1B[38;2;17;245;120m$ #Copia de un fichero en un directorio\x1b[0m"
#echo -e "\x1B[38;2;17;245;120m$./mi_cp_f disco    /dir1/dir12/fic121     /dir3/\x1b[0m"
#./mi_cp_f disco    /dir1/dir12/fic121     /dir3/
echo -e "\x1B[38;2;17;245;120m$./mi_cp disco    /dir1/dir12/fic121     /dir3/\x1b[0m"
./mi_cp disco    /dir1/dir12/fic121     /dir3/
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco

echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir1/dir12/fic121\x1b[0m"
./mi_stat disco /dir1/dir12/fic121
echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir3/fic121 #ha de ser igual salvo sellos de tiempo\x1b[0m"
./mi_stat disco /dir3/fic121

echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir1/dir12/fic121\x1b[0m"
./mi_cat disco /dir1/dir12/fic121
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir3/fic121\x1b[0m"
./mi_cat disco /dir3/fic121

echo -e "\x1B[38;2;17;245;120m######################################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ #Copia de un directorio en otro\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_cp disco /dir2/ /dir3/dir32/\x1b[0m"
./mi_cp disco /dir2/ /dir3/dir32/
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco

echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/\x1b[0m"
./mi_ls -l disco /dir2/
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir3/dir32/dir2/\x1b[0m"
./mi_ls -l disco /dir3/dir32/dir2/

echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/dir21/\x1b[0m"
./mi_ls -l disco /dir2/dir21/
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir3/dir32/dir2/dir21/\x1b[0m"
./mi_ls -l disco /dir3/dir32/dir2/dir21/

echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/dir22/\x1b[0m"
./mi_ls -l disco /dir2/dir22/
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir3/dir32/dir2/dir22/\x1b[0m"
./mi_ls -l disco /dir3/dir32/dir2/dir22/

echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir2/dir21/fic211\x1b[0m"
./mi_stat disco /dir2/dir21/fic211
echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir3/dir32/dir2/dir21/fic211\x1b[0m"
./mi_stat disco /dir3/dir32/dir2/dir21/fic211

echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir2/dir22/fic221\x1b[0m"
./mi_stat disco /dir2/dir22/fic221
echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir3/dir32/dir2/dir22/fic221\x1b[0m"
./mi_stat disco /dir3/dir32/dir2/dir22/fic221

echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir2/dir21/fic211\x1b[0m"
./mi_cat disco /dir2/dir21/fic211
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir3/dir32/dir2/dir21/fic211\x1b[0m"
./mi_cat disco /dir3/dir32/dir2/dir21/fic211

echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir2/dir22/fic221\x1b[0m"
./mi_cat disco /dir2/dir22/fic221
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir3/dir32/dir2/dir22/fic221\x1b[0m"
./mi_cat disco /dir3/dir32/dir2/dir22/fic221