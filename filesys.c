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
    else if (strcmp(args[0], "create") == 0) create_disk(args);
    else if (strcmp(args[0], "select") == 0) select_disk(args);
    else if (strcmp(args[0], "ls") == 0) ls(args);

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
    disk_selected = 1;
    strcpy(disk_name, args[1]);
    strcpy(path, "/");
    int wstatus;
    pid_t pid = fork();
    if (pid == 0)
    {
        execvp("./mi_mkfs", args);
        return;
    }
    else if (pid > 0) wait(&wstatus);
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

void ls(char **args)
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
    } else if (strcmp(args[1], "-l") == 0)
    {
        args[2] = disk_name;
        args[3] = path;
    }
    int wstatus;
    pid_t pid = fork();
    if (pid == 0)
    {
        execvp("./mi_ls", args);
        return;
    } else if (pid > 0) wait(&wstatus);
}

int file_exists(char *path_to_file)
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