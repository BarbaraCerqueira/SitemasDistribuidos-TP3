#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define COORDINATOR_PORT 8888
#define BUFFER_SIZE 1024
#define MESSAGE_SIZE 10
#define MAX_CLIENTS 200
#define REQUEST_MESSAGE_ID 1
#define GRANT_MESSAGE_ID 2
#define RELEASE_MESSAGE_ID 3

void sendRequest(int processId, int socket) {
    char message[MESSAGE_SIZE + 1];
    int padding; // padding length
    padding = MESSAGE_SIZE - snprintf(NULL, 0, "%d|%d|", REQUEST_MESSAGE_ID, processId);
    snprintf(message, MESSAGE_SIZE + 1, "%d|%d|%0*d", REQUEST_MESSAGE_ID, processId, padding, 0);
    if (send(socket, message, MESSAGE_SIZE, 0) < 0) {
        perror("Error sending REQUEST message");
        return;
    }
}
void sendRelease(int processId, int socket) {
    char message[MESSAGE_SIZE + 1];
    int padding; // padding length
    padding = MESSAGE_SIZE - snprintf(NULL, 0, "%d|%d|", RELEASE_MESSAGE_ID, processId);
    snprintf(message, MESSAGE_SIZE + 1, "%d|%d|%0*d", RELEASE_MESSAGE_ID, processId, padding, 0);
    if (send(socket, message, MESSAGE_SIZE, 0) < 0) {
        perror("Error sending RELEASE message");
        return;
    }
}
int main(int argc, char *argv[]) {
    int pid, id_message;
    int sockets[MAX_CLIENTS];
    struct sockaddr_in address;
    if (argc != 4) {
        printf("Usage: %s <n> <r> <k>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]); // Number of processes
    int r = atoi(argv[2]); // Number of repetitions
    int k = atoi(argv[3]); // Time to wait in critical zone
    for (int i = 1; i <= n; i++) {
        pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            return 1;
        } else if (pid == 0) { // Child process
            // Create socket
            sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
            if (sockets[i] == -1) {
                perror("Socket creation failed");
                return 1;
            }

            memset(&address, 0, sizeof(address));
            address.sin_family = AF_INET;
            address.sin_port = htons(COORDINATOR_PORT);
            address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Coordinator IP (localhost)

            if (connect(sockets[i], (struct sockaddr *)&address, sizeof(address)) == -1) {
                perror("Connection to coordinator failed");
                return 1;

            int processId = getpid();
            
            for (int j = 0; j < r; j++) {
                sendRequest(processId, sockets[i]);
                char buffer[MESSAGE_SIZE + 1];
                memset(buffer, 0, MESSAGE_SIZE + 1);
                recv(sockets[i], buffer, MESSAGE_SIZE, 0);
                sscanf(buffer, "%d|", &id_message);
                if (id_message == GRANT_MESSAGE_ID) {
                    FILE *file = fopen("resultado.txt", "a");
                    if (file != NULL) {
                        struct timeval tv;
                        char timeString[30];
                        char processIdString[10];

                        gettimeofday(&tv, NULL);
                        // Format seconds
                        strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
                        // Format milliseconds
                        char milliseconds[4];
                        snprintf(milliseconds, sizeof(milliseconds), "%.3d", (int)(tv.tv_usec / 1000));
                        // Append milliseconds to the time string
                        strcat(timeString, ".");
                        strcat(timeString, milliseconds);
                        snprintf(processIdString, sizeof(processIdString), "%d", processId);
                        fprintf(file, "Process %s - %s\n", processIdString, timeString);
                        fclose(file);
                        sleep(k); // Wait for k seconds
                    }
                    sendRelease(processId, sockets[i]);
                }
            }
            close(sockets[i]);
            return 0;
        }
    }
    // Parent process
    while (wait(NULL) > 0);
    return 0;
}
