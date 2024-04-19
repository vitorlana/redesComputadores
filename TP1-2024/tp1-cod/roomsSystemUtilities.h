#ifndef ROOMS_SYSTEM_UTILITIES_H 
#define ROOMS_SYSTEM_UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define MAX_ROOMS 8

// Definition of the structure to store the state of each fan
typedef struct {
    int id;     // ID of the fan
    int state;  // State of the fan (0 = Faulty, 1 = Off, 2 = On)
} Fan;

// Definition of the structure to store information about a classroom
typedef struct {
    int roomID;         // ID of the room
    int temperature;  // Temperature of the room in Celsius
    int humidity;     // Air humidity in percentage
    Fan fans[4];        // Array of fans, assuming 4 fans per room
} Classroom;

char* convertErrosAndSuccess(char* inputParameter);
int checkCommand(char* text);
char* executeCommand(char* command, Classroom* classrooms);
char* readFileContents(char* fileName);
char* getNthCommand(char* text, int n);
char* getInputDataFromString(char* data, int n);
char* createRoomIfNotExists(Classroom* classrooms, int roomId);
int checkRoomCreated(Classroom* classrooms, int roomId);
char* printClassrooms(Classroom* classrooms);
char* printClassroom(Classroom* classrooms, int classroomID);
char* checkInputAndCreatePayload(char* input);
int checkInputRegisterRoom(char* roomIDParameter);
int checkSensorsInputs(char* inputsParameter);
int checkFanData(int fanPos, Fan fan);
Fan getFanData(char* fanData);
int countSpaces(char* text);
char* turnOnSensorsValues(Classroom* classrooms, int roomId, char* sensorValues);
void initializeClassrooms(Classroom* classrooms);
char* createClassroomStringData(Classroom* classroom, bool withParenteses);
char* shutdownSensorsOfClassroom(Classroom* classrooms, int roomId);
int checkRoomsSensorsAreInstalled(Classroom* classrooms, int roomId);
char* updateSensorsValues(Classroom* classrooms, int roomId, char* sensorValues);

#endif