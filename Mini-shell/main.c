#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>    // Added for waitpid()
#include <string.h>      // Added for getenv() support
//unix-based
// Configuration constants
#define MAX_COMMAND_LENGTH 256
#define HISTORY_SIZE 10
#define MAX_ARGS 64          // Added for command parsing
#define PROMPT "mini-shell> "

// Shell structure
typedef struct {
    char **history;          // Array of command strings
    int history_count;       // Total number of commands entered
    int history_start;       // Start index for circular buffer
    int current_size;        // Current number of commands in history
} Shell;

// Command structure for parsing - NEW
typedef struct {
    char **args;            // Array of argument strings
    int argc;              // Number of arguments
} Command;

// Function prototypes
void init_shell(Shell *shell);
void cleanup_shell(Shell *shell);
void shell_loop(Shell *shell);
int read_command(char *buffer, int max_len);
void add_to_history(Shell *shell, const char *command);
void print_history(Shell *shell);
void execute_command(Shell *shell, const char *command);
void print_string(const char *str);
char *get_history_command(Shell *shell, int cmd_num);
void trim_whitespace(char *str);  // NEW

// Command parsing and execution - NEW
Command *parse_command(const char *input);
void free_command(Command *cmd);
int execute_external_command(Command *cmd);
int is_builtin_command(const char *command);
void execute_builtin(Shell *shell, Command *cmd);

// Custom string functions
int my_strlen(const char *str);
char *my_strdup(const char *str);
int my_strcmp(const char *s1, const char *s2);
void my_strcpy(char *dest, const char *src);

// Signal handlers
void handle_sigint(int sig);
void handle_sigchld(int sig);  // NEW

// Global shell pointer for signal handler
Shell *global_shell = NULL;

// ============================================================================
// CUSTOM STRING FUNCTIONS 
// ============================================================================

int my_strlen(const char *str) {
    int len = 0;
    if (!str) return 0;
    
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void my_strcpy(char *dest, const char *src) {
    if (!dest || !src) return;
    
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';  // Never forget the null terminator!
}

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

// ============================================================================
// UTILITY FUNCTIONS 
// ============================================================================

void trim_whitespace(char *str) {
    if (!str) return;
    
    // Remove trailing whitespace
    int len = my_strlen(str);
    while (len > 0 && (str[len-1] == ' ' || str[len-1] == '\t' || 
                       str[len-1] == '\n' || str[len-1] == '\r')) {
        str[--len] = '\0';
    }
    
    // Remove leading whitespace
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t') {
        start++;
    }
    
    if (start > 0) {
        int i = 0;
        while (str[start + i] != '\0') {
            str[i] = str[start + i];
            i++;
        }
        str[i] = '\0';
    }
}

void print_string(const char *str) {
    if (!str) return;
    write(STDOUT_FILENO, str, my_strlen(str));
}

// ============================================================================
// COMMAND PARSING 
// ============================================================================

Command *parse_command(const char *input) {
    if (!input) return NULL;
    
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) return NULL;
    
    cmd->args = malloc(MAX_ARGS * sizeof(char*));
    if (!cmd->args) {
        free(cmd);
        return NULL;
    }
    
    cmd->argc = 0;
    
    // Create working copy
    char *input_copy = my_strdup(input);
    if (!input_copy) {
        free(cmd->args);
        free(cmd);
        return NULL;
    }
    
    // Simple tokenization by spaces
    char *token_start = NULL;
    int in_token = 0;
    
    for (int i = 0; input_copy[i] != '\0' && cmd->argc < MAX_ARGS - 1; i++) {
        if (input_copy[i] != ' ' && input_copy[i] != '\t') {
            if (!in_token) {
                token_start = &input_copy[i];
                in_token = 1;
            }
        } else {
            if (in_token) {
                input_copy[i] = '\0';
                cmd->args[cmd->argc] = my_strdup(token_start);
                cmd->argc++;
                in_token = 0;
            }
        }
    }
    
    // Handle last token
    if (in_token && cmd->argc < MAX_ARGS - 1) {
        cmd->args[cmd->argc] = my_strdup(token_start);
        cmd->argc++;
    }
    
    // Null-terminate argument array
    cmd->args[cmd->argc] = NULL;
    
    free(input_copy);
    return cmd;
}

