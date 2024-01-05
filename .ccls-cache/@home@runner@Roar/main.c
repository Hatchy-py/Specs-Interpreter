#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VARIABLES 10

// Structure to store variables
struct Variable {
    char name[50];
    char value[256];
};

// Array to store variables
struct Variable variables[MAX_VARIABLES];
int variableCount = 0;

// Function to find a variable by name
struct Variable *findVariable(const char *name, size_t len) {
    for (int i = 0; i < variableCount; ++i) {
        if (memcmp(variables[i].name, name, len) == 0
        &&  variables[i].name[len] == '\0') {
            return &variables[i];
        }
    }
    return NULL; // Variable not found
}

void console_write(const char *s, size_t len) {
    // Print the message, replacing variables if present
    for (const char *p = s, *end = s + len; p < end; p++) {
        if (*p == '*' && ++p < end && *p != '*') {
            const char *varName = p;
            while (p < end && *p != '*') {
                ++p;
            }
            size_t nameLen = p - varName;
            struct Variable *variable = findVariable(varName, nameLen);
            if (variable != NULL) {
                printf("%s", variable->value);
            } else {
                fprintf(stderr, "Undefined variable: %.*s\n", (int)nameLen, varName);
            }
        } else {
            putchar(*p);
        }
    }
}

// Function to interpret commands
int interpretCommand(const char *line) {
    char current_value[250];

    *current_value = '\0';
    for (const char *command = line; *command;) {
        if (strncmp(command, "console:write->", 15) == 0) {
            // Extract the message within double quotes
            const char *messageStart = strchr(command, '"');
            const char *messageEnd = strrchr(command, '"');
            if (messageStart == NULL || messageEnd == NULL || messageStart >= messageEnd) {
                fprintf(stderr, "Invalid message format\n");
                return 1;
            }
            console_write(messageStart + 1, messageEnd - messageStart - 1);
            putchar('\n');
            command = messageEnd + 1;
        } else
        if (strncmp(command, "console:read->", 14) == 0) {
            // Extract the prompt within double quotes
            const char *promptStart = strchr(command, '"');
            const char *promptEnd = strrchr(command, '"');
            if (promptStart == NULL || promptEnd == NULL || promptStart >= promptEnd) {
                fprintf(stderr, "Invalid prompt format\n");
                return 1;
            }
            // Print the prompt and read user input
            console_write(promptStart + 1, promptEnd - promptStart - 1);
            // Read user input
            fgets(current_value, sizeof(current_value), stdin);
            size_t len = strlen(current_value);
            if (len > 0 && current_value[len - 1] == '\n')
                current_value[--len] = '\0';
            command = promptEnd + 1;
        } else
        if (!strncmp(command, "->", 2)) {
            // ->variableName
            const char *name = command + 2;
            size_t len = strlen(name);
            struct Variable *variable = findVariable(name, len);
            if (variable == NULL) {
                if (len >= sizeof(variable->name)) {
                    fprintf(stderr, "Variable name too long: %.*s\n", (int)len, name);
                    return 1;
                }
                if (variableCount >= MAX_VARIABLES) {
                    fprintf(stderr, "Maximum number of variables reached\n");
                    return 1;
                }
                memcpy(variables[variableCount].name, name, len);
                variables[variableCount].name[len] = '\0';
                variable = &variables[variableCount++];
            }
            strcpy(variable->value, current_value);
            command = name + len;
        } else
        if (*command == '\"') {
            // string value
            const char *valueStart = command;
            const char *valueEnd = strrchr(command, '"');
            if (valueStart >= valueEnd) {
                fprintf(stderr, "Unterminated value string: %s\n", command);
                return 1;
            }
            size_t len = valueEnd - valueStart - 1;
            if (len >= sizeof(current_value)) {
                fprintf(stderr, "Value string too long: %.*s\n", (int)len + 1, valueStart);
                return 1;
            }
            memcpy(current_value, valueStart + 1, len);
            current_value[len] = '\0';
            command = valueEnd + 1;
        } else {
            char *arrow = strstr(command, "->");
            if (arrow != NULL) {
                size_t len = arrow - command;
                struct Variable *variable = findVariable(command, len);
                if (variable == NULL) {
                    fprintf(stderr, "Unknown variable: %.*s\n", (int)len, command);
                    return 1;
                }
                strcpy(current_value, variable->value);
            } else {
                fprintf(stderr, "Unknown command: %s\n", command);
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // Open the file
    const char *filename = argc > 1 ? argv[1] : "King.roar";
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s: %s", filename, strerror(errno));
        return 1;
    }

    char line[256]; // Assuming a maximum line length of 255 characters

    // Read and interpret each line from the file
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Interpret the command
        if (interpretCommand(line))
            break;
    }

    // Close the file
    fclose(file);

    return 0;
}