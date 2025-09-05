Mini-Shell

A fully-functional, lightweight command-line shell implementation in C, featuring custom memory management, circular buffer history, and real process execution capabilities.

📋 Table of Contents
Overview
Features
Architecture
Installation
Usage
Built-in Commands
Technical Implementation
Code Structure
Memory Management
Signal Handling
Testing
Contributing
License
🎯 Overview

This project implements a production-ready mini-shell from scratch in C, demonstrating advanced systems programming concepts including:

Custom memory management without relying on standard library functions
Process creation and management using fork(), exec(), and wait()
Circular buffer implementation for efficient command history storage
Low-level I/O operations using system calls
Signal handling for graceful interrupt management
Command parsing and execution with argument separation

Perfect for understanding operating systems internals, process management, and systems programming fundamentals.

✨ Features
🚀 Core Functionality
Real command execution - Execute system programs with arguments
Interactive command line - Full terminal interaction with character echoing
Command history - Circular buffer storing last 10 commands
History replay - Execute previous commands with !! and !n
Built-in commands - Essential shell operations
Process management - Proper child process handling and cleanup
🛠️ Advanced Features
Custom string functions - No dependency on string.h
Memory leak prevention - Comprehensive cleanup and error handling
Signal handling - Graceful Ctrl+C handling and zombie process prevention
Command parsing - Intelligent argument separation and validation
Error reporting - Detailed error messages and exit code reporting
Cross-platform compatibility - Works on Linux and Unix-like systems
🎨 User Experience
Intuitive prompt - Clear command prompt with shell identification
Backspace support - Visual character deletion during input
Input validation - Filters non-printable characters
Help system - Built-in help command with usage instructions


🔧 Installation
Prerequisites
GCC compiler (or any C99-compatible compiler)
Linux/Unix environment (tested on Ubuntu, CentOS, macOS)
POSIX-compliant system for system calls
Build Instructions
# Clone the repository
git clone https://github.com/yourusername/enhanced-mini-shell.git
cd Mini-Shell

# Compile with recommended flags
gcc -Wall -Wextra -std=c99 -o myshell main.c
or
gcc <source-code-file-name>.c -o <output-file-name>

# Run the shell
./output-file-name


Compilation Options
# Debug build with symbols
gcc -Wall -Wextra -std=c99 -g -DDEBUG -o myshell improved_shell.c



🚀 Usage
Starting the Shell
$ ./myshell
Welcome to this Mini-Shell!
Mini-Shell  - Type 'help' for commands
mini-shell> 

Basic Command Execution
mini-shell> ls -la
Executing: ls
total 48
drwxr-xr-x  3 user user  4096 Dec 15 10:30 .
drwxr-xr-x 25 user user  4096 Dec 15 10:25 ..
-rwxr-xr-x  1 user user 15432 Dec 15 10:30 myshell
-rw-r--r--  1 user user 12847 Dec 15 10:29 improved_shell.c

mini-shell> echo "Hello, World!"
Executing: echo
Hello, World!

mini-shell> date
Executing: date
Fri Dec 15 10:31:42 PST 2023

History Management
mini-shell> pwd
/home/user/projects

mini-shell> ls
file1.txt  file2.txt  myshell

mini-shell> history
Command History:
  1: pwd
  2: ls

mini-shell> !1
Executing: pwd
/home/user/projects

mini-shell> !!
Executing: pwd
/home/user/projects

📚 Built-in Commands
Command	Description	Usage Example
help	Display available commands	help
exit	Exit the shell	exit
history	Show command history	history
clear	Clear the screen	clear
cd [dir]	Change directory	cd /tmp or cd (home)
pwd	Print working directory	pwd
!!	Repeat last command	!!
!n	Repeat command number n	!3
Advanced Usage Examples
# Directory navigation
mini-shell> cd /usr/local/bin
mini-shell> pwd
/usr/local/bin

mini-shell> cd
mini-shell> pwd
/home/user

# Command chaining with history
mini-shell> ls /etc
mini-shell> !1
mini-shell> !!

# Complex commands with arguments
mini-shell> find . -name "*.c" -type f
mini-shell> grep -r "main" *.c
mini-shell> ps aux | grep myshell

🔬 Technical Implementation
Memory Management Strategy

The shell implements a zero-leak memory management system:

// Custom memory allocation with error checking
char *my_strdup(const char *str) {
    if (!str) return NULL;
    
    int len = my_strlen(str);
    char *dup = malloc(len + 1);
    
    if (!dup) {
        write(STDERR_FILENO, "Memory allocation failed\n", 25);
        return NULL;
    }
    
    my_strcpy(dup, str);
    return dup;
}

Process Execution Model
// Fork-exec-wait pattern for command execution
int execute_external_command(Command *cmd) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process: execute command
        execvp(cmd->args[0], cmd->args);
        exit(127);  // Command not found
    } else if (pid > 0) {
        // Parent process: wait for completion
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
    
    return -1;  // Fork failed
}

Circular Buffer History
// Efficient O(1) history operations
void add_to_history(Shell *shell, const char *command) {
    int index = (shell->history_start + shell->current_size) % HISTORY_SIZE;
    
    if (shell->current_size == HISTORY_SIZE) {
        // Overwrite oldest command
        free(shell->history[shell->history_start]);
        shell->history_start = (shell->history_start + 1) % HISTORY_SIZE;
    } else {
        shell->current_size++;
    }
    
    shell->history[index] = my_strdup(command);
    shell->history_count++;
}




Development Setup
# Fork and clone the repository
git clone https://github.com/yourusername/enhanced-mini-shell.git
cd enhanced-mini-shell
it is highly recommended to use a UNIX-based OS like MacOs or Linux. WSL on windows will do the job. because of the sys/wait.h library which is available only on Linux or MacOs compilers.
# Create development branch
git checkout -b feature/your-feature-name






THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


⭐ Star this repository if you found it helpful!

Report Bug • Request Feature • Contribute

