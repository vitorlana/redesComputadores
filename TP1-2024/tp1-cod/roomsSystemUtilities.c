#include "roomsSystemUtilities.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initializes the array of classrooms with default values.
 *
 * @param classrooms The array of classrooms to be initialized.
 */
void initializeClassrooms(Classroom* classrooms) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        classrooms[i].roomID = -999;
        classrooms[i].temperature = -999;
        classrooms[i].humidity = -999;
        classrooms[i].fans[0].id = 1;
        classrooms[i].fans[0].state = -999;
        classrooms[i].fans[1].id = 2;
        classrooms[i].fans[1].state = -999;
        classrooms[i].fans[2].id = 3;
        classrooms[i].fans[2].state = -999;
        classrooms[i].fans[3].id = 4;
        classrooms[i].fans[3].state = -999;
    }
}

/**
 * Converts error and success codes to their corresponding messages.
 *
 * @param command The error or success code to be converted.
 * @return The corresponding error or success message.
 */
char* convertErrosAndSuccess(char* inputParameter) {
    if (strcmp(inputParameter, "OK_01") == 0) {
        return "sala instanciada com sucesso";
    } else if (strcmp(inputParameter, "OK_02") == 0) {
        return "sensores inicializados com sucesso";
    } else if (strcmp(inputParameter, "OK_03") == 0) {
        return "sensores desligados com sucesso";
    } else if (strcmp(inputParameter, "OK_04") == 0) {
        return "informações atualizadas com sucesso";
    } else if (strcmp(inputParameter, "ERROR_01") == 0) {
        return "sala inválida";
    } else if (strcmp(inputParameter, "ERROR_02") == 0) {
        return "sala já existe";
    } else if (strcmp(inputParameter, "ERROR_03") == 0) {
        return "sala inexistente";
    } else if (strcmp(inputParameter, "ERROR_04") == 0) {
        return "sensores inválidos";
    } else if (strcmp(inputParameter, "ERROR_05") == 0) {
        return "sensores já instalados";
    } else if (strcmp(inputParameter, "ERROR_06") == 0) {
        return "sensores não instalados";
    } else if (strcmp(getNthCommand(inputParameter, 1), "INF_RES") == 0) {
        char message[499] = "salas: ";
        return strcat(message, getInputDataFromString(inputParameter, 2));
    } else if (strcmp(getNthCommand(inputParameter, 1), "SAL_RES") == 0) {
        char message[499] = "sala ";
        return strcat(message, getInputDataFromString(inputParameter, 1));
    } else {
        return inputParameter;
    }
}

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
char* executeCommand(char* command, Classroom* classrooms) {
    char* commandBase = getNthCommand(command, 1);

    if (strcmp(commandBase, "CAD_REQ") == 0) {
        char* roomID = getNthCommand(command, 2);
        return (createRoomIfNotExists(classrooms, atoi(roomID)));

    } else if (strcmp(commandBase, "INI_REQ") == 0) {
        char* roomID = getNthCommand(command, 2);
        char* sensorsInputs = getInputDataFromString(command, 2);

        return turnOnSensorsValues(classrooms, atoi(roomID), sensorsInputs);
    } else if (strcmp(commandBase, "DES_REQ") == 0) {
        char* roomID = getNthCommand(command, 2);
        return shutdownSensorsOfClassroom(classrooms, atoi(roomID));
    } else if (strcmp(commandBase, "ALT_REQ") == 0) {
        char* roomID = getNthCommand(command, 2);
        char* sensorsInputs = getInputDataFromString(command, 2);

        return updateSensorsValues(classrooms, atoi(roomID), sensorsInputs);

    } else if (strcmp(commandBase, "SAL_REQ") == 0) {
        char* roomID = getNthCommand(command, 2);
        return printClassroom(classrooms, atoi(roomID));
    } else if (strcmp(commandBase, "INF_REQ") == 0) {
        return printClassrooms(classrooms);
    } else {
        return NULL;
    }
}

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

    // Remove the last space from the contents
    if (strlen(contents) > 0) {
        contents[strlen(contents) - 1] = '\0';
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
    // Find the nth space in the data string
    char* space = data;
    for (int i = 0; i < n; i++) {
        space = strchr(space, ' ');
        if (space == NULL) {
            return NULL;  // Return NULL if the nth space does not exist
        }
        space++;  // Move to the next character after the space
    }
    // Return the substring starting from the nth space
    return space;
}

