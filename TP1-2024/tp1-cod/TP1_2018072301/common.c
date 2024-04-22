#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include "common.h"

static const int MAXPENDING = 5;  // Maximum outstanding connection requests


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
 * Translates the response of the server to a human-readable message.
 *
 * @param command The error or success code to be converted.
 * @return The corresponding error or success message.
 */
char* translateResponseToMessage(char* responseParameter) {
    // Check if the responseParameter matches specific strings and return corresponding messages
    if (strcmp(responseParameter, "OK_01") == 0) {
        return "sala instanciada com sucesso";
    } else if (strcmp(responseParameter, "OK_02") == 0) {
        return "sensores inicializados com sucesso";
    } else if (strcmp(responseParameter, "OK_03") == 0) {
        return "sensores desligados com sucesso";
    } else if (strcmp(responseParameter, "OK_04") == 0) {
        return "informações atualizadas com sucesso";
    } else if (strcmp(responseParameter, "ERROR_01") == 0) {
        return "sala inválida";
    } else if (strcmp(responseParameter, "ERROR_02") == 0) {
        return "sala já existe";
    } else if (strcmp(responseParameter, "ERROR_03") == 0) {
        return "sala inexistente";
    } else if (strcmp(responseParameter, "ERROR_04") == 0) {
        return "sensores inválidos";
    } else if (strcmp(responseParameter, "ERROR_05") == 0) {
        return "sensores já instalados";
    } else if (strcmp(responseParameter, "ERROR_06") == 0) {
        return "sensores não instalados";
    } else if (strcmp(getNthCommand(responseParameter, 1), "INF_RES") == 0) {
        // If the responseParameter has the command "INF_RES", construct a message with the rooms
        char message[499] = "salas: ";
        return strcat(message, getInputDataFromString(responseParameter, 2));
    } else if (strcmp(getNthCommand(responseParameter, 1), "SAL_RES") == 0) {
        // If the responseParameter has the command "SAL_RES", construct a message with the room
        char message[499] = "sala ";
        return strcat(message, getInputDataFromString(responseParameter, 1));
    } else {
        // If none of the above conditions are met, return the responseParameter as is
        return responseParameter;
    }
}

