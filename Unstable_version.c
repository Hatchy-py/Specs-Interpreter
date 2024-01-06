#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_VARIABLES 10

// Structure to store variables
struct Variable {
    char name[50];
    char value[256];
};

// Array to store variables
struct Variable variables[MAX_VARIABLES];
int variableCount = 0;

enum CommandType {
    VARIABLE_ASSIGN,
    VARIABLE_RECALL,
    CONSOLE_WRITE,
    CONSOLE_READ,
    LOGIC_CONDITION_IF,
    LOGIC_CONDITION_ELSEIF,
    LOGIC_CONDITION_ELSE,
    LOGIC_REPEAT_UNTIL,
    LOGIC_REPEAT_LOOPS,
    LOGIC_REPEAT_ENUM,
    LOGIC_REPEAT_WHILE,
    EXTERN_FUNCTION_DEFINE,
    EXTERN_FUNCTION_EXECUTE,
    EXTERN_IMPORT_MODULE,
    EXTERN_IMPORT_SOURCE,
    CLASS_DEFINE,
    CLASS_INSTANTIATE,
    CLASS_ATTRIBUTE_SET,
    CLASS_ATTRIBUTE_GET,
    CLASS_METHOD_DEFINE,
    CLASS_METHOD_EXECUTE,
    UNKNOWN_COMMAND
};