char* shutdownSensorsOfClassroom(Classroom* classrooms, int roomId) {
    // Check if the room exists and installed
    int roomExistsAndInstalled = checkRoomsSensorsAreInstalled(classrooms, roomId);

    if (roomExistsAndInstalled == -2) {
        // Update the sensor values
        classrooms[roomId].temperature = -1;
        classrooms[roomId].humidity = -1;
        classrooms[roomId].fans[0].state = -1;
        classrooms[roomId].fans[1].state = -1;
        classrooms[roomId].fans[2].state = -1;
        classrooms[roomId].fans[3].state = -1;
        return "OK_03";

    } else if (roomExistsAndInstalled == -1) {
        return "ERROR_03";
    } else if (roomExistsAndInstalled == 1) {
        return "ERROR_06";
    } else {
        return NULL;
    }
}

char* createRoomIfNotExists(Classroom* classrooms, int roomId) {
    // Check if the room already exists
    if (checkRoomCreated(classrooms, roomId) == 1)
        return "ERROR_02";
    else {
        classrooms[roomId].roomID = roomId;
        classrooms[roomId].temperature = -1;
        classrooms[roomId].humidity = -1;
        classrooms[roomId].fans[0].id = 1;
        classrooms[roomId].fans[0].state = -1;
        classrooms[roomId].fans[1].id = 2;
        classrooms[roomId].fans[1].state = -1;
        classrooms[roomId].fans[2].id = 3;
        classrooms[roomId].fans[2].state = -1;
        classrooms[roomId].fans[3].id = 4;
        classrooms[roomId].fans[3].state = -1;
        return "OK_01";
    }
}

int checkRoomCreated(Classroom* classrooms, int roomId) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (classrooms[i].roomID == roomId) {
            return 1;  // Room already exists, no need to create
        }
    }
    return -1;  // Room does not exist
}

// Function to check if the values of a room are -1
int checkRoomsSensorsAreInstalled(Classroom* classrooms, int roomId) {
    // Check if the room exists
    int roomExists = checkRoomCreated(classrooms, roomId);

    if (roomExists == 1) {
        // Check if the temperature and humidity values are -1
        if (classrooms[roomId].temperature == -1 && classrooms[roomId].humidity == -1 && classrooms[roomId].fans[0].state == -1 &&
            classrooms[roomId].fans[1].state == -1 && classrooms[roomId].fans[2].state == -1 && classrooms[roomId].fans[3].state == -1) {
            return 1;  // Values are -1 (sensors are created)
        } else {
            return -2;  // Values are not -1 and initialized (sensors used)
        }

    } else if (roomExists == -1) {
        return -1;  // Room does not exist
    } else {
        return 0;
    }
}

char* turnOnSensorsValues(Classroom* classrooms, int roomId, char* sensorValues) {
    // Check if the room exists and installed
    int roomExistsAndInstalled = checkRoomsSensorsAreInstalled(classrooms, roomId);

    if (roomExistsAndInstalled == 1) {
        // Update the sensor values
        classrooms[roomId].temperature = atoi(getNthCommand(sensorValues, 1));
        classrooms[roomId].humidity = atoi(getNthCommand(sensorValues, 2));
        classrooms[roomId].fans[0] = getFanData(getNthCommand(sensorValues, 3));
        classrooms[roomId].fans[1] = getFanData(getNthCommand(sensorValues, 4));
        classrooms[roomId].fans[2] = getFanData(getNthCommand(sensorValues, 5));
        classrooms[roomId].fans[3] = getFanData(getNthCommand(sensorValues, 6));
        return "OK_02";

    } else if (roomExistsAndInstalled == -1) {
        return "ERROR_03";
    } else if (roomExistsAndInstalled == -2) {
        return "ERROR_05";
    } else {
        return NULL;
    }
}

char* updateSensorsValues(Classroom* classrooms, int roomId, char* sensorValues) {
    // Check if the room exists and installed
    int roomExistsAndInstalled = checkRoomsSensorsAreInstalled(classrooms, roomId);

    if (roomExistsAndInstalled == -2) {
        // Update the sensor values
        classrooms[roomId].temperature = atoi(getNthCommand(sensorValues, 1));
        classrooms[roomId].humidity = atoi(getNthCommand(sensorValues, 2));
        classrooms[roomId].fans[0] = getFanData(getNthCommand(sensorValues, 3));
        classrooms[roomId].fans[1] = getFanData(getNthCommand(sensorValues, 4));
        classrooms[roomId].fans[2] = getFanData(getNthCommand(sensorValues, 5));
        classrooms[roomId].fans[3] = getFanData(getNthCommand(sensorValues, 6));
        return "OK_04";

    } else if (roomExistsAndInstalled == -1) {
        return "ERROR_03";
    } else if (roomExistsAndInstalled == 1) {
        return "ERROR_06";
    } else {
        return NULL;
    }
}

// Function to check if any room was created
int checkAnyRoomCreated(Classroom* classrooms) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        Classroom room = classrooms[i];
        if (classrooms[i].roomID != -999) {
            return 1;  // Room exists
        }
    }
    return -1;  // No room exists
}