/**
 * Checks if the given command is valid based on the first word
 * and the list os valid strings.
 *
 * @param text The command to be checked.
 * @return 1 if the command is valid, -1 otherwise.
 */
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
        for (unsigned long i = 0; i < sizeof(validStrings) / sizeof(validStrings[0]); i++) {
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

/**
 * Executes the command based on the input string.
 *
 * @param command The input command string.
 * @param classrooms An array of Classroom structures.
 * @return A string containing the result of the command execution.
 */
char* executeCommand(char* command, Classroom* classrooms) {
    // Extract the base command from the input command
    char* commandBase = getNthCommand(command, 1);

    // Check the base command and perform the corresponding action
    if (strcmp(commandBase, "CAD_REQ") == 0) {
        // If the base command is "CAD_REQ", extract the room ID from the command
        char* roomID = getNthCommand(command, 2);
        // Create a room if it doesn't already exist and return the result
        return (createRoomIfNotExists(classrooms, atoi(roomID)));

    } else if (strcmp(commandBase, "INI_REQ") == 0) {
        // If the base command is "INI_REQ", extract the room ID and sensor inputs from the command
        char* roomID = getNthCommand(command, 2);
        char* sensorsInputs = getInputDataFromString(command, 2);
        // Turn on the sensors with the provided values for the specified room and return the result
        return turnOnSensorsValues(classrooms, atoi(roomID), sensorsInputs);

    } else if (strcmp(commandBase, "DES_REQ") == 0) {
        // If the base command is "DES_REQ", extract the room ID from the command
        char* roomID = getNthCommand(command, 2);
        // Shutdown the sensors of the specified room and return the result
        return shutdownSensorsOfClassroom(classrooms, atoi(roomID));

    } else if (strcmp(commandBase, "ALT_REQ") == 0) {
        // If the base command is "ALT_REQ", extract the room ID and sensor inputs from the command
        char* roomID = getNthCommand(command, 2);
        char* sensorsInputs = getInputDataFromString(command, 2);
        // Update the sensor values of the specified room with the provided inputs and return the result
        return updateSensorsValues(classrooms, atoi(roomID), sensorsInputs);

    } else if (strcmp(commandBase, "SAL_REQ") == 0) {
        // If the base command is "SAL_REQ", extract the room ID from the command
        char* roomID = getNthCommand(command, 2);
        // Print the details of the specified room and return the result
        return printClassroom(classrooms, atoi(roomID));

    } else if (strcmp(commandBase, "INF_REQ") == 0) {
        // If the base command is "INF_REQ", print the details of all the rooms and return the result
        return printClassrooms(classrooms);

    } else {
        // If the base command is not recognized, return NULL
        return NULL;
    }
}

/**
 * Function to read and return the contents of a file.
 *
 * @param fileName The name of the file to be read with the path if necessary.
 * @return A string of the contents of the file, or NULL if the file cannot be opened.
 */
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

/**
 * Retrieves the nth command from a given string using space as the delimiter.
 *
 * @param text The input string.
 * @param n The position of the command to retrieve.
 * @return The nth command from the input string, or NULL if the nth command does not exist.
 */
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

/**
 * Function to get the input data from the string of command
 *
 * @param data The string of command
 * @param n The position of the space to find in the data string
 * @return A pointer to the substring starting from the nth space, or NULL if the nth space does not exist
 */
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



/**
 * Creates a room if it does not already exist in the given array of classrooms.
 *
 * @param classrooms The array of classrooms.
 * @param roomId The ID of the room to create.
 * @return Returns "ERROR_02" if the room already exists, or "OK_01" if the room was created successfully.
 */
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

/**
 * Updates the sensor values for a specific room.
 *
 * @param classrooms The array of classrooms.
 * @param roomId The ID of the room to update.
 * @param sensorValues The string containing the sensor values.
 * @return A string indicating the result of the update:
 *         - "OK_04" if the update was successful.
 *         - "ERROR_03" if the room does not exist or the sensors are not installed.
 *         - "ERROR_06" if the room is already occupied.
 *         - NULL if an unknown error occurred.
 */
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

/**
 * Updates the sensor values for a specific room.
 *
 * @param classrooms The array of classrooms.
 * @param roomId The ID of the room.
 * @param sensorValues The string containing the sensor values.
 * @return A string indicating the status of the operation. Possible values are:
 *         - "OK_02" if the room exists and the sensors are installed.
 *         - "ERROR_03" if the room does not exist.
 *         - "ERROR_05" if the sensors are not installed in the room.
 *         - NULL if an unknown error occurred.
 */
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

/**
 * Shuts down the sensors of a classroom.
 *
 * This function checks if the specified classroom exists and if its sensors are installed.
 * If the room exists and the sensors are installed, the function updates the sensor values
 * to -1 and returns "OK_03". If the room exists but the sensors are not installed, it returns
 * "ERROR_03". If the room does not exist, it returns "ERROR_06". If an error occurs during
 * the execution of the function, it returns NULL.
 *
 * @param classrooms The array of classrooms.
 * @param roomId The ID of the room to shut down the sensors.
 * @return A string indicating the result of the operation.
 */
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

/**
 * Checks if any room was created.
 *
 * This function iterates through the array of classrooms and checks if any room was created.
 *
 * @param classrooms The array of classrooms.
 * @return Returns 1 if any room was created, -1 otherwise.
 */
int checkAnyRoomCreated(Classroom* classrooms) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (classrooms[i].roomID != -999) {
            return 1;  // Room exists
        }
    }
    return -1;  // No room exists
}

/**
 * Function to concatenate all data for a Classroom.
 *
 * This function takes a Classroom object and concatenates its data into a string.
 * The resulting string includes the room ID, temperature, humidity, and fan states.
 * The format of the string depends on the value of the 'withParenteses' parameter
 * and if the room was registered.
 *
 * @param classroom The Classroom object to extract data from.
 * @param withParenteses A boolean value indicating whether to include parentheses in the string.
 * @return A dynamically allocated string containing the concatenated data.
 */
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

/**
 * Function to print the data of a Classroom struct.
 *
 * This function takes an array of Classroom structs and a classroom ID as input.
 * It checks if the room exists and if its sensors are installed.
 * If the room exists and the sensors are installed, it creates a payload string
 * containing the room data and returns it.
 * If the room does not exist, it returns the string "ERROR_03".
 * If the sensors are not installed, it returns the string "ERROR_06".
 * If any other error occurs, it returns NULL.
 *
 * @param classrooms An array of Classroom structs.
 * @param classroomID The ID of the classroom to print.
 * @return A string containing the room data, or an error message, or NULL.
 */
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
/**
 * Function to print the data of each Classroom struct in a array
 * of Classroom structs. The function check if exists any room created
 * in the array of classrooms, if not return "ERROR_03".
 *
 * @param classrooms An array of Classroom structs.
 * @return A string containing the data of each Classroom struct.
 *         If no rooms exist, returns "ERROR_03".
 */
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





