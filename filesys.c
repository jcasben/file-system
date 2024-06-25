/*
    Autores:
        - Marc Link Cladera
        - Jesús Castillo Benito
*/

#include "filesys.h"

int disk_selected = 0;
char disk_name[TAMDISKNAME];
char path[PATHSIZE];

int main()
{
    char line[COMMAND_LINE_SIZE];
    while (1) if(read_line(line)) execute_line(line);
}

char *read_line(char *line)
{
    print_prompt();
    char *result_fgets = fgets(line, COMMAND_LINE_SIZE, stdin);
    if (result_fgets) line[strlen(line) - 1] = '\0';
    else
    {
        printf("\n\r");
        if (feof(stdin))
        {
            fs_exit();
        }
    }

    return result_fgets;
}

void print_prompt()
{
    if (disk_selected == 0) printf(YELLOW "filesys> " RESET);
    else printf(YELLOW "filesys> %s%s" RESET "$ ", disk_name, path);
}

int execute_line(char *line)
{
    char *args[ARGS_SIZE];
    memset(args, 0, ARGS_SIZE * 8);
    int argc = parse_args(args, line);
    if (strcmp(args[0], "exit") == 0)
    {
        fs_exit();
    }
    if (strcmp(args[0], "create") == 0) create_disk(args, argc);
    else if (strcmp(args[0], "select") == 0) select_disk(args, argc);
    else if (strcmp(args[0], "ls") == 0) fs_ls(args, argc);
    else if (strcmp(args[0], "mkdir") == 0) fs_mkdir(args, argc);
    else if (strcmp(args[0], "touch") == 0) fs_touch(args, argc);
    else if (strcmp(args[0], "chmod") == 0) fs_chmod(args, argc);
    else if (strcmp(args[0], "cd") == 0) fs_cd(args, argc);
    else if (strcmp(args[0], "rm") == 0) fs_rm(args, argc);
    else if (strcmp(args[0], "rmdir") == 0) fs_rmdir(args, argc);
    else if (strcmp(args[0], "cat") == 0) fs_cat(args, argc);
    else if (strcmp(args[0], "write") == 0) fs_write(args, argc);
    else if (strcmp(args[0], "link") == 0) fs_link(args, argc);
    else if (strcmp(args[0], "stat") == 0) fs_stat(args, argc);
    else if (strcmp(args[0], "rn") == 0) fs_rn(args, argc);
    else if (strcmp(args[0], "mv") == 0) fs_mv(args, argc);
    else if (strcmp(args[0], "rm_r") == 0) fs_rm_r(args, argc);
    else if (strcmp(args[0], "cp") == 0) fs_cp(args, argc);
    else if (strcmp(args[0], "help") == 0) help();
    else fprintf(stderr, RED "ERROR: unknown function\n" RESET);

    return 0;
}

int parse_args(char **args, char *line)
{
    char *delim = " ";
    int i = 0;
    args[i] = strtok(line, delim);

    while (args[i] != NULL)
    {
        i++;
        args[i] = strtok(NULL, delim);
    }
    args[i] = NULL;

    return i;
}

void create_disk(char **args, int argc)
{
    if (file_exists(args[1]) == 1)
    {
        fprintf(stderr, RED "ERROR: the specified disk already exists\n" RESET);
        return;
    }
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: create <disk_name> <nblocks>\n" RESET);
        return;
    }
    disk_selected = 1;
    strcpy(disk_name, args[1]);
    strcpy(path, "/");
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_mkfs", args);
    else if (pid > 0) wait(&wstatus);
    printf(BOLD GREEN "\nSUCCESSFULLY CREATED DISK %s\n\n" RESET, disk_name);
}

void select_disk(char **args, int argc)
{
    if (file_exists(args[1]) == 0)
    {
        fprintf(stderr, RED "ERROR: the specified disk does not exist\n" RESET);
        return;
    }
    if (argc != 2)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: select <disk_name>\n" RESET);
        return;
    }
    disk_selected = 1;
    strcpy(disk_name, args[1]);
    strcpy(path, "/");
    printf(BOLD GREEN "\nSUCCESSFULLY SELECTED DISK %s\n\n" RESET, disk_name);
}