// Function to concatenate all data for a Classroom
char* createClassroomStringData(Classroom* classroom, bool withParenteses) {
    char* data = malloc(sizeof(char[100]));
    if (classroom->temperature != -1 && withParenteses) {
        sprintf(data, "%d (%d %d %d%d %d%d %d%d %d%d)", classroom->roomID, classroom->temperature, classroom->humidity, classroom->fans[0].id,
                classroom->fans[0].state, classroom->fans[1].id, classroom->fans[1].state, classroom->fans[2].id, classroom->fans[2].state,
                classroom->fans[3].id, classroom->fans[3].state);
    } else if (classroom->temperature == -1 && withParenteses) {
        sprintf(data, "%d (%d %d %d %d %d %d)", classroom->roomID, classroom->temperature, classroom->humidity, classroom->fans[0].state,
                classroom->fans[1].state, classroom->fans[2].state, classroom->fans[3].state);
    } else if (classroom->temperature != -1 && !withParenteses) {
        sprintf(data, "%d: %d %d %d%d %d%d %d%d %d%d", classroom->roomID, classroom->temperature, classroom->humidity, classroom->fans[0].id,
                classroom->fans[0].state, classroom->fans[1].id, classroom->fans[1].state, classroom->fans[2].id, classroom->fans[2].state,
                classroom->fans[3].id, classroom->fans[3].state);
    } else if (classroom->temperature == -1 && !withParenteses) {
        sprintf(data, "%d: %d %d %d %d %d %d", classroom->roomID, classroom->temperature, classroom->humidity, classroom->fans[0].state,
                classroom->fans[1].state, classroom->fans[2].state, classroom->fans[3].state);
    }
    return data;
}

// Function to print the data of Classroom struct
char* printClassroom(Classroom* classrooms, int classroomID) {
    int roomExistsAndInstalled = checkRoomsSensorsAreInstalled(classrooms, classroomID);

    if (roomExistsAndInstalled == -2) {
        char* payload = malloc(sizeof(char[500]));
        strcat(payload, "SAL_RES ");
        strcat(payload, createClassroomStringData(&classrooms[classroomID], false));

        return payload;
    } else if (roomExistsAndInstalled == -1) {
        return "ERROR_03";
    } else if (roomExistsAndInstalled == 1) {
        return "ERROR_06";
    } else {
        return NULL;
    }
}

// Function to print the data of each Classroom struct
char* printClassrooms(Classroom* classrooms) {
    int roomsExists = checkAnyRoomCreated(classrooms);

    if (roomsExists == -1) {
        return "ERROR_03";
    } else {
        char* payload = malloc(sizeof(char[500]));
        strcat(payload, "INF_RES ");

        for (int i = 0; i < MAX_ROOMS; i++) {
            if (classrooms[i].roomID != -999) {
                strcat(payload, " ");
                strcat(payload, createClassroomStringData(&classrooms[i], true));
            }
        }
        return payload;
    }
}

int checkInputRegisterRoom(char* roomIDParameter) {
    int roomID = atoi(roomIDParameter);
    if (roomID >= 0 && roomID < MAX_ROOMS) {
        return 1;
    } else {
        fprintf(stderr, "%s\n", convertErrosAndSuccess("ERROR_01"));
        return -1;
    }
}

Fan getFanData(char* fanData) {
    Fan fan;
    fan.id = fanData[0] - '0';
    fan.state = fanData[1] - '0';
    return fan;
}

int checkFanData(int fanPos, Fan fan) {
    if (fan.id == fanPos && fan.state >= 0 && fan.state <= 2) {
        return 1;
    } else {
        return -1;
    }
}

// Function to count the number of spaces in a string
int countSpaces(char* text) {
    int count = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == ' ') {
            count++;
        }
    }
    return count;
}

int checkSensorsInputs(char* inputsParameter) {
    if (countSpaces(inputsParameter) != 6) {
        fprintf(stderr, "%s\n", convertErrosAndSuccess("ERROR_04"));
        return -1;
    }

    int temp = atoi(getNthCommand(inputsParameter, 2));
    int moist = atoi(getNthCommand(inputsParameter, 3));
    Fan fan1 = getFanData(getNthCommand(inputsParameter, 4));
    Fan fan2 = getFanData(getNthCommand(inputsParameter, 5));
    Fan fan3 = getFanData(getNthCommand(inputsParameter, 6));
    Fan fan4 = getFanData(getNthCommand(inputsParameter, 7));

    if (temp >= 0 && temp <= 40 && moist >= 0 && moist <= 100 && checkFanData(1, fan1) == 1 && checkFanData(2, fan2) == 1 && checkFanData(3, fan3) == 1 &&
        checkFanData(4, fan4) == 1) {
        return 1;
    } else {
        fprintf(stderr, "%s\n", convertErrosAndSuccess("ERROR_04"));
        return -1;
    }
}

