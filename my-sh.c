#include "functions.h"

#define BUFFER_SIZE 64

char *builtIn[] = { "cd", "help", "exit", "clear", "cat", "cp", "mkdir", "rmdir", "chmod", "filter_by_ext", "filter_by_time"};


/*** function to clear the screen ***/
void clear_shell_window(){
    printf("\033[H\033[J");
}

void help_page(){
    return;
}

int num_of_builtins(){
    return sizeof(builtIn) / sizeof(char*);
}


/*** making an array of tokens ***/
char** split_line(char* line){
    int buf = BUFFER_SIZE;
    int pos = 0;
    char **tokens = malloc(buf * sizeof(char*));
    char *token;

    o_assert(tokens != NULL, "Memory allocations for tokens failed");

    token = strtok(line, " \t\n\r\a");
    while(token){
        tokens[pos++] = token;
        if(pos > buf){
            buf += BUFFER_SIZE;
            tokens = realloc(tokens, buf * sizeof(char*));
            o_assert(tokens != NULL, "Memory reallocation for tokens failed");
        }
        token = strtok(NULL, " \t\n\r\a");
    }
    tokens[pos] = NULL;
    return tokens;
}

/*** executing built-in shell processes located in /bin ***/
int exec_shell_process(char **args){
    pid_t cpid, w;
    int wstatus;

    cpid = fork();
    o_assert(cpid != -1, "fork process failed");

    if(cpid == 0){
        o_assert(execvp(args[0], args) >= 0, "execvp failed");

        exit(0);
    }
    else{
        do{
            w = waitpid(cpid, &wstatus, WUNTRACED);
        } while(!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
    }

return 1;
}


int exec_shell_cmd(char **tokens){
    int i;
    int funcToExec = -1;


    if(tokens[0] == NULL){
        return 1;
    }

    for(i = 0; i < num_of_builtins(); i++){
        if(strcmp(tokens[0], builtIn[i]) == 0){
            funcToExec = i;
            break;
        }
    }
    switch(funcToExec){
        case 0:
            if(chdir(tokens[1]) != 0){
                perror("Error while changing directory.\n");
            }
            return 1;
        case 1:
            help_page();
            return 1;
        case 2:
            exit(0);
        case 3:
            clear_shell_window();
            return 1;
        case 4:
            return catfile(2, tokens);
        case 5:
            return cpfile(tokens);
        case 6:
            return mkdir_f(tokens);
        case 7:
            return rmdir_f(tokens);
        case 8:
            return chmod_f(tokens);
        case 9:
            return filter_by_ext(tokens);
        case 10:
            return filter_by_time(tokens);
    }

    return exec_shell_process(tokens);
}

int main(){
    char *line = NULL;
    char **tokens;
    char* info = malloc(1024);
    int exe = 1;
    char* username = getenv("USER");
    char cwd[1024];

    rl_bind_key('\t', rl_insert);
    clear_shell_window();

    while(exe){
        getcwd(cwd, sizeof(cwd));

        sprintf(info, "\e[1m\x1b[31m[\x1b[36m@%s \x1b[35m%s\e[31m]\e[0m\x1b[39m$ ", username, cwd);

        line = readline(info);
        if(strlen(line) > 0){
            add_history(line);
        }

        tokens = split_line(line);
        exe = exec_shell_cmd(tokens);

        free(line);
        free(tokens);
    }

    free(info);
    return 0;
}
