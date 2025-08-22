#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#define MAX_COMMAND_LENGTH 256
#define HISTORY_SIZE 10
#define command "mini-shell>"
#define MAX_SIZE = 100
typedef struct{
    char **history; //array of command strings
    int history_count; // total number of commands entered
    int history_start;//start index for circular buffer
    int current_size;// current number of commands in history
}Shell;
void init_shell(Shell *shell);
void cleanup_shell(Shell *shell);
void shell_loop(Shell *shell);
int read_command(char *buffer, int max_len);
void add_to_history(Shell *shell, const char *command);
void print_history(Shell *shell);
void execute_command(Shell *shell, const char *command);

// Custom string functions
int my_strlen(const char *str);
char *my_strdup(const char *str);
int my_strcmp(const char *s1, const char *s2);
void my_strcpy(char *dest, const char *src);

// Signal handler
void handle_sigint(int sig);


int main(){
    printf("Welcome to the Text File Analyzer!\n");
    // there is going to be an input loop that will continue until the user decides to exit
    // it'll store all the input in a dynamically allocated array of strings
    //np strcpy(), no strlen() they should be reimplemented
    //Circular buffer will be used to store the input
    // it will keep memory usage constant while allowing for dynamic input and keeping track of input history
    Shell shell;
//set up signal handler for SIGINT
    signal(SIGINT, handle_sigint);
    // Initialize our shell
    init_shell(&shell);
    
    // Main shell loop
    shell_loop(&shell);
    
    // Cleanup
    cleanup_shell(&shell);


    return 0;
}


//Shell initialization and cleanup


void init_shell(Shell *shell){
    if(!shell) return; // check for null pointer
    shell->history = malloc(HISTORY_SIZE * sizeof(char *));
    if (!shell->history) {
        write(STDERR_FILENO,"Failed to allocate memory for shell history\n",34);
        exit(1);
    }
    // Initialize history pointers to null
    for (int i = 0; i < HISTORY_SIZE; i++) {
        shell->history[i] = NULL;
    }
    shell->history_count = 0;
    shell->history_start = 0;
    shell->current_size = 0;

    write(STDOUT_FILENO, "Shell v1.0 initialized - Type 'exit' to quit\n", 42);



}

/*

clean up allocated memory




*/
void cleanup_shell(Shell *shell){
    if(!shell || !shell->history) return; // check for null pointer
    //free all allocated command strings
    for (int i = 0; i < HISTORY_SIZE; i++)
    {
        free(shell->history[i]);
        shell->history[i] = NULL; // set to NULL after freeing
    }
    //free the history array itself
    free(shell->history);
    shell->history = NULL; // set to NULL after freeing
    write(STDOUT_FILENO, "Shell cleanup complete\n", 24);
}
