#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include "directorios.h"

#define TAMDISKNAME 128
#define BLACK "\x1B[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1B[37m"
#define ORANGE "\x1B[38;2;255;128;0m"
#define ROSE "\x1B[38;2;255;151;203m"
#define LBLUE "\x1B[38;2;53;149;240m"
#define LGREEN "\x1B[38;2;17;245;120m"
#define GRAY "\x1B[38;2;176;174;174m"
#define BOLD "\x1b[1m"

#define COMMAND_LINE_SIZE 4096
#define PATHSIZE 1024
#define ARGS_SIZE 64

void print_prompt();
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
void select_disk(char **args, int argc);
void create_disk(char **args, int argc);
void fs_ls(char **args, int argc);
void fs_mkdir(char **args, int argc);
void fs_touch(char **args, int argc);
void fs_chmod(char **args, int argc);
void fs_cd(char **args, int argc);
void fs_rm(char **args, int argc);
void fs_rmdir(char **args, int argc);
void fs_cat(char **args, int argc);
void fs_write(char **args, int argc);
void fs_link(char **args, int argc);
void fs_stat(char **args, int argc);
void fs_rn(char **args, int argc);
void fs_mv(char **args, int argc);
void fs_rm_r(char **args, int argc);
void fs_cp(char **args, int argc);
void help();
int file_exists(const char *path_to_file);
void fs_exit();