char* checkInputAndCreatePayload(char* inputParameter) {
    // Remove the newline character from inputParameter
    inputParameter[strcspn(inputParameter, "\n")] = '\0';
    char* input = getNthCommand(inputParameter, 1);

    if (strcmp(input, "register") == 0) {
        char* roomID = getNthCommand(inputParameter, 2);
        if (checkInputRegisterRoom(roomID) == 1) {
            char* payload = malloc(strlen("CAD_REQ ") + strlen(roomID) + 1);
            strcpy(payload, "CAD_REQ ");
            strcat(payload, roomID);
            return payload;
        } else {
            return NULL;
        }

    } else if (strcmp(input, "init") == 0) {
        char* command = getNthCommand(inputParameter, 2);
        if (strcmp(command, "file") == 0) {
            char* fileName = getNthCommand(inputParameter, 3);
            char* fileData = readFileContents(fileName);
            if (fileData != NULL) {
                char* roomID = getNthCommand(fileData, 1);

                if (checkInputRegisterRoom(roomID) == 1) {
                    if (checkSensorsInputs(fileData) == 1) {
                        char* payload = malloc(strlen("INI_REQ ") + strlen(roomID) + strlen(fileData) + 1);
                        strcpy(payload, "INI_REQ ");
                        strcat(payload, fileData);
                        return payload;
                    } else {
                        return NULL;
                    }
                } else {
                    return NULL;
                }
            } else {
                return NULL;
            }

        } else if (strcmp(command, "info") == 0) {
            char* roomID = getNthCommand(inputParameter, 3);
            char* sensorsInputs = getInputDataFromString(inputParameter, 2);

            if (checkInputRegisterRoom(roomID) == 1) {
                if (checkSensorsInputs(sensorsInputs) == 1) {
                    char* payload = malloc(strlen("INI_REQ ") + strlen(roomID) + strlen(sensorsInputs) + 1);
                    strcpy(payload, "INI_REQ ");
                    strcat(payload, sensorsInputs);
                    return payload;
                } else {
                    return NULL;
                }
            } else {
                return NULL;
            }

        } else {
            return NULL;
        }

    } else if (strcmp(input, "shutdown") == 0) {
        char* roomID = getNthCommand(inputParameter, 2);
        if (checkInputRegisterRoom(roomID) == 1) {
            char* payload = malloc(strlen("DES_REQ ") + strlen(roomID) + 1);
            strcpy(payload, "DES_REQ ");
            strcat(payload, roomID);
            return payload;
        } else {
            return NULL;
        }
    } else if (strcmp(input, "update") == 0) {
        char* command = getNthCommand(inputParameter, 2);
        if (strcmp(command, "file") == 0) {
            char* fileName = getNthCommand(inputParameter, 3);
            char* fileData = readFileContents(fileName);
            if (fileData != NULL) {
                char* roomID = getNthCommand(fileData, 1);

                if (checkInputRegisterRoom(roomID) == 1) {
                    if (checkSensorsInputs(fileData) == 1) {
                        char* payload = malloc(strlen("ALT_REQ ") + strlen(roomID) + strlen(fileData) + 1);
                        strcpy(payload, "ALT_REQ ");
                        strcat(payload, fileData);
                        return payload;
                    } else {
                        return NULL;
                    }
                } else {
                    return NULL;
                }
            } else {
                return NULL;
            }

        } else if (strcmp(command, "info") == 0) {
            char* roomID = getNthCommand(inputParameter, 3);
            char* sensorsInputs = getInputDataFromString(inputParameter, 2);

            if (checkInputRegisterRoom(roomID) == 1) {
                if (checkSensorsInputs(sensorsInputs) == 1) {
                    char* payload = malloc(strlen("ALT_REQ ") + strlen(roomID) + strlen(sensorsInputs) + 1);
                    strcpy(payload, "ALT_REQ ");
                    strcat(payload, sensorsInputs);
                    return payload;
                } else {
                    return NULL;
                }
            } else {
                return NULL;
            }

        } else {
            return NULL;
        }
    } else if (strcmp(input, "load") == 0) {
        char* command = getNthCommand(inputParameter, 2);
        if (strcmp(command, "info") == 0) {
            char* roomID = getNthCommand(inputParameter, 3);
            if (checkInputRegisterRoom(roomID) == 1) {
                char* payload = malloc(strlen("SAL_REQ ") + strlen(roomID) + 1);
                strcpy(payload, "SAL_REQ ");
                strcat(payload, roomID);
                return payload;
            } else {
                return NULL;
            }
        } else if (strcmp(command, "rooms") == 0) {
            char* payload = malloc(strlen("INF_REQ") + 1);
            strcpy(payload, "INF_REQ");
            return payload;
        } else {
            return NULL;
        }

    } else {
        return NULL;
    }
}