#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

// configuration constants
#define MAX_COMMAND_LENGTH 256
#define HISTORY_SIZE 10
#define MAX_SIZE  100
#define PROMPT "mini-shell>"

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

// ============================================================================
// CUSTOM STRING FUNCTIONS - No standard library dependencies
// ============================================================================

/**
 * Calculate string length manually
 * Learning: Pointer arithmetic and null terminator detection
 */
int my_strlen(const char *str) {
    int len = 0;
    if (!str) return 0;
    
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/**
 * Copy string from source to destination
 * Learning: Manual memory copying and bounds awareness
 */
void my_strcpy(char *dest, const char *src) {
    if (!dest || !src) return;
    
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';  // Don't forget null terminator!
}

/**
 * Compare two strings
 * Learning: Character-by-character comparison
 * Returns: 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int my_strcmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);
    
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) {
            return s1[i] - s2[i];
        }
        i++;
    }
    return s1[i] - s2[i];
}

/**
 * Duplicate a string (allocate new memory)
 * Learning: Manual memory allocation and error handling
 */
char *my_strdup(const char *str) {
    if (!str) return NULL;
    
    int len = my_strlen(str);
    char *dup = malloc(len + 1);  // +1 for null terminator
    
    if (!dup) {
        write(STDERR_FILENO, "Memory allocation failed\n", 25);
        return NULL;
    }
    
    my_strcpy(dup, str);
    return dup;
}





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

// Implementing Low level I/O functions
int read_command(char *buffer, int max_len){
    if (!buffer || max_len<=0)
    {
        return -1;
    }
    int pos = 0;
    char ch;
    while(pos<max_len-1){
        //leave space for null terminator "max_len-1"
        ssize_t bytes_read = read(STDIN_FILENO, &ch,1);
        if (bytes_read<=0)
        {
            return -1;
            /* code */

        }
        if (ch == '\n')
        {
            // if the current character is a new line character we stop
            break;
            /* code */
        }
        if (ch=='\b' || ch ==127){
            if (pos>0)
            {
                pos--;
                write(STDOUT_FILENO,&ch,1);
                /* code */
            }
            continue;
            
        }
        
    }
    buffer[pos] = '\0';
    write(STDOUT_FILENO,"\n",1);
    return pos;
    
}

/**
 * Print a string using low-level write()
 * Learning: Manual string output without printf
 */

void print_string(const char *str){
    // we pass the pointer to the string because strings in C are stored as arrays. and arrays cannot be passed by value
    /*
    Passing a pointer allows the function to access the original string data stored in memory, regardless of its length.
     If you tried to pass the whole array by value, 
    only the address would be passed, not the contents, and the function wouldn’t know how much data to print.
    
    */
    if (!str) return;
    write(STDOUT_FILENO,str,my_strlen(str));


}

// ============================================================================
// CIRCULAR BUFFER HISTORY MANAGEMENT
// ============================================================================

/**
 * Add command to circular history buffer
 * Learning: Circular buffer logic and memory reuse
 */

void add_to_history(Shell *shell, const char *command)
{
    if(!shell || !command || my_strlen(command)==0) return ;
    //calculate the string where the command will be stored
    int index = (shell->history_start + shell->current_size) % HISTORY_SIZE;
    /*
    let's say we have a buffer of size 5
    [0][1][2][3][4]
    our current size is at 5
    and history_start will always be at the first command we stored
    so for this example
    index = (0+5)%5=0; so the oldest command will be overwritten with the newest one.
    
    */
   // if the buffer is full we need to free the oldest command
   if (shell->current_size == HISTORY_SIZE)
   {
    if(shell->history[shell->history_start]){
        free(shell->history[shell->history_start]);
    }
    shell->history_start = (shell->history_start+1)%HISTORY_SIZE;
   }else{
    shell->current_size++;
   }
   
   //store the new command
   shell->history[index] = my_strdup(command);
   shell->history_count++;

}
/**
 * Print command history
 * Learning: Circular buffer traversal
 */

void print_history(Shell *shell){
    if(!shell) return;
    print_string("command history:\n");
    if (shell->current_size==0)
    {
        print_string("no cammands yet :-) \n");
        return;
        for (int i = 0; i < shell->current_size; i++)
        {
            int index = (shell->history_start+1)%HISTORY_SIZE;
            if(shell->history[index]){
                char num_str[16];
                int cmd_num = shell->history_count -shell->current_size + i + 1;
                // integer to string conversion
                int len = 0;
                int temp = cmd_num;
                do
                {
                    num_str[len++] = '0' + (temp%10);
                    temp/=10;
                    /* code */
                } while (temp>0);
            // reverse the string
            for (int j = 0; j < len/2; j++)
            {
                char swap = num_str[j];
                num_str[j] = num_str[len - 1 - j];
                num_str[len - 1 - j] = swap;
            }    
            num_str[len] = '\0';
            print_string(" ");
            print_string(num_str);
            print_string(": ");
            print_string(shell->history[index]);
            print_string("\n");
            }
            

            
            /* code */
        }
        
    }
    
}

// ============================================================================
// COMMAND EXECUTION AND HISTORY REPLAY
// ============================================================================
 
/**
 * Get command from history by number
 * Learning: History indexing and bounds checking
 */


char *get_history_command(Shell *shell, int cmd_num){
    if(!shell || cmd_num<=0) return NULL;
    int relative_pos = cmd_num -(shell->history_count - shell->current_size+1);
    if (relative_pos <0 || relative_pos>=shell->current_size)
    {
        return NULL; // i.e command number out of range
    }
    int index = (shell->history_start + relative_pos) % HISTORY_SIZE;
    return shell->history[index];
}
/**
 * Execute a command (built-ins and history replay)
 * Learning: Command parsing and execution logic
 */
void execute_command(Shell *shell ,const char *command){
    if(!shell|| !command) return;
    //skip empty commands
    if(my_strlen(command) == 0) return;
    //handle built-in commands
    if (my_strcmp(command,"exit")==0){
        print_string("Exiting shell.....\n");
        cleanup_shell(shell);
        exit(0);
    }
    if (my_strcmp(command, "history") == 0) {
        print_history(shell);
        return;
    }
    
    if (my_strcmp(command, "clear") == 0) {
        write(STDOUT_FILENO, "\033[2J\033[H", 7);  // ANSI clear screen
        return;
    }
    //handle history replay commands
    if (command[0] == '!') {
        if (command[1] == '!') {
            /*!! - repeat last command*/
            if (shell->current_size > 0) {
                int last_index = (shell->history_start + shell->current_size - 1) % HISTORY_SIZE;
                char *last_cmd = shell->history[last_index];
                if (last_cmd) {
                    print_string("Executing: ");
                    print_string(last_cmd);
                    print_string("\n");
                    execute_command(shell, last_cmd);
                }
            } else {
                print_string("No previous command\n");
            }
            return;
        } else {
            /* ! n - repeat command number n*/
            int cmd_num = 0;
            int i = 1;
            while (command[i] >= '0' && command[i] <= '9') {
                cmd_num = cmd_num * 10 + (command[i] - '0');
                i++;
            }
            
            char *hist_cmd = get_history_command(shell, cmd_num);
            if (hist_cmd) {
                print_string("Executing: ");
                print_string(hist_cmd);
                print_string("\n");
                execute_command(shell, hist_cmd);
            } else {
                print_string("Command not found in history\n");
            }
            return;
        }

}
    // For other commands, just echo them (in a practical shell, i should execute them)
    print_string("command executed!");
    print_string(command);
    print_string("\n");    

}
// ============================================================================
// MAIN SHELL LOOP AND SIGNAL HANDLING
// ============================================================================

/**
 * Signal handler for Ctrl+C
 * Learning: Signal handling and graceful interruption
 */
void handle_sigint(int sig){
    (void)sig;  // Suppress unused parameter warning
    write(STDOUT_FILENO, "\n" PROMPT, my_strlen(PROMPT) + 1);
}

/**
 * Main shell loop
 * Learning: Event loop pattern and user interaction
 */
void shell_loop(Shell *shell) {
    char command_buffer[MAX_COMMAND_LENGTH];
    
    while (1) {
        // Print prompt
        print_string(PROMPT);
        
        // Read command
        int cmd_len = read_command(command_buffer, MAX_COMMAND_LENGTH);
        
        if (cmd_len < 0) {
            print_string("Error reading command\n");
            continue;
        }
        
        if (cmd_len == 0) {
            continue;  // Empty command
        }
        
        // Add to history (before execution to handle recursive calls)
        add_to_history(shell, command_buffer);
        
        // Execute command
        execute_command(shell, command_buffer);
    }

}

// Next fork() and exec() needs to be implemented.

/*
    The process flow will be the following:
            Parent Shell Process
                     |
                fork() called
                     |
                __________________
               |                  |
            child process         Parent process
               |                  | 
               excec("ls")        wait() for child
               |                  |
               becomes "ls"       child completes
               |                  |  
               exit()            continues shell loop
*/
