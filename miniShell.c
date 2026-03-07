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
#include <linux/limits.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_JOBS 100
#define ARGS_LIMIT 10

/*
pwd() retrieves the current working directory into a PATH_MAX-sized buffer using getcwd() and prints it to stdout as "Current working directory: ". If getcwd() fails it prints "Cannot get current working directory path" to stderr and returns.
*/
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
typedef struct
{
    int pid;                // Process ID of the job
    char command[PATH_MAX]; // Command that started the process
    int is_active;          // 1 if the process is still running, 0 if it finished
} Job;

// Array that stores all jobs
Job jobList[MAX_JOBS];

// Number of jobs currently stored in the list
int jobCount = 0;

// Function to display all jobs (internal shell command like `jobs`)
void displayJobs()
{

    // Print table header
    printf("\nID\tPID\tStatus\t\tCommand\n");

    // Loop through all stored jobs
    for (int i = 0; i < jobCount; i++)
    {

        // Print job information
        printf("[%d]\t%d\t%s\t%s\n",
               i + 1,                 // Job ID (index + 1 for user-friendly numbering)
               jobList[i].pid,        // Process ID
               jobList[i].is_active ? // Check if job is active
                   "Running"
                                    : "Finished", // Display status
               jobList[i].command);               // Command used to start the job
    }
}

// Structure used to store the command and its arguments for a background task
typedef struct
{
    int jobIndex;
    char command[PATH_MAX];         // Command to execute
    char *argument[ARGS_LIMIT + 1]; // Array of arguments passed to execvp (NULL terminated)
} Threads;

// Buffer that stores background jobs waiting to be executed
Threads jobBuffer[MAX_JOBS];

// Signal handler that reaps finished child processes
// wait(NULL) collects the exit status of any finished child
void handler(int sig)
{
    wait(NULL);
}

// Function executed by a thread to run a background command
void *runBackground(void *arg)
{
    // Cast the generic pointer to the Threads structure
    Threads *data = arg;
    // Create a new child process
    int pid = fork();

    // Child process
    if (pid == 0)
    {
        // Replace the child process with the requested command
        execvp(data->command, data->argument);

        // If execvp returns, execution failed
        perror("execvp failed");
        exit(1);
    }

    // Parent process
    else
    {
        //Update the list of jobs
        jobList[data->jobIndex].pid = pid;
        // Wait for the child process to finish execution
        wait(NULL);
        //When the job is finished, we update the status
        jobList[data->jobIndex].is_active = 0;
        // Notify that the background job has completed
        printf("\n[Background job finished: %s]\n", data->command);
    }
    // Return NULL because the thread function must return a void*
    return NULL;
}