void fs_ls(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc > 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: ls [-l] <path>\n" RESET);
        return;
    }

    if (args[1] == NULL)
    {
        args[1] = disk_name;
        args[2] = path;
        args[3] = NULL;
    } else
    {
        if (strcmp(args[1], "-l") == 0)
        {
            if (args[2] != NULL)
            {
                char file_path[PATHSIZE];
                memset(file_path, 0, PATHSIZE);
                strcat(file_path, path);
                strcat(file_path, args[2]);
                args[3] = file_path;
            } else args[3] = path;
            args[2] = disk_name;
            args[4] = NULL;
        } else
        {
            char file_path[PATHSIZE];
            memset(file_path, 0, PATHSIZE);
            strcat(file_path, path);
            strcat(file_path, args[1]);
            args[1] = disk_name;
            args[2] = file_path;
            args[3] = NULL;
        }
    }
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_ls", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_mkdir(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: mkdir <permissions> <path/>\n" RESET);
        return;
    }

    char permisos[sizeof(args[1])];
    memset(permisos, 0, sizeof(permisos));
    strcpy(permisos, args[1]); 
    args[1] = disk_name;
    char dir_path[sizeof(path)];
    memset(dir_path, 0, sizeof(path));
    strcpy(dir_path, path);
    strcat(dir_path, args[2]);
    args[2] = permisos;
    args[3] = dir_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_mkdir", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_touch(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: touch <permissions> <path>\n" RESET);
        return;
    }

    char permisos[sizeof(args[1])];
    strcpy(permisos, args[1]); 
    args[1] = disk_name;
    char dir_path[sizeof(path)];
    memset(dir_path, 0, sizeof(path));
    strcpy(dir_path, path);
    strcat(dir_path, args[2]);
    args[2] = permisos;
    args[3] = dir_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_touch", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_chmod(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: chmod <permissions> <path>\n" RESET);
        return;
    }


    char permisos[sizeof(args[1])];
    memset(permisos, 0, sizeof(permisos));
    strcpy(permisos, args[1]);
    args[1] = disk_name;
    char file_path[PATHSIZE];
    memset(file_path, 0, PATHSIZE);
    strcpy(file_path, path);
    strcat(file_path, args[2]);
    args[2] = permisos;
    args[3] = file_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_chmod", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_cd(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 2)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: cd <path/> | ..\n" RESET);
        return;
    }

    if (strcmp(args[1], "..") == 0)
    {
        if (strcmp(path, "/") == 0)
        {
            fprintf(stderr, RED "ERROR: you already are at the root directory\n" RESET);
            return;
        }
        char tmp[PATHSIZE];
        memset(tmp, 0, PATHSIZE);
        strncpy(tmp, path, strlen(path) - 1);
        char *last_dir = strrchr(tmp, '/');
        char new_path[PATHSIZE];
        memset(new_path, 0, PATHSIZE);
        strncpy(new_path, path, strlen(path) - strlen(last_dir));
        new_path[strlen(new_path) - 1] = '\0';
        memset(path, 0, PATHSIZE);
        if (strcmp(new_path, "/") != 0) strcat(new_path, "/");
        strcpy(path, new_path);

        return;
    }

    if (args[1][strlen(args[1]) - 1] != '/')
    {
        fprintf(stderr, RED "ERROR: destination must be a directory\n" RESET);
        return;
    }
    bmount(disk_name);
    char new_path[PATHSIZE];
    memset(new_path, 0, PATHSIZE);
    strcpy(new_path, path);
    strcat(new_path, args[1]);
    if (is_directory(new_path) == FALLO)
    {
        fprintf(stderr, RED "ERROR: no such directory\n" RESET);
        bumount();
        return;
    }
    bumount();

    strcpy(path, new_path);
}

void fs_rm(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 2)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: rm <path>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    memset(file_path, 0, PATHSIZE);
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    args[1] = disk_name;
    args[2] = file_path;
    args[3] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_rm", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_rm_r(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 2)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: rm_r <path>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    memset(file_path, 0, PATHSIZE);
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    args[1] = disk_name;
    args[2] = file_path;
    args[3] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_rm_r", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_rmdir(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 2)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: rmdir <path>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    memset(file_path, 0, PATHSIZE);
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    args[1] = disk_name;
    args[2] = file_path;
    args[3] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_rmdir", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_rn(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: rn <old_path> <new_name>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    memset(file_path, 0, PATHSIZE);
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    char new_path[PATHSIZE];
    memset(new_path, 0, PATHSIZE);
    strcpy(new_path, args[2]);
    args[1] = disk_name;
    args[2] = file_path;
    args[3] = new_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_rn", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_cat(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 2)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: cat <path>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    memset(file_path, 0, PATHSIZE);
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    args[1] = disk_name;
    args[2] = file_path;
    args[3] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_cat", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_write(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc < 4)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: write <file_path> <text> <offset>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    memset(file_path, 0, sizeof(file_path));
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    char text[(argc - 3) * 64];
    memset(text, 0, sizeof(text));
    unsigned int tam = 0;
    for (int i = 3; i < argc ; ++i) {
        tam += sizeof(args[i]);
        tam++; //Para el espacio en blanco
        strcat(text, args[i]);
        if(i != (argc-1)) strcat(text, " ");
    }
    char clear_text[tam];
    memset(clear_text, 0, sizeof(clear_text));
    strncpy(clear_text, text, tam);

    char offset[sizeof(args[2])];
    memset(offset, 0, sizeof(offset));
    strcpy(offset, args[2]);

    args[1] = disk_name;
    args[2] = file_path;
    args[3] = clear_text;
    args[4] = offset;
    args[5] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_escribir", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_link(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: link <original_path> <link_path>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    char new_path[PATHSIZE];
    strcpy(new_path, args[2]);
    args[1] = disk_name;
    args[2] = file_path;
    args[3] = new_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_link", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_stat(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 2)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: stat <path>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    args[1] = disk_name;
    args[2] = file_path;
    args[3] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_stat", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_mv(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: mv <original_path> <new_path>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    char new_path[PATHSIZE];
    strcpy(new_path, args[2]);
    args[1] = disk_name;
    args[2] = file_path;
    args[3] = new_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_mv", args);
    else if (pid > 0) wait(&wstatus);
}