void free_command(Command *cmd) {
    if (!cmd) return;
    
    for (int i = 0; i < cmd->argc; i++) {
        if (cmd->args[i]) {
            free(cmd->args[i]);
        }
    }
    free(cmd->args);
    free(cmd);
}

// ============================================================================
// PROCESS EXECUTION - FORK/EXEC IMPLEMENTATION
// ============================================================================

int execute_external_command(Command *cmd) {
    if (!cmd || cmd->argc == 0) return -1;
    
    print_string("Executing: ");
    print_string(cmd->args[0]);
    print_string("\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        if (execvp(cmd->args[0], cmd->args) == -1) {
            write(STDERR_FILENO, "Command not found: ", 19);
            write(STDERR_FILENO, cmd->args[0], my_strlen(cmd->args[0]));
            write(STDERR_FILENO, "\n", 1);
            exit(127);
        }
    } else if (pid > 0) {
        // Parent process
        int status;
        pid_t waited_pid = waitpid(pid, &status, 0);
        
        if (waited_pid == -1) {
            write(STDERR_FILENO, "Error waiting for child\n", 24);
            return -1;
        }
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                char msg[] = "Command exited with code: X\n";
                msg[26] = '0' + (exit_code % 10);
                write(STDOUT_FILENO, msg, 28);
            }
            return exit_code;
        }
    } else {
        // Fork failed
        write(STDERR_FILENO, "Fork failed\n", 12);
        return -1;
    }
    
    return 0;
}

// ============================================================================
// BUILT-IN COMMANDS 
// ============================================================================

int is_builtin_command(const char *command) {
    if (!command) return 0;
    
    return (my_strcmp(command, "exit") == 0 ||
            my_strcmp(command, "history") == 0 ||
            my_strcmp(command, "clear") == 0 ||
            my_strcmp(command, "cd") == 0 ||
            my_strcmp(command, "pwd") == 0 ||
            my_strcmp(command, "help") == 0);
}

void execute_builtin(Shell *shell, Command *cmd) {
    if (!shell || !cmd || cmd->argc == 0) return;
    
    if (my_strcmp(cmd->args[0], "exit") == 0) {
        print_string("Exiting shell...\n");
        cleanup_shell(shell);
        exit(0);
    }
    
    if (my_strcmp(cmd->args[0], "history") == 0) {
        print_history(shell);
        return;
    }
    
    if (my_strcmp(cmd->args[0], "clear") == 0) {
        write(STDOUT_FILENO, "\033[2J\033[H", 7);
        return;
    }
    
    if (my_strcmp(cmd->args[0], "cd") == 0) {
        if (cmd->argc < 2) {
            char *home = getenv("HOME");
            if (home && chdir(home) != 0) {
                write(STDERR_FILENO, "cd: cannot change to home\n", 27);
            }
        } else {
            if (chdir(cmd->args[1]) != 0) {
                write(STDERR_FILENO, "cd: cannot change directory\n", 29);
            }
        }
        return;
    }
    
    if (my_strcmp(cmd->args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            print_string(cwd);
            print_string("\n");
        } else {
            write(STDERR_FILENO, "pwd: error\n", 11);
        }
        return;
    }
    
    if (my_strcmp(cmd->args[0], "help") == 0) {
        print_string("Built-in commands:\n");
        print_string("  exit     - Exit shell\n");
        print_string("  history  - Show history\n");
        print_string("  clear    - Clear screen\n");
        print_string("  cd [dir] - Change directory\n");
        print_string("  pwd      - Print working directory\n");
        print_string("  help     - Show this help\n");
        print_string("  !!       - Repeat last command\n");
        print_string("  !n       - Repeat command n\n");
        return;
    }
}