/**
 * Retrieves the fan data from the given string.
 *
 * @param fanData The string containing the fan data.
 * @return The Fan structure with the extracted fan data.
 */
Fan getFanData(char* fanData) {
    Fan fan;
    fan.id = fanData[0] - '0';
    fan.state = fanData[1] - '0';
    return fan;
}



/**
 * Counts the number of spaces in a given string.
 *
 * @param text The string to count spaces in.
 * @return The number of spaces found in the string.
 */
int countSpaces(char* text) {
    int count = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == ' ') {
            count++;
        }
    }
    return count;
}

/**
 * Checks the sensor inputs and validates their values.
 *
 * @param inputsParameter The string containing the sensor inputs.
 * @return Returns 1 if the sensor inputs are valid, -1 otherwise.
 */
int checkSensorsInputs(char* inputsParameter) {
    if (countSpaces(inputsParameter) != 6) {
        fprintf(stderr, "%s\n", translateResponseToMessage("ERROR_04"));
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
        fprintf(stderr, "%s\n", translateResponseToMessage("ERROR_04"));
        return -1;
    }
}

/**
 * Checks if the sensors are installed in a specific room.
 *
 * @param classrooms An array of Classroom objects representing the classrooms.
 * @param roomId The ID of the room to check.
 * @return Returns 1 if the sensors are installed (values are -1), -2 if the sensors are used (values are not -1 and initialized), 
 * -1 if the room does not exist, or 0 for other cases.
 */
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

/**
 * Checks if a room with the given room ID already exists in the array of classrooms.
 *
 * @param classrooms The array of classrooms.
 * @param roomId The ID of the room to check.
 * @return 1 if the room already exists, -1 if the room does not exist.
 */
int checkRoomCreated(Classroom* classrooms, int roomId) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (classrooms[i].roomID == roomId) {
            return 1;  // Room already exists, no need to create
        }
    }
    return -1;  // Room does not exist
}

/**
 * Checks if the input room ID is valid.
 *
 * @param roomIDParameter The room ID as a string.
 * @return 1 if the room ID is valid, -1 otherwise.
 */
int checkInputRegisterRoom(char* roomIDParameter) {
    int roomID = atoi(roomIDParameter);
    if (roomID >= 0 && roomID < MAX_ROOMS) {
        return 1;
    } else {
        fprintf(stderr, "%s\n", translateResponseToMessage("ERROR_01"));
        return -1;
    }
}

/**
 * Checks if the fan data is valid.
 *
 * @param fanPos The position of the fan.
 * @param fan The fan object containing the fan's ID and state.
 * @return 1 if the fan data is valid, -1 otherwise.
 */
int checkFanData(int fanPos, Fan fan) {
    if (fan.id == fanPos && fan.state >= 0 && fan.state <= 2) {
        return 1;
    } else {
        return -1;
    }
}

/**
 * Checks the input parameter and creates a payload to send to server
 * based on the input of the user.
 *
 * @param inputParameter The input parameter to be checked.
 * @return A string representing the payload, or NULL if the input is invalid.
 */