int main()
{
    // Buffer that stores the raw command entered by the user
    char userInput[PATH_MAX];
    // lastChar: used to detect '&' for background execution
    // command: pointer to the main command entered by the user
    // argument: array of pointers to store the command arguments
    // ARGS_LIMIT defines the maximum number of arguments allowed
    char lastChar, *command, *argument[ARGS_LIMIT];
    // Stores the length of the user input string
    int stringLen = 0;

    // Declare a sigaction structure used to configure the signal handler
    struct sigaction sa;
    // Assign the function 'handler' as the signal handler
    // This function will be executed whenever the signal is received
    sa.sa_handler = handler;
    // Initialize the signal mask to empty
    // This means no additional signals will be blocked while the handler runs
    sigemptyset(&sa.sa_mask);
    // Set signal handling flags
    // SA_RESTART automatically restarts interrupted system calls (like read, fgets, etc.)
    sa.sa_flags = SA_RESTART;
    // Register the signal handler for SIGCHLD
    // SIGCHLD is sent to the parent process when a child process terminates
    sigaction(SIGCHLD, &sa, NULL);

    printf("Input your command.\n");
    printf("To stop the mini shell, type \"exit\"\n");
    printf("Type \"?\" for help\n");

    while (1)
    {
        // Flag indicating whether the command should run in background
        bool backgroundFlag = false;
        int argsLen = 0;
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
        // Continue splitting the rest of the input into arguments
        // Each token is stored in the argument array
        while ((argument[argsLen] = strtok(NULL, " ")) != NULL)
        {
            argsLen++; // Move to the next argument position
        }

        // Compare the command entered by the user with supported commands
        // We use string compare to check what the command the user has inputted
        if (strcmp(command, "?") == 0)
        {
            printf("\nHelp Menu:\n");
            printf("- Prefix external commands with \"!\"\n");
            printf("- Run background commands with \"&\"\n");
            printf("- Type \"exit\" to quit\n");
            printf("- cd,\n- pwd,\n- mkdir,\n- exit,\n- jobs\n");
        }

        else if (strcmp(command, "cd") == 0)
        {
            // Check if the user has not inserted a path, if so the program output an error
            if (argument[0] == NULL)
            {
                printf("Error: cd require a path\n");
            }
            // Execute the change directory method and also check for error
            else if (chdir(argument[0]) != 0)
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
            if (argument[0] == NULL)
            {
                fprintf(stderr, "Error: mkdir requires a directory name\n");
            }
            // Attempt to create the new directory and set the standard read/write/execute (0777) permissions
            else if (mkdir(argument[0], 0777) != 0)
            {
                // If mkdir fails, print the system error message
                perror("mkdir failed");
            }
            // If the directory is successfully created, print confirmation
            else
            {
                printf("Directory '%s' created successfully.\n", argument[0]);
            }
        }
        else if (strcmp(command, "jobs") == 0)
        {
            displayJobs();
        }
        // Check if the command starts with '!' (used to execute a system command)
        else if (command[0] == '!')
        {
            // Skip the '!' character to get the real command
            char *newCommand = command + 1;

            // Create the argument list that will be passed to execvp
            // execvp expects an array where:
            // args[0] = program name
            // args[1..n] = arguments
            // last element must be NULL
            char *args[12];
            args[0] = newCommand;
            // Copy all user-provided arguments into the args array
            for (int j = 0; j < argsLen; j++)
            {
                args[j + 1] = argument[j];
            }
            args[argsLen + 1] = NULL; // Terminate the array with NULL as required by execvp

            // Check if the command should run in the background
            if (backgroundFlag)
            {
                // Determine the slot in the job buffer using modulo to avoid overflow
                // This allows the buffer to wrap around if MAX_JOBS is reached
                int slot = jobCount % MAX_JOBS;
                
                //
                jobBuffer[slot].jobIndex = slot;
                // Copy the command name into the job buffer
                // strncpy is used to avoid overflowing the command array
                strncpy(jobBuffer[slot].command, newCommand, PATH_MAX);

                // Copy the argument list into the job buffer
                // The loop copies all arguments including the terminating NULL
                for (int k = 0; k <= argsLen + 1; k++){
                    jobBuffer[slot].argument[k] = args[k];
                }

                //add the job inside the joblist so that it can be displayed if jobs is run
                jobList[slot].is_active = 1; // 1 = Running
                strncpy(jobList[slot].command, newCommand, PATH_MAX);
                // Create a new thread that will run the background job
                pthread_t thread;
                pthread_create(&thread, NULL, runBackground, &jobBuffer[slot]);

                // Detach the thread so it cleans up automatically when it finishes
                // This avoids needing pthread_join()
                pthread_detach(thread);

                // Inform the user that the job has started in the background
                printf("[Job started in background]\n");

                // Increment the total number of jobs started
                jobCount++;
            }
            else
            {
                // Create a new process
                int pid = fork();

                // Check if fork failed
                if (pid < 0)
                {
                    perror("fork failed"); // Print error message
                }
                // Child process
                else if (pid == 0)
                {
                    // Replace the child process with the new program
                    execvp(newCommand, args);

                    // If execvp returns, it means it failed
                    perror("execvp failed");
                    exit(1);
                }
                // Parent process
                else
                {
                    // Wait for the child process to finish execution
                    wait(NULL);
                }
            }
        }
        else
        {
            printf("Unknown command. Use '?' for help or '!' for external commands\n");
        }
    }

    return 0;
}