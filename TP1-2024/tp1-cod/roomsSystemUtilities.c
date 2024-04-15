#include "roomsSystemUtilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to check if the command is valid
int checkCommand(char* text) {
    // List of valid strings
    char* validStrings[] = {"register", "init", "shutdown", "update", "load", "kill"};
    // Find the first space in the text
    char* space = strchr(text, ' ');
    if (space != NULL) {
        // Calculate the length of the text before the first space
        int length = space - text;
        // Create a temporary string to store the text before the first space
        char temp[length + 1];
        strncpy(temp, text, length);
        temp[length] = '\0';
        // Check if the temporary string belongs to the valid strings list
        for (int i = 0; i < sizeof(validStrings) / sizeof(validStrings[0]); i++) {
            if (strcmp(temp, validStrings[i]) == 0) {
                return 1;  // Text belongs to the valid strings list
            }
        }
    } else {
        // Check if the text is equal to "kill"
        if (strcmp(text, "kill") == 0) {
            return 1;  // Text is a valid command
        }
    }
    return -1;  // Text does not belong to the valid strings list
}

// Function to execute the command based on the input string
void executeCommand(char* command, Classroom* classrooms) {
    char* commandBase = getNthCommand(command, 1);

    if (strcmp(commandBase, "register") == 0) {
        char* roomID = getNthCommand(command, 2);
        createRoomIfNotExists(classrooms, atoi(roomID));
        printClassrooms(classrooms);

        // Call the register function
        // registerFunction();
    } else if (strcmp(commandBase, "init") == 0) {
        // Get the second word after the space
        char* secondCommand = getNthCommand(command, 2);
        if (strcmp(secondCommand, "file") == 0) {
            char* fileName = getNthCommand(command, 3);
            char* fileData = readFileContents(fileName);
            fprintf(stderr, "File data recovered - %s\n", fileData);
        } else {
            // createRoomIfNotExists(classrooms, getInputDataFromString(command, 3));
            printClassrooms(classrooms);
        }

        // Call the init function
        // initFunction();
    } else if (strcmp(commandBase, "shutdown") == 0) {
        // Call the shutdown function
        // shutdownFunction();
    } else if (strcmp(commandBase, "update") == 0) {
        // Call the update function
        // updateFunction();
    } else if (strcmp(commandBase, "load") == 0) {
        // Call the load function
        // loadFunction();
    } else {
        printf("Invalid command\n");
    }
}
// TODO NEED TO REVISE CASE MULTIPLE COMMANDS ON A FILE
//  Function to read and return the contents of a file
char* readFileContents(char* fileName) {
    // Remove the newline character from the fileName
    fileName[strcspn(fileName, "\n")] = '\0';

    // Open the file in read mode
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return NULL;
    }

    // Read the contents of the file
    char* contents = malloc(1);
    contents[0] = '\0';
    char line[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove the newline character from the line
        line[strcspn(line, "\n")] = '\0';

        // Append the line to the contents string
        contents = realloc(contents, strlen(contents) + strlen(line) + 2);
        strcat(contents, line);
        strcat(contents, " ");
    }

    // Close the file
    fclose(file);

    return contents;
}

// Function to get the nth word from a string
char* getNthCommand(char* text, int n) {
    // Create a copy of the input string
    char* copy = strdup(text);
    // Tokenize the string using space as the delimiter
    char* token = strtok(copy, " ");
    // Iterate through the tokens until the nth word is reached
    for (int i = 1; i < n; i++) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            free(copy);
            return NULL;  // Return NULL if the nth word does not exist
        }
    }
    // Return the nth word
    char* result = strdup(token);
    free(copy);
    return result;
}

// Function to get the input data from the string of command
char* getInputDataFromString(char* data, int n) {
    // Create a copy of the input string
    char* copy = strdup(data);
    // Tokenize the string using space as the delimiter
    char* token = strtok(copy, " ");
    // Iterate through the tokens until the nth space is reached
    for (int i = 1; i < n; i++) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            free(copy);
            return NULL;  // Return NULL if the nth space does not exist
        }
    }
    // Get the rest of the string starting from the nth space
    char* result = strdup(token);
    free(copy);
    return result;
}

void createRoomIfNotExists(Classroom* classrooms, int roomId) {
    // Check if a room with the given roomId already exists
    int numrooms = sizeof(*classrooms) / sizeof(classrooms[0]);
    int sizeff = sizeof(classrooms[0]);
    int sizefq = sizeof(*classrooms);
    int sizeoff = sizeof(*classrooms);
    int sizeo1 = sizeof(classrooms);
    int sizeof0 = sizeof(&classrooms[0]);
    int sizeof1 = sizeof(classrooms[0]);
    if (classrooms != NULL) {
        for (int i = 0; i < (sizeof(*classrooms) / sizeof(classrooms[0])); i++) {
            if (classrooms[i].roomID == roomId) {
                return;  // Room already exists, no need to create a new one
            }
        }
    }

    // Create a new room with the given roomId and default values
    Classroom newClassroom;
    newClassroom.roomID = roomId;
    newClassroom.temperature = 0.0;
    newClassroom.humidity = 0.0;

    // Initialize the fans in the new room
    for (int i = 0; i < sizeof(newClassroom.fans) / sizeof(newClassroom.fans[0]); i++) {
        newClassroom.fans[i].id = i + 1;
        newClassroom.fans[i].state = 0;
    }

    // Add the new room to the rooms array
    int numClassrooms = sizeof(*classrooms) / sizeof(classrooms[0]);
    classrooms = realloc(classrooms, (numClassrooms + 1) * sizeof(Classroom));
    classrooms[numClassrooms] = newClassroom;
}

// Function to print the data of each Classroom struct
void printClassrooms(Classroom* classrooms) {
    int numrooms = sizeof(*classrooms) / sizeof(classrooms[0]);
    int sizeoff = sizeof(*classrooms);
    int sizeof0 = sizeof(&classrooms[0]);
    int sizeof1 = sizeof(classrooms)[0];
    for (int i = 0; i < (sizeof(*classrooms) / sizeof(classrooms[0])); i++) {
        fprintf(stderr, "%d %.2f %.2f\n", classrooms[i].roomID, classrooms[i].temperature, classrooms[i].humidity);
    }
}