char* checkInputAndCreatePayload(char* inputParameter) {
    // Remove the newline character from inputParameter
    inputParameter[strcspn(inputParameter, "\n")] = '\0';
    // Extract the first command from inputParameter
    char* input = getNthCommand(inputParameter, 1);

    if (strcmp(input, "register") == 0) {
        // If the command is "register", extract the room ID
        char* roomID = getNthCommand(inputParameter, 2);
        // Check if the room ID is valid
        if (checkInputRegisterRoom(roomID) == 1) {
            // If valid, create a payload string with the format "CAD_REQ <roomID>"
            char* payload = malloc(strlen("CAD_REQ ") + strlen(roomID) + 1);
            strcpy(payload, "CAD_REQ ");
            strcat(payload, roomID);
            return payload;
        } else {
            return NULL;
        }

    } else if (strcmp(input, "init") == 0) {
        // If the command is "init", check the sub-command
        char* command = getNthCommand(inputParameter, 2);
        if (strcmp(command, "file") == 0) {
            // If the sub-command is "file", extract the file name
            char* fileName = getNthCommand(inputParameter, 3);
            // Read the contents of the file
            char* fileData = readFileContents(fileName);
            if (fileData != NULL) {
                // Extract the room ID from the file data
                char* roomID = getNthCommand(fileData, 1);

                if (checkInputRegisterRoom(roomID) == 1) {
                    // Check if the sensors inputs in the file data are valid
                    if (checkSensorsInputs(fileData) == 1) {
                        // If valid, create a payload string with the format "INI_REQ <roomID> <fileData>"
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
            // If the sub-command is "info", extract the room ID and sensors inputs
            char* roomID = getNthCommand(inputParameter, 3);
            char* sensorsInputs = getInputDataFromString(inputParameter, 2);

            if (checkInputRegisterRoom(roomID) == 1) {
                // Check if the sensors inputs are valid
                if (checkSensorsInputs(sensorsInputs) == 1) {
                    // If valid, create a payload string with the format "INI_REQ <roomID> <sensorsInputs>"
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
        // If the command is "shutdown", extract the room ID
        char* roomID = getNthCommand(inputParameter, 2);
        if (checkInputRegisterRoom(roomID) == 1) {
            // If valid, create a payload string with the format "DES_REQ <roomID>"
            char* payload = malloc(strlen("DES_REQ ") + strlen(roomID) + 1);
            strcpy(payload, "DES_REQ ");
            strcat(payload, roomID);
            return payload;
        } else {
            return NULL;
        }
    } else if (strcmp(input, "update") == 0) {
        // If the command is "update", check the sub-command
        char* command = getNthCommand(inputParameter, 2);
        if (strcmp(command, "file") == 0) {
            // If the sub-command is "file", extract the file name
            char* fileName = getNthCommand(inputParameter, 3);
            // Read the contents of the file
            char* fileData = readFileContents(fileName);
            if (fileData != NULL) {
                // Extract the room ID from the file data
                char* roomID = getNthCommand(fileData, 1);

                if (checkInputRegisterRoom(roomID) == 1) {
                    // Check if the sensors inputs in the file data are valid
                    if (checkSensorsInputs(fileData) == 1) {
                        // If valid, create a payload string with the format "ALT_REQ <roomID> <fileData>"
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
            // If the sub-command is "info", extract the room ID and sensors inputs
            char* roomID = getNthCommand(inputParameter, 3);
            char* sensorsInputs = getInputDataFromString(inputParameter, 2);

            if (checkInputRegisterRoom(roomID) == 1) {
                // Check if the sensors inputs are valid
                if (checkSensorsInputs(sensorsInputs) == 1) {
                    // If valid, create a payload string with the format "ALT_REQ <roomID> <sensorsInputs>"
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
        // If the command is "load", check the sub-command
        char* command = getNthCommand(inputParameter, 2);
        if (strcmp(command, "info") == 0) {
            // If the sub-command is "info", extract the room ID
            char* roomID = getNthCommand(inputParameter, 3);
            // Create a payload string with the format "SAL_REQ <roomID>"
            char* payload = malloc(strlen("SAL_REQ ") + strlen(roomID) + 1);
            strcpy(payload, "SAL_REQ ");
            strcat(payload, roomID);
            return payload;
        } else if (strcmp(command, "rooms") == 0) {
            // If the sub-command is "rooms", create a payload string with the format "INF_REQ"
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

/**
 * Prints an error message and exits the program.
 *
 * This function prints the specified error message using perror() and then
 * terminates the program by calling exit() with the EXIT_FAILURE constant.
 *
 * @param msg The error message to be printed.
 */
void error(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/**
 * Prints an error message without terminating the program.
 *
 * @param msg The error message to be printed.
 */
void errorWithoutKill(char* msg) {
    perror(msg);
}

/**
 * Prints the socket port and address.
 *
 * This function takes a pointer to a sockaddr structure and a message string as input.
 * It extracts the address and port from the sockaddr structure and prints them to stderr.
 * The address can be either IPv4 or IPv6.
 *
 * @param address A pointer to a sockaddr structure containing the address and port.
 * @param message The message string to be printed along with the address and port.
 */
void printSocketPortAndAddress(const struct sockaddr* address, char* message) {
    // Test for address and stream
    if (address == NULL)
        return;

    void* numericAddress;  // Pointer to binary address
    // Buffer to contain result (IPv6 sufficient to hold IPv4)
    char addrBuffer[INET6_ADDRSTRLEN];
    in_port_t port;  // Port to print
    // Set pointer to address based on address family
    switch (address->sa_family) {
        case AF_INET:
            numericAddress = &((struct sockaddr_in*)address)->sin_addr;
            port = ntohs(((struct sockaddr_in*)address)->sin_port);
            break;
        case AF_INET6:
            numericAddress = &((struct sockaddr_in6*)address)->sin6_addr;
            port = ntohs(((struct sockaddr_in6*)address)->sin6_port);
            break;
        default:
            error("[unknown type]");  // Unhandled type
            return;
    }
    // Convert binary to printable address
    if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL)
        error("[invalid address]");  // Unable to convert
    else {
        fprintf(stderr, "%s address: %s port: %hu\n", message, addrBuffer, port);
    }
}

/**
 * Sets up a TCP server socket.
 *
 * This function creates a TCP server socket and binds it to the local address
 * specified by the service parameter. It then sets the socket to listen for
 * incoming connections.
 *
 * @param service The service or port number to bind the socket to.
 * @return The file descriptor of the server socket if successful, -1 otherwise.
 */
int setupTCPServerSocket(const char* service) {
    // Construct the server address structure
    struct addrinfo addrCriteria;                    // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria));  // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;              // Any address family
    addrCriteria.ai_flags = AI_PASSIVE;              // Accept on any address/port
    addrCriteria.ai_socktype = SOCK_STREAM;          // Only stream sockets
    addrCriteria.ai_protocol = IPPROTO_TCP;          // Only TCP protocol

    struct addrinfo* servAddr;  // List of server addresses
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if (rtnVal != 0)
        error("getaddrinfo() failed");

    int servSock = -1;
    for (struct addrinfo* addr = servAddr; addr != NULL; addr = addr->ai_next) {
        // Create a TCP socket
        servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (servSock < 0)
            continue;  // Socket creation failed; try next address

        // Bind to the local address and set socket to listen
        if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) && (listen(servSock, MAXPENDING) == 0)) {
            // Print local address of socket
            struct sockaddr_storage localAddr;
            socklen_t addrSize = sizeof(localAddr);
            if (getsockname(servSock, (struct sockaddr*)&localAddr, &addrSize) < 0)
                error("getsockname() failed");
            // printSocketPortAndAddress((struct sockaddr*)&localAddr, "Binding to");
            break;  // Bind and listen successful
        }

        close(servSock);  // Close and try again
        servSock = -1;
    }

    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);

    return servSock;
}

/**
 * Accepts a TCP connection on the given server socket.
 *
 * @param servSock The server socket to accept the connection on.
 * @return The client socket connected to the client.
 */
int acceptTCPConnection(int servSock) {
    struct sockaddr_storage clntAddr;  // Client address
    // Set length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntAddrLen);
    if (clntSock < 0)
        error("accept() failed");

    // clntSock is connected to a client!

    // printSocketPortAndAddress((struct sockaddr*)&clntAddr, "Handling client: ");

    return clntSock;
}

/**
 * Establishes a TCP connection to a server specified by the host and service.
 *
 * @param host The host name or IP address of the server.
 * @param service The service name or port number of the server.
 * @return The socket file descriptor if the connection is successful, -1 otherwise.
 */
int setupTCPClientSocket(const char* host, const char* service) {
    // Inform the system what type(s) of address information we want
    struct addrinfo addrCriteria;                    // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria));  // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;              // v4 or v6 is OK
    addrCriteria.ai_socktype = SOCK_STREAM;          // Only stream sockets
    addrCriteria.ai_protocol = IPPROTO_TCP;          // Only TCP protocol

    // Get the address(es)
    struct addrinfo* servAddr;  // Stores the returned server addresses list
    int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
    if (rtnVal != 0) {
        error("getaddrinfo() failed");
    }

    int sock = -1;
    for (struct addrinfo* addr = servAddr; addr != NULL; addr = addr->ai_next) {
        // Create a reliable, stream socket using TCP
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock < 0)
            continue;  // Socket creation failed; try next address

        // Establish the connection to the server
        if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
            break;  // Socket connection successful; stop and return the socket

        close(sock);  // Socket connection failed; try next address
        sock = -1;
    }

    freeaddrinfo(servAddr);  // Free the memory allocated in getaddrinfo()
    return sock;
}