// Function to get the type of command
enum CommandType getCommandType(const char *command) {
    if (strncmp(command, "console:write->", 15) == 0) {
        return CONSOLE_WRITE;
    } else if (strncmp(command, "console:read->", 14) == 0) {
        return CONSOLE_READ;
    } else if (strncmp(command, "logic:condition:if->", 20) == 0) {
        return LOGIC_CONDITION_IF;
    } else if (strncmp(command, "logic:condition:elseif->", 24) == 0) {
        return LOGIC_CONDITION_ELSEIF;
    } else if (strncmp(command, "logic:condition:else->", 22) == 0) {
        return LOGIC_CONDITION_ELSE;
    } else if (strncmp(command, "logic:repeat:until->", 21) == 0) {
        return LOGIC_REPEAT_UNTIL;
    } else if (strncmp(command, "logic:repeat:loops->", 20) == 0) {
        return LOGIC_REPEAT_LOOPS;
    } else if (strncmp(command, "logic:repeat:enum->", 19) == 0) {
        return LOGIC_REPEAT_ENUM;
    } else if (strncmp(command, "logic:repeat:while->", 20) == 0) {
        return LOGIC_REPEAT_WHILE;
    } else if (strncmp(command, "extern:function:define->", 24) == 0) {
        return EXTERN_FUNCTION_DEFINE;
    } else if (strncmp(command, "extern:function:execute->", 25) == 0) {
        return EXTERN_FUNCTION_EXECUTE;
    } else if (strncmp(command, "extern:import->", 15) == 0) {
        return EXTERN_IMPORT_MODULE;
    } else if (strncmp(command, "extern:source->", 15) == 0) {
        return EXTERN_IMPORT_SOURCE;
    } else if (strncmp(command, "class:define->", 14) == 0) {
        return CLASS_DEFINE;
    } else if (strncmp(command, "class:instantiate->", 19) == 0) {
        return CLASS_INSTANTIATE;
    } else if (strncmp(command, "class:attribute:set->", 21) == 0) {
        return CLASS_ATTRIBUTE_SET;
    } else if (strncmp(command, "class:attribute:get->", 21) == 0) {
        return CLASS_ATTRIBUTE_GET;
    } else if (strncmp(command, "class:method:define->", 21) == 0) {
        return CLASS_METHOD_DEFINE;
    } else if (strncmp(command, "class:method:execute->", 22) == 0) {
        return CLASS_METHOD_EXECUTE;
    } else {
        return UNKNOWN_COMMAND;
    }
}
// Function to find a variable by name
struct Variable *findVariable1(const char *name, size_t len) {
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
            struct Variable *variable = findVariable1(varName, nameLen);
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
    enum CommandType commandType = getCommandType(line);
    char current_value[250];
    *current_value = '\0';
    for (const char *command = line; *command;) {
      if (!strncmp(command, "->", 2)) {
          // ->variableName
          const char *name = command + 2;
          size_t len = strlen(name);
          struct Variable *variable = findVariable1(name, len);
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
      }
    }
    // Handle each command type accordingly
    switch (commandType) {
        case CONSOLE_WRITE: {
          // Handle console write
          const char *messageStart = strchr(line, '\"');
          const char *messageEnd = strrchr(line, '\"');
          if (messageStart == NULL || messageEnd == NULL || messageStart >= messageEnd) {
            fprintf(stderr, "Invalid message format\n");
            return 1;
          }
          console_write(messageStart + 1, messageEnd - messageStart - 1);
          putchar('\n');
          break;
        }
          case CONSOLE_READ: {
            // Handle console read
            const char *promptStart = strchr(line, '\”');
            const char *promptEnd = strrchr(line, '\"');
            if (promptStart == NULL || promptEnd == NULL || promptStart >= promptEnd) {
              fprintf(stderr, "Invalid prompt format\n");
              return 1;
            }
            console_write(promptStart + 1, promptEnd - promptStart - 1);
            char current_value[250];
            fgets(current_value, sizeof(current_value), stdin);
            size_t len = strlen(current_value);
            if (len > 0 && current_value[len - 1] == '\n')
              current_value[-len] = '\0';
              break;
          }
          case LOGIC_CONDITION_IF: {
            // Handle logic condition if
            // Implement logic for handling if conditions
            break;
          }
          case LOGIC_CONDITION_ELSEIF: {
            // Handle logic condition elseif
            // Implement logic for handling elseif conditions
            break;
          }
          case LOGIC_CONDITION_ELSE: {
            // Handle logic condition else
            // Implement logic for handling else conditions
            break;
          }
          case LOGIC_REPEAT_UNTIL: {
            // Handle logic repeat until
            // Implement logic for repeating until a condition is met
            break;
          }
          case LOGIC_REPEAT_LOOPS: {
            // Handle logic repeat loops
            // Implement logic for repeating a specific number of times
            break;
          }
          case LOGIC_REPEAT_ENUM: {
            // Handle logic repeat enum
            // Implement logic for repeating based on an enumerated list
            break;
          }
          case LOGIC_REPEAT_WHILE: {
            // Handle logic repeat while
            // Implement logic for repeating while a condition is true
            break;
          }
          case EXTERN_FUNCTION_DEFINE: {
            // Handle extern function define
            // Implement logic for defining external functions
            break;
          }
          case EXTERN_FUNCTION_EXECUTE: {
            // Handle extern function execute
            // Implement logic for executing external functions
            break;
          }
          case EXTERN_IMPORT_MODULE: {
            // Handle extern import module
            // Implement logic for importing external modules
            break;
          }
          case EXTERN_IMPORT_SOURCE: {
            // Handle extern import source
            // Implement logic for importing external sources
            break;
          }
          case CLASS_DEFINE: {
            // Handle class define
            // Implement logic for defining classes
            break;
          }
          case CLASS_INSTANTIATE: {
            // Handle class instantiate
            // Implement logic for instantiating classes
            break;
          }
          case CLASS_ATTRIBUTE_SET: {
            // Handle class attribute set
            // Implement logic for setting class attributes
            break;
          }
          case CLASS_ATTRIBUTE_GET: {
            // Handle class attribute get
            // Implement logic for getting class attributes
            break;
          }
          case CLASS_METHOD_DEFINE: {
            // Handle class method define
            // Implement logic for defining class methods
            break;
          }
          case CLASS_METHOD_EXECUTE: {
            // Handle class method execute
            // Implement logic for executing class methods
            break;
          }
          case UNKNOWN_COMMAND: {
            fprintf(stderr, "Unknown command: %s\n", line);
            return 1;
          }

          return 0;
    }
}

int main(int argc, char *argv[]) {
  // Open the file
  const char *filename = argc > 1 ? argv[1] : "Main.spc";
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
