# Adelaida
# estructura.sh

clear
make clean
make
echo -e "\x1B[38;2;17;245;120mESTE SCRIPT CREA VARIOS DIRECTORIOS Y FICHEROS, CON INFORMACIÓN EN VARIOS OFFSETS CON EL PROPÓSITO DE HACER PRUEBAS\x1b[0m"
echo -e "\x1B[38;2;17;245;120m#####################################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkfs disco 100000\x1b[0m"
./mi_mkfs disco 100000
echo -e "\x1B[38;2;17;245;120m#####################################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120mCreamos un set de directorios y ficheros para hacer pruebas\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir1/\x1b[0m"
./mi_mkdir disco 6 /dir1/
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir1/dir11/\x1b[0m"
./mi_mkdir disco 6 /dir1/dir11/
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir1/dir12/\x1b[0m"
./mi_mkdir disco 6 /dir1/dir12/
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir1/dir12/fic121/\x1b[0m"
./mi_touch disco 6 /dir1/dir12/fic121
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir1/dir12/fic122/\x1b[0m"
./mi_touch disco 6 /dir1/dir12/fic122
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir1/dir13/\x1b[0m"
./mi_mkdir disco 6 /dir1/dir13/
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir1/fic11/\x1b[0m"
./mi_touch disco 6 /dir1/fic11
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir2/\x1b[0m"
./mi_mkdir disco 6 /dir2/
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir2/dir21/\x1b[0m"
./mi_mkdir disco 6 /dir2/dir21/
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir2/dir21/fic211\x1b[0m"
./mi_touch disco 6 /dir2/dir21/fic211
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir2/dir22/\x1b[0m"
./mi_mkdir disco 6 /dir2/dir22/
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir2/dir22/fic221\x1b[0m"
./mi_touch disco 6 /dir2/dir22/fic221
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir2/dir23/\x1b[0m"
./mi_mkdir disco 6 /dir2/dir23/
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir3/\x1b[0m"
./mi_mkdir disco 6 /dir3/
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir3/fic31\x1b[0m"
./mi_touch disco 6 /dir3/fic31
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir3/dir31/\x1b[0m"
./mi_mkdir disco 6 /dir3/dir31/
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir3/dir32/\x1b[0m"
./mi_mkdir disco 6 /dir3/dir32/
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir3/dir33/\x1b[0m"
./mi_mkdir disco 6 /dir3/dir33/
echo -e "\x1B[38;2;17;245;120m#####################################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120mMetemos información en diferentes offsets de los ficheros\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_escribir disco /dir1/dir12/fic121 \"Haz un buen día - \" 0\x1b[0m"
./mi_escribir disco /dir1/dir12/fic121 "Haz un buen día - " 0
echo -e "\x1B[38;2;17;245;120m$ ./mi_escribir disco /dir1/dir12/fic121 \"Hoy puede ser un gran día, plantéatelo así. Aprovecharlo o que pase de largo depende en parte de ti. - \" 5120 \x1b[0m"
./mi_escribir disco /dir1/dir12/fic121 "Hoy puede ser un gran día, plantéatelo así. Aprovecharlo o que pase de largo depende en parte de ti. - " 5120
echo -e "\x1B[38;2;17;245;120m$ ./mi_escribir disco /dir1/dir12/fic121 \"Si la rutina te aplasta dile que ya basta de mediocridad - \" 48000 \x1b[0m"
./mi_escribir disco /dir1/dir12/fic121 "Si la rutina te aplasta dile que ya basta de mediocridad - " 48000
echo -e "\x1B[38;2;17;245;120m$ ./mi_escribir disco /dir1/dir12/fic122 \"Pelea por lo que quieres y no desesperes si algo no anda bien - \" 90000\x1b[0m"
./mi_escribir disco /dir1/dir12/fic122 "Pelea por lo que quieres y no desesperes si algo no anda bien - " 90000
echo -e "\x1B[38;2;17;245;120m$ ./mi_escribir disco /dir1/dir12/fic122 \"Hoy puede ser un gran día y mañana también - \" 15000\x1b[0m"
./mi_escribir disco /dir1/dir12/fic122 "Hoy puede ser un gran día y mañana también - " 15000
echo -e "\x1B[38;2;17;245;120m$ ./mi_escribir disco /dir1/fic11 \"Hoy puede ser un gran día, date una oportunidad - \" 70000\x1b[0m"
./mi_escribir disco /dir1/fic11 "Hoy puede ser un gran día, date una oportunidad - " 70000
echo -e "\x1B[38;2;17;245;120m$ ./mi_escribir disco /dir2/dir21/fic211 \"Hoy puede ser un gran día, imposible de recuperar - \" 100\x1b[0m"
./mi_escribir disco /dir2/dir21/fic211 "Hoy puede ser un gran día, imposible de recuperar - " 100
echo -e "\x1B[38;2;17;245;120m$ ./mi_escribir disco /dir2/dir22/fic221 \"Hoy puede ser un gran día, duro con él - \" 3333\x1b[0m"
./mi_escribir disco /dir2/dir22/fic221 "Hoy puede ser un gran día, duro con él - " 3333
echo -e "\x1B[38;2;17;245;120m$ ./mi_escribir disco /dir3/fic31 \"Que la fuerza te acompañe - \" 1000\x1b[0m"
./mi_escribir disco /dir3/fic31 "Que la fuerza te acompañe - " 1000
echo -e "\x1B[38;2;17;245;120m#####################################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120mComprobamos el contenido de algunos directorios\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /\x1b[0m"
./mi_ls -l disco /
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir1/\x1b[0m"
./mi_ls -l disco /dir1/
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir1/fic11\x1b[0m"
./mi_cat disco /dir1/fic11
echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir1/fic11\x1b[0m"
./mi_stat disco /dir1/fic11
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir1/dir12/\x1b[0m"
./mi_ls -l disco /dir1/dir12/
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir1/dir12/fic121\x1b[0m"
./mi_cat disco /dir1/dir12/fic121
echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir1/dir12/fic121\x1b[0m"
./mi_stat disco /dir1/dir12/fic121
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir1/dir12/fic122\x1b[0m"
./mi_cat disco /dir1/dir12/fic122
echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir1/dir12/fic122\x1b[0m"
./mi_stat disco /dir1/dir12/fic122
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/\x1b[0m"
./mi_ls -l disco /dir2/
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/dir21/\x1b[0m"
./mi_ls -l disco /dir2/dir21/
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir2/dir21/fic211\x1b[0m"
./mi_cat disco /dir2/dir21/fic211
echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir2/dir21/fic211\x1b[0m"
./mi_stat disco /dir2/dir21/fic211
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir2/dir22/\x1b[0m"
./mi_ls -l disco /dir2/dir22/
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir2/dir22/fic221\x1b[0m"
./mi_cat disco /dir2/dir22/fic221
echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir2/dir22/fic221\x1b[0m"
./mi_stat disco /dir2/dir22/fic221
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls -l disco /dir3/\x1b[0m"
./mi_ls -l disco /dir3/
echo -e "\x1B[38;2;17;245;120m$ ./mi_cat disco /dir3/fic31\x1b[0m"
./mi_cat disco /dir3/fic31
echo -e "\x1B[38;2;17;245;120m$ ./mi_stat disco /dir3/fic31\x1b[0m"
./mi_stat disco /dir3/fic31
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco