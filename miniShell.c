#include <stdio.h>
#include <string.h>

int main() {
    char userInput[100];

    printf("Input your command.\n");
    printf("To stop the mini shell, type \"exit\"\n");
    printf("Type \"?\" for help\n");

    while (1) {
        printf(">>>: ");
        
        // Read input and limit to 99 chars to prevent overflow
        if (scanf("%99s", userInput) != 1) {
            break;
        }

        if (strcmp(userInput, "exit") == 0) {
            break;
        } 
        else if (strcmp(userInput, "?") == 0) {
            printf("\nHelp Menu:\n");
            printf("- Prefix external commands with \"!\"\n");
            printf("- Run background commands with \"&\"\n");
            printf("- Type \"exit\" to quit\n\n");
        }
    }

    return 0;
}