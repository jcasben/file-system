make clean
make all
echo -e "\nCompilation... DONE\n"
echo -e "Executing ./mi_mkfs...\n"
./mi_mkfs disco 100000
echo -e "\nExecuting ./leer_sf...\n"
./leer_sf disco