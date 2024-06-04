# Adelaida
# mover.sh
echo -e "\x1B[38;2;17;245;120m$./leer_sf disco\x1b[0m"
./leer_sf disco
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /dir2/dir22/\x1b[0m"
./mi_ls -l disco /dir2/dir22/
echo -e "\x1B[38;2;17;245;120m$./mi_mv disco /dir2/dir22/fic221 /dir1/fic11 #error destino\x1b[0m"
./mi_mv disco /dir2/dir22/fic221 /dir1/fic11
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /dir2/dir21/\x1b[0m"
./mi_ls -l disco /dir2/dir21/
echo -e "\x1B[38;2;17;245;120m$./mi_mv disco /dir2/dir22/fic221 /dir2/dir21/ #movemos un fichero\x1b[0m"
./mi_mv disco /dir2/dir22/fic221 /dir2/dir21/
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /dir2/dir22/\x1b[0m"
./mi_ls -l disco /dir2/dir22/
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /dir2/dir21/\x1b[0m"
./mi_ls -l disco /dir2/dir21/
echo -e "\x1B[38;2;17;245;120m$./leer_sf disco #Un bloque más libre\x1b[0m"
./leer_sf disco
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /\x1b[0m"
./mi_ls -l disco /
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /dir1/\x1b[0m"
./mi_ls -l disco /dir1/
echo -e "\x1B[38;2;17;245;120m$./mi_mv disco /dir3/ /dir1/ #movemos un directorio\x1b[0m"
./mi_mv disco /dir3/ /dir1/
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /\x1b[0m"
./mi_ls -l disco /
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /dir1/\x1b[0m"
./mi_ls -l disco /dir1/
echo -e "\x1B[38;2;17;245;120m$./leer_sf disco\x1b[0m"
./leer_sf disco
echo -e "\x1B[38;2;17;245;120m$./mi_mv disco /dir1/ /dir3/ #error\x1b[0m"
./mi_mv disco /dir1/ /dir3/

echo -e "\x1B[38;2;17;245;120m###lo dejamos todo como antes!\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$./mi_mv disco /dir2/dir21/fic221 /dir2/dir22/\x1b[0m"
./mi_mv disco /dir2/dir21/fic221 /dir2/dir22/
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /dir2/dir22/\x1b[0m"
./mi_ls -l disco /dir2/dir22/
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /dir2/dir21/\x1b[0m"
./mi_ls -l disco /dir2/dir21/
echo -e "\x1B[38;2;17;245;120m$./leer_sf disco #Un bloque menos libre\x1b[0m"
./leer_sf disco
echo -e "\x1B[38;2;17;245;120m$./mi_mv disco /dir1/dir3/ /\x1b[0m"
./mi_mv disco /dir1/dir3/ /
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /\x1b[0m"
./mi_ls -l disco /
echo -e "\x1B[38;2;17;245;120m$./mi_ls -l disco /dir1/\x1b[0m"
./mi_ls -l disco /dir1/
echo -e "\x1B[38;2;17;245;120m$./leer_sf disco\x1b[0m"
./leer_sf disco