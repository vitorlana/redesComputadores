#ifndef COMMON_H 
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


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

//Generic functions

void error(char* msg);
void errorWithoutKill(char* msg);
Fan getFanData(char* fanData);
char* readFileContents(char* fileName);
void initializeClassrooms(Classroom* classrooms);

//String manipulation functions

int countSpaces(char* text);
int checkCommand(char* text);
char* getNthCommand(char* text, int n);
char* getInputDataFromString(char* data, int n);
char* createClassroomStringData(Classroom* classroom, bool withParenteses);

//Network manipulation functions

void printSocketPortAndAddress(const struct sockaddr* address, char* message);
int setupTCPServerSocket(const char* service);
int setupTCPClientSocket(const char* host, const char* service);
int acceptTCPConnection(int servSock);

//Specific functions for the project

char* executeCommand(char* command, Classroom* classrooms);
char* translateResponseToMessage(char* responseParameter);
char* checkInputAndCreatePayload(char* input);

int checkSensorsInputs(char* inputsParameter);
int checkRoomsSensorsAreInstalled(Classroom* classrooms, int roomId);
int checkRoomCreated(Classroom* classrooms, int roomId);
int checkInputRegisterRoom(char* roomIDParameter);
int checkFanData(int fanPos, Fan fan);

char* createRoomIfNotExists(Classroom* classrooms, int roomId);
char* turnOnSensorsValues(Classroom* classrooms, int roomId, char* sensorValues);
char* updateSensorsValues(Classroom* classrooms, int roomId, char* sensorValues);
char* shutdownSensorsOfClassroom(Classroom* classrooms, int roomId);

char* printClassrooms(Classroom* classrooms);
char* printClassroom(Classroom* classrooms, int classroomID);




#endif