void fs_cp(char **args, int argc)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (argc != 3)
    {
        fprintf(stderr, RED "ERROR: invalid syntax. Usage: cp <original_path> <destination/>\n" RESET);
        return;
    }

    char file_path[PATHSIZE];
    strcpy(file_path, path);
    strcat(file_path, args[1]);
    char new_path[PATHSIZE];
    strcpy(new_path, args[2]);
    args[1] = disk_name;
    args[2] = file_path;
    args[3] = new_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0) execvp("./mi_cp", args);
    else if (pid > 0) wait(&wstatus);
}

void help()
{
    printf(BOLD "\n**In all functions where a path can be entered as a parameter, the path used will be the path you are currently working on, followed by the path you passed as an argument to the function. In addition, the functions will be executed on the device that has been selected.**\n\n" RESET
            "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
            BLUE "create:" RESET " Creates the virtual device with the specified name and the specified blocks.\n"
            "\tSyntax: create <device name> <nblocks>\n\n"
            BLUE "select:" RESET " Changes to the working device that you enter by parameter. The disk that you enter by parameter must have been created previously.\n"
            "\tSyntax: select <device name>\n\n"
            BLUE "ls:" RESET " Show the information of the entries of a directory or a file. Path is optional.\n"
            "\tSyntax: ls [-l] [<path>] \n\n"
            BLUE "mkdir:" RESET " Creates the directory with the permissions that are entered by parameter.\n"
            "\tSyntax: mkdir <permissions> <path>\n\n"
            BLUE "touch:" RESET " Creates the file with the permissions that are entered by parameter.\n"
            "\tSyntax: touch <permissions <path>\n\n"
            BLUE "chmod:" RESET " Modify the permissions of a file or directory.\n"
            "\tSyntax: chmod <permissions> [<path>]\n\n"
            BLUE "cd:" RESET " Tool to move between directories. With .. you can go to the previous directory and if you enter a path(it must be a directory) by parameters you can go to the route as long as it exists\n"
            "\tSyntax: cd ..|<path>\n\n"
            BLUE "rm:" RESET " Ddelete files.\n"
            "\tSyntax: rm [<path>] \n\n"
            BLUE "rmdir:" RESET " Delete directories. The directory must be empty.\n"
            "\tSyntax: rmdir [<path>] \n\n"
            BLUE "rm_r:" RESET " Delete files or directories. If there are other files or directories within the directory, these will also be deleted.\n"
            "\tSyntax: rm_r [<path>] \n\n"
            BLUE "cat:" RESET " Shows the information of a file.\n"
            "\tSyntax: cat [<path>] \n\n"
            BLUE "write:" RESET " Write the text that you pass as a parameter in the indicated file. The text must not be enclosed in double quotes.\n"
            "\tSyntax: write <file_path> <offset> <text>\n\n"
            BLUE "link:" RESET " Creates a link to the directory entry of the given path.\n"
            "\tSyntax: link <path> <new_path>\n"
            "\t\t <path> is the path of the file to be linked.\n"
            "\t\t <new_path> is the path of the new link.\n\n"
            BLUE "stat:" RESET " Shows the information of a file or a directory.\n"
            "\tSyntax: stat [<path>]\n\n"
            BLUE "rn:" RESET " Changes the name of the directory or file that is located in the path that is entered by parameter.\n"
            "\tSyntax: rn <path> <new_name>\n\n"
            BLUE "mv:" RESET " Move a file or directory to the destination path(destination must be a directory).\n"
            "\tSyntax: mv <src> <dest>\n\n"
            BLUE "cp:" RESET " Copy a file or directory to the destination path that you enter as a parameter. The destination must be a directory.\n"
            "\tSyntax: cp <src> <dest>\n\n"
           "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
    );

}

int file_exists(const char *path_to_file)
{
    FILE *fp = fopen(path_to_file, "r");
    int is_exist = 0;
    if (fp != NULL)
    {
        is_exist = 1;
        fclose(fp); // close the file
    }

    return is_exist;
}

void fs_exit()
{
    printf(BOLD MAGENTA "Closing file system...\n" RESET);
    sleep(1);
    printf(BOLD GREEN "\nFILE SYSTEM CLOSED SUCCESSFULLY.\n\n" RESET);
    exit(0);
}