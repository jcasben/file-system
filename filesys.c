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
            printf(BOLD MAGENTA "Closing file system...\n" RESET);
            exit(0);
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
    parse_args(args, line);
    if (strcmp(args[0], "exit") == 0)
    {
        printf(BOLD MAGENTA "Closing file system...\n" RESET);
        exit(0);
    }
    if (strcmp(args[0], "create") == 0) create_disk(args);
    else if (strcmp(args[0], "select") == 0) select_disk(args);
    else if (strcmp(args[0], "ls") == 0) fs_ls(args);
    else if (strcmp(args[0], "mkdir") == 0) fs_mkdir(args);
    else if (strcmp(args[0], "touch") == 0) fs_touch(args);
    else if (strcmp(args[0], "chmod") == 0) fs_chmod(args);
    else if (strcmp(args[0], "cd") == 0) fs_cd(args);
    else if (strcmp(args[0], "rm") == 0) fs_rm(args);
    else if (strcmp(args[0], "rmdir") == 0) fs_rmdir(args);
    else if (strcmp(args[0], "rn") == 0) fs_rn(args);
    else if (strcmp(args[0], "help") == 0) help();

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

void create_disk(char **args)
{
    if (file_exists(args[1]) == 1)
    {
        fprintf(stderr, RED "ERROR: the specified disk already exists\n" RESET);
        return;
    }
    disk_selected = 1;
    strcpy(disk_name, args[1]);
    strcpy(path, "/");
    int wstatus;
    pid_t pid = fork();
    if (pid == 0)
    {
        execvp("./mi_mkfs", args);
    }
    else if (pid > 0) wait(&wstatus);
    printf(BOLD GREEN "\nSUCCESSFULLY CREATED DISK %s\n\n" RESET, disk_name);
}

void select_disk(char **args)
{
    if (file_exists(args[1]) == 0)
    {
        fprintf(stderr, RED "ERROR: the specified disk does not exist\n" RESET);
        return;
    }
    disk_selected = 1;
    strcpy(disk_name, args[1]);
    strcpy(path, "/");
    printf(BOLD GREEN "\nSUCCESSFULLY SELECTED DISK %s\n\n" RESET, disk_name);
}

void fs_ls(char **args)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    if (args[1] == NULL)
    {
        args[1] = disk_name;
        args[2] = path;
        args[3] = NULL;
    } else if (strcmp(args[1], "-l") == 0)
    {
        args[2] = disk_name;
        args[3] = path;
        args[4] = NULL;
    }
    int wstatus;
    pid_t pid = fork();
    if (pid == 0)
    {
        execvp("./mi_ls", args);
    } else if (pid > 0) wait(&wstatus);
}

void fs_mkdir(char **args)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    char permisos[sizeof(args[1])];
    strcpy(permisos, args[1]); 
    args[1] = disk_name;
    char dir_path[sizeof(path)];
    strcpy(dir_path, path);
    strcat(dir_path, args[2]);
    args[2] = permisos;
    args[3] = dir_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0)
    {
        execvp("./mi_mkdir", args);
    } else if (pid > 0) wait(&wstatus);
}

void fs_touch(char **args)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }
    char permisos[sizeof(args[1])];
    strcpy(permisos, args[1]); 
    args[1] = disk_name;
    char dir_path[sizeof(path)];
    strcpy(dir_path, path);
    strcat(dir_path, args[2]);
    args[2] = permisos;
    args[3] = dir_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0)
    {
        execvp("./mi_touch", args);
    } else if (pid > 0) wait(&wstatus);
}

void fs_chmod(char **args)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
        return;
    }

    char permisos[sizeof(args[1])];
    strcpy(permisos, args[1]);
    args[1] = disk_name;
    char file_path[PATHSIZE];
    strcpy(file_path, path);
    strcat(file_path, args[2]);
    args[2] = permisos;
    args[3] = file_path;
    args[4] = NULL;
    int wstatus;
    pid_t pid = fork();
    if (pid == 0)
    {
        execvp("./mi_chmod", args);
    } else if (pid > 0) wait(&wstatus);
}

void fs_cd(char **args)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
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
        strncpy(new_path, path, strlen(path) - strlen(last_dir));
        new_path[strlen(new_path) - 1] = '\0';
        memset(path, 0, PATHSIZE);
        if (strcmp(new_path, "/") != 0) strcat(new_path, "/");
        strcpy(path, new_path);

        return;
    }
    bmount(disk_name);
    char new_path[PATHSIZE];
    strcpy(new_path, path);
    strcat(new_path, args[1]);
    if (is_directory(new_path) == FALLO)
    {
        bumount();
        return;
    }
    bumount();
    strcpy(path, new_path);
}

void fs_rm(char **args)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
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
    if (pid == 0)
    {
        execvp("./mi_rm", args);
    } else if (pid > 0) wait(&wstatus);
}

void fs_rmdir(char **args)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
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
    if (pid == 0)
    {
        execvp("./mi_rmdir", args);
    } else if (pid > 0) wait(&wstatus);
}

void fs_rn(char **args)
{
    if (disk_selected == 0)
    {
        fprintf(stderr, RED "ERROR: select or create a disk to execute this command\n" RESET);
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
    if (pid == 0)
    {
        execvp("./mi_rn", args);
    } else if (pid > 0) wait(&wstatus);
}

void help()
{

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