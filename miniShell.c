#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_JOBS 100

/***
pwd() retrieves the current working directory into a PATH_MAX-sized buffer using getcwd() and prints it to stdout as "Current working directory: ". If getcwd() fails it prints "Cannot get current working directory path" to stderr and returns.
***/
void pwd()
{
    // Buffer to store the current working directory path
    char buffer[PATH_MAX];
    // Attempt to retrieve the current working directory
    if (getcwd(buffer, PATH_MAX) == NULL)
    {
        // If getcwd fails, print an error message to stderr
        fprintf(stderr, "Cannot get current working directory path\n");
        return;
    }
    printf("Current working directory: %s\n", buffer);
}

// Structure that represents a background job
typedef struct {
    pid_t pid;                 // Process ID of the job
    char command[PATH_MAX];    // Command that started the process
    int is_active;             // 1 if the process is still running, 0 if it finished
} Job;

// Array that stores all jobs
Job jobList[MAX_JOBS];

// Number of jobs currently stored in the list
int jobCount = 0;


// Function to display all jobs (internal shell command like `jobs`)
void displayJobs() {

    // Print table header
    printf("\nID\tPID\tStatus\t\tCommand\n");

    // Loop through all stored jobs
    for (int i = 0; i < jobCount; i++) {

        // Print job information
        printf("[%d]\t%d\t%s\t%s\n", 
               i + 1,                     // Job ID (index + 1 for user-friendly numbering)
               jobList[i].pid,            // Process ID
               jobList[i].is_active ?     // Check if job is active
               "Running" : "Finished",    // Display status
               jobList[i].command);       // Command used to start the job
    }
}

int main()
{
    // Buffer that stores the raw command entered by the user
    char userInput[PATH_MAX];
    // lastChar: used to detect '&' for background execution
    // command: first token of the user input (the command itself)
    // argument: second token (argument to the command)
    char lastChar, *command, *argument;
    // Flag indicating whether the command should run in background
    bool backgroundFlag = false;
    // Stores the length of the user input string
    int stringLen = 0;

    printf("Input your command.\n");
    printf("To stop the mini shell, type \"exit\"\n");
    printf("Type \"?\" for help\n");

    while (1)
    {
        // Display the shell prompt
        printf(">>>: ");

        // Read user input from stdin
        if (fgets(userInput, PATH_MAX, stdin) == NULL)
        {
            // Exit loop if input fails
            break;
        }
        // Determine length of the input string
        stringLen = strlen(userInput);

        // If input is empty, skip to next iteration
        if (stringLen == 0)
        {
            continue;
        }

        // Remove the nextine(\n) in the command added by fgets
        userInput[stringLen - 1] = '\0';
        stringLen--;
        // Store the last character of the input (used to detect '&')
        lastChar = userInput[stringLen - 1];

        // The program is terminated if the user input either exit or EXIT
        if (strcmp(userInput, "exit") == 0 || strcmp(userInput, "EXIT") == 0)
        {
            break;
        }

        // check to see if the command should run in the background
        if (lastChar == '&')
        {
            backgroundFlag = true;
            userInput[stringLen - 1] = '\0';
        }

        // Split the user input into tokens using space as delimiter
        // First token is the command
        command = strtok(userInput, " ");
        argument = strtok(NULL, " ");

        // Compare the command entered by the user with supported commands
        // We use string compare to check what the command the user has inputted
        if (strcmp(command, "?") == 0)
        {
            printf("\nHelp Menu:\n");
            printf("- Prefix external commands with \"!\"\n");
            printf("- Run background commands with \"&\"\n");
            printf("- Type \"exit\" to quit\n\n");
            printf("- cd,\n- pwd,\n- mkdir,\n- exit,\n- jobs\n");
        }

        else if (strcmp(command, "cd") == 0)
        {
            // Check if the user has not inserted a path, if so the program output an error
            if (argument == NULL)
            {
                printf("Error: cd require a path\n");
            }
            // Execute the change directory method and also check for error
            else if (chdir(argument) != 0)
            {
                perror("cd failed");
                continue;
            }
            // If there is no error we output the new working directory
            pwd();
        }

        else if (strcmp(command, "pwd") == 0)
        {
            pwd();
        }

        else if (strcmp(command, "mkdir") == 0)
        {
            // Check if the user provided a directory name argument
            // If no argument is given, print an error message
            if (argument == NULL)
            {
                fprintf(stderr, "Error: mkdir requires a directory name\n");
            }
            // Attempt to create the new directory and set the standard read/write/execute (0777) permissions
            else if (mkdir(argument, 0777) != 0)
            {
                // If mkdir fails, print the system error message
                perror("mkdir failed");
            }
            // If the directory is successfully created, print confirmation
            else
            {
                printf("Directory '%s' created successfully.\n", argument);
            }
        }
        else if (strcmp(command, "jobs") == 0)
        {
            displayJobs();
        }
        else
        {
            printf("Unknown command. Use '?' for help or '!' for external commands\n");
        }
    }

    return 0;
}