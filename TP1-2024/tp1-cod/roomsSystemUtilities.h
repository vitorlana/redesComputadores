#ifndef ROOMS_SYSTEM_UTILITIES_H 
#define ROOMS_SYSTEM_UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definition of the structure to store the state of each fan
typedef struct {
    int id;     // ID of the fan
    int state;  // State of the fan (0 = Faulty, 1 = Off, 2 = On)
} Fan;

// Definition of the structure to store information about a classroom
typedef struct {
    int roomID;         // ID of the room
    float temperature;  // Temperature of the room in Celsius
    float humidity;     // Air humidity in percentage
    Fan fans[4];        // Array of fans, assuming 4 fans per room
} Classroom;

int checkCommand(char* text);
void executeCommand(char* command, Classroom* classrooms);
char* readFileContents(char* fileName);
char* getNthCommand(char* text, int n);
char* getInputDataFromString(char* data, int n);
void createRoomIfNotExists(Classroom* classrooms, int roomId);
void printClassrooms(Classroom* classrooms);

#endif