// ============================================================================
// SHELL INITIALIZATION AND CLEANUP 
// ============================================================================

void init_shell(Shell *shell) {
    if (!shell) return;
    
    shell->history = malloc(HISTORY_SIZE * sizeof(char*));
    if (!shell->history) {
        write(STDERR_FILENO, "Failed to allocate memory for shell history\n", 44);
        exit(1);
    }
    
    // Initialize history pointers to NULL
    for (int i = 0; i < HISTORY_SIZE; i++) {
        shell->history[i] = NULL;
    }
    
    shell->history_count = 0;
    shell->history_start = 0;
    shell->current_size = 0;
    
    global_shell = shell;  // Set global pointer
    
    write(STDOUT_FILENO, " Mini-Shell - Type 'help' for commands\n", 38);
}

void cleanup_shell(Shell *shell) {
    if (!shell || !shell->history) return;
    
    // Free all allocated command strings
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (shell->history[i]) {  // Only free non-NULL pointers
            free(shell->history[i]);
            shell->history[i] = NULL;
        }
    }
    
    // Free the history array itself
    free(shell->history);
    shell->history = NULL;
    
    write(STDOUT_FILENO, "Shell cleanup complete!\n", 33);
}

// ============================================================================
// I/O FUNCTIONS 
// ============================================================================

int read_command(char *buffer, int max_len) {
    if (!buffer || max_len <= 0) return -1;
    
    int pos = 0;
    char ch;
    
    while (pos < max_len - 1) {
        ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
        
        if (bytes_read <= 0) return -1;
        
        if (ch == '\n') break;
        
        if (ch == '\b' || ch == 127) {  // Backspace
            if (pos > 0) {
                pos--;
                write(STDOUT_FILENO, "\b \b", 3);  // Erase character visually
            }
            continue;
        }
        
        if (ch >= 32 && ch <= 126) {  // Printable characters only
            buffer[pos++] = ch;
            write(STDOUT_FILENO, &ch, 1);  // Echo character back 
        }
    }
    
    buffer[pos] = '\0';
    write(STDOUT_FILENO, "\n", 1);
    
    trim_whitespace(buffer);  // Clean up input
    return my_strlen(buffer);
}

// ============================================================================
// HISTORY MANAGEMENT 
// ============================================================================

void add_to_history(Shell *shell, const char *command) {
    if (!shell || !command || my_strlen(command) == 0) return;
    
    // Don't add history commands to history
    if (command[0] == '!' || my_strcmp(command, "history") == 0) return;
    
    int index = (shell->history_start + shell->current_size) % HISTORY_SIZE;
    
    if (shell->current_size == HISTORY_SIZE) {
        if (shell->history[shell->history_start]) {
            free(shell->history[shell->history_start]);
        }
        shell->history_start = (shell->history_start + 1) % HISTORY_SIZE;
    } else {
        shell->current_size++;
    }
    
    shell->history[index] = my_strdup(command);
    shell->history_count++;
}

void print_history(Shell *shell) {  
    if (!shell) return;
    
    print_string("Command History:\n");
    
    if (shell->current_size == 0) {
        print_string("  (no commands yet)\n");
        return;
    }
    
    for (int i = 0; i < shell->current_size; i++) {
        int index = (shell->history_start + i) % HISTORY_SIZE;  
        if (shell->history[index]) {
            char num_str[16];
            int cmd_num = shell->history_count - shell->current_size + i + 1;
            
            // Integer to string conversion
            int len = 0;
            int temp = cmd_num;
            if (temp == 0) {
                num_str[len++] = '0';
            } else {
                while (temp > 0) {
                    num_str[len++] = '0' + (temp % 10);
                    temp /= 10;
                }
            }
            
            // Reverse the string
            for (int j = 0; j < len / 2; j++) {
                char swap = num_str[j];
                num_str[j] = num_str[len - 1 - j];
                num_str[len - 1 - j] = swap;
            }
            num_str[len] = '\0';
            
            print_string("  ");
            print_string(num_str);
            print_string(": ");
            print_string(shell->history[index]);
            print_string("\n");
        }
    }
}

