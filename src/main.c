/*
 * wvsh: A simple shell implementation in C
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* --- Built-in Command Declarations --- */
int wvsh_cd(char **args);
int wvsh_help(char **args);
int wvsh_exit(char **args);

/* Struct definition linking string command names to their C function pointers */
typedef struct {
    const char *name;
    int (*func)(char **args);
    const char *description;
}BuiltinCommands;

/* Global registry of shell built-ins */
BuiltinCommands builtins[] = {
    {"cd", wvsh_cd, "Change the current directory"},
    {"help", wvsh_help, "Display this help message"},
    {"exit", wvsh_exit, "Exit the shell"}
};

/* 
 * Built-in: cd
 * Changes the process working directory using the chdir() system call.
 * Defaults to the $HOME environment variable if no path argument is provided.
 */
int wvsh_cd(char **args) {
    if(args[1] == NULL) {
        char *home = getenv("HOME");
        if(home == NULL) {
            fprintf(stderr, "wvsh: cd: HOME not set\n");
        } else {
            if(chdir(home) == -1) {
                perror("wvsh: cd");
            }
        }
    } else {
        if(chdir(args[1]) == -1) {
            perror("wvsh: cd");
        }
    }
    return 1;
}
/* 
 * Built-in: help
 * Displays a simple help message listing built-in commands and their descriptions.
 */
int wvsh_help(char **args) {
    printf("wvsh: A simple shell implementation in C\n");
    printf("Built-in commands:\n");
    for(int i = 0; i < sizeof(builtins) / sizeof(BuiltinCommands); i++) {
        printf("  %s: %s\n", builtins[i].name, builtins[i].description);
    }
    return 1;
}
/* 
 * Built-in: exit
 * Exits the shell by returning 0, which signals the main loop to terminate.
 */
int wvsh_exit(char **args) {
    return 0;
}



/* 
 * Reads a line of input from the user. Uses getline() if available, otherwise falls back to a custom implementation.
 * Handles EOF and allocation errors gracefully.
 */
char *wvsh_read_line() {
    #ifdef DWVSH_USE_STD_GETLINE
        char *line = NULL;
        size_t bufsize = 0; 
        if(getline(&line, &bufsize, stdin) == -1) {
            if(feof(stdin)) {
                exit(EXIT_SUCCESS); 
            } else {
                perror("wvsh: getline");
                exit(EXIT_FAILURE);
            }
        }
            return line;
        #else
        #define DWVSH_READ_CHUNK_SIZE 1024
        int capacity = DWVSH_READ_CHUNK_SIZE;
        int position = 0;
        char *buffer= malloc(sizeof(char) * capacity);
        int ch;

        if(!buffer) {
            fprintf(stderr, "wvsh: allocation error\n");
            exit(EXIT_FAILURE);
        }
        while(1) {
    
            ch = fgetc(stdin);  // Read a character from standard input

            if (ch == EOF) {
                if(position == 0) {
                    exit(EXIT_SUCCESS); // Exit if EOF is encountered at the beginning of input
                } else {
                    break; // Break the loop if EOF is encountered after some input
                }
            } else if(ch == '\n') {
                break; // Break the loop if a newline character is encountered, indicating end of input
            } else {
                buffer[position] = ch; // Store the read character in the buffer
            }
            position++;

            // Dynamically resize the buffer if we've exceeded the current capacity
            if(position >= capacity) {
                capacity += DWVSH_READ_CHUNK_SIZE;
                buffer = realloc(buffer, sizeof(char) * capacity);                                                                                                                                                                                                                                                                  
                if(!buffer) {
                    fprintf(stderr, "wvsh: allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        buffer[position] = '\0'; // Null-terminate the string
        return buffer;
        #endif

}

#define DWVSH_TOKEN_BUFSIZE 64
#define DWVSH_TOKEN_DELIMITERS " \t\r\n\a"

/* 
 * Tokenizes a line of input into an array of strings (tokens) based on defined delimiters.
 * Dynamically resizes the token array as needed and handles allocation errors.
 */

char **wvsh_tokenize_line(char *line) {
    int capacity = DWVSH_TOKEN_BUFSIZE;
    int position = 0;
    char **tokens = malloc(sizeof(char *) * capacity);
    char *token;

    // Check for allocation failure
    if(!tokens) {
        fprintf(stderr, "wvsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, DWVSH_TOKEN_DELIMITERS); // Get the first token


    // Continue tokenizing the input line until there are no more tokens
    while(token != NULL) {
        if(position >= capacity) {
            capacity += DWVSH_TOKEN_BUFSIZE;
            tokens = realloc(tokens, sizeof(char *) * capacity);


        // Check for allocation failure during resizing
            if(!tokens) {
                fprintf(stderr, "wvsh: allocation error\n");
                exit(EXIT_FAILURE);
            }

        }

        tokens[position] = token;   
        position++;       

        token = strtok(NULL, DWVSH_TOKEN_DELIMITERS);    // Get the next token

    }
    tokens[position] = NULL;               
    return tokens;
}

/* 
 * Executes a command by first checking if it matches any built-in commands, and if not, it forks a child process to execute the command using execvp().
 * Handles errors in forking and executing, and waits for the child process to finish before returning.
 */
int wvsh_execute_command(char **args) {
    if(args[0] == NULL) {
        return 1;
    }

    // Check if the command matches any built-in commands
    for(int i = 0; i < sizeof(builtins) / sizeof(BuiltinCommands); i++) {
        if(strcmp(args[0], builtins[i].name) == 0) {
            return builtins[i].func(args);
        }
    }

    // If not a built-in command, fork a child process to execute the command
    pid_t pid = fork();
    if(pid < 0) {
        perror("wvsh: fork");
        exit(EXIT_FAILURE);
    } else if(pid == 0) {

        // Child Process Context: Replace the child process image with the command to be executed
        if(execvp(args[0], args) == -1) {
            perror("wvsh: execvp");
        }
        exit(EXIT_FAILURE);
    }
    waitpid(pid, NULL, 0);  // Parent Process Context: Block and wait for the child process to finish
    return 1;
}




/* 
 * Main loop of the shell: Continuously prompts the user for input, reads a line, tokenizes it, and executes the command until the user exits.
 * Handles memory cleanup for each iteration and checks the status returned by command execution to determine when to exit.
 */
int main() {
    char *line;
    char **args;
    int status;

    while(1) {
        printf("wvsh >> ");
        line = wvsh_read_line(); 
        args = wvsh_tokenize_line(line);
        status = wvsh_execute_command(args);
        
        /* Free the dynamically allocated memory for the input line and the token array after each command execution to prevent memory leaks. */
        free(line);
        free(args);

        if(status == 0) {
            break;
        }
    }
    return EXIT_SUCCESS;
}