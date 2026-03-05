#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define lenInput 100

int main()
{
    char userInput[lenInput];
    char lastChar;
    bool backgroundFlag = false;
    int stringLen = 0;

    printf("Input your command.\n");
    printf("To stop the mini shell, type \"exit\"\n");
    printf("Type \"?\" for help\n");

    while (1)
    {
        printf(">>>: ");

        if (fgets(userInput, lenInput, stdin) == NULL)
        {
            break;
        }

        stringLen = strlen(userInput);

        if (stringLen == 0)
        {
            continue;
        }

        // Remove the nextine(\n) in the command from the user
        userInput[stringLen - 1] = '\0';
        stringLen--;
        lastChar = userInput[stringLen - 1];

        if (strcmp(userInput, "exit") == 0 || strcmp(userInput, "EXIT") == 0)
        {
            break;
        }

        // check to see if the command should run in the background
        if (lastChar == '&')
        {
            backgroundFlag = true;
            userInput[stringLen - 1] = '\0';
            stringLen--;
        }

        if (strcmp(userInput, "?") == 0)
        {
            printf("\nHelp Menu:\n");
            printf("- Prefix external commands with \"!\"\n");
            printf("- Run background commands with \"&\"\n");
            printf("- Type \"exit\" to quit\n\n");
            printf("- cd,\n- pwd,\n- mkdir,\n- exit,\n- jobs\n");
        }

        else if (strstr(userInput, "cd"))
        {
        }

        else if (strstr(userInput, "pwd"))
        {
        }

        else if (strstr(userInput, "mkdir"))
        {
        }
        else if (strstr(userInput, "jobs"))
        {
        }
        else {
            printf("Unknown command. Use '?' for help or '!' for external commands\n");
        }
    }

    return 0;
}