char *get_history_command(Shell *shell, int cmd_num) {
    if (!shell || cmd_num <= 0) return NULL;
    
    int relative_pos = cmd_num - (shell->history_count - shell->current_size + 1);
    
    if (relative_pos < 0 || relative_pos >= shell->current_size) {
        return NULL;
    }
    
    int index = (shell->history_start + relative_pos) % HISTORY_SIZE;
    return shell->history[index];
}

// ============================================================================
// COMMAND EXECUTION 
// ============================================================================

void execute_command(Shell *shell, const char *command) {
    if (!shell || !command || my_strlen(command) == 0) return;
    
    // Handle history replay - FIXED to prevent infinite recursion
    if (command[0] == '!') {
        if (my_strlen(command) == 1) {
            print_string("Usage: !! (last) or !n (number)\n");
            return;
        }
        
        char *target_cmd = NULL;
        
        if (command[1] == '!') {
            /*!! - repeat last command*/ 
            if (shell->current_size > 0) {
                int last_index = (shell->history_start + shell->current_size - 1) % HISTORY_SIZE;
                target_cmd = shell->history[last_index];
            } else {
                print_string("No previous command\n");
                return;
            }
        } else {
            /*  !n - repeat 
            command number n */
            int cmd_num = 0;
            int i = 1;
            while (command[i] >= '0' && command[i] <= '9') {
                cmd_num = cmd_num * 10 + (command[i] - '0');
                i++;
            }
            
            if (cmd_num == 0) {
                print_string("Invalid command number\n");
                return;
            }
            
            target_cmd = get_history_command(shell, cmd_num);
        }
        
        if (target_cmd) {
            print_string("Executing: ");
            print_string(target_cmd);
            print_string("\n");
            execute_command(shell, target_cmd);  // Recursive call with actual command
        } else {
            print_string("Command not found in history\n");
        }
        return;
    }
    
    // Parse command into arguments
    Command *cmd = parse_command(command);
    if (!cmd || cmd->argc == 0) {
        if (cmd) free_command(cmd);
        return;
    }
    
    // Execute built-in or external command
    if (is_builtin_command(cmd->args[0])) {
        execute_builtin(shell, cmd);
    } else {
        execute_external_command(cmd);
    }
    
    free_command(cmd);
}

// ============================================================================
// SIGNAL HANDLING 
// ============================================================================

void handle_sigint(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
    if (global_shell) {
        write(STDOUT_FILENO, PROMPT, my_strlen(PROMPT));
    }
}

void handle_sigchld(int sig) {
    (void)sig;
    // Reap zombie children
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// ============================================================================
// MAIN SHELL LOOP 
// ============================================================================

void shell_loop(Shell *shell) {
    char command_buffer[MAX_COMMAND_LENGTH];
    
    while (1) {
        print_string(PROMPT);
        
        int cmd_len = read_command(command_buffer, MAX_COMMAND_LENGTH);
        
        if (cmd_len < 0) {
            print_string("Error reading command\n");
            continue;
        }
        
        if (cmd_len == 0) continue;  // Empty command
        
        add_to_history(shell, command_buffer);
        execute_command(shell, command_buffer);
    }
}

// ============================================================================
// MAIN FUNCTION 
// ============================================================================

int main() {
    printf("Welcome to the Mini-Shell!\n");
    
    Shell shell;
    
    // Set up signal handlers
    signal(SIGINT, handle_sigint);    // Handle Ctrl+C
    signal(SIGCHLD, handle_sigchld);  // Handle child processes
    
    init_shell(&shell);
    shell_loop(&shell);
    cleanup_shell(&shell);
    
    return 0;
}
