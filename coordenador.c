#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>

#define MESSAGE_SIZE 10
#define MAX_CLIENTS 200
#define MAX_QUEUE_SIZE 2000

#define PORT 8888
#define REQUEST_MESSAGE_ID 1
#define GRANT_MESSAGE_ID 2
#define RELEASE_MESSAGE_ID 3

typedef struct {
    int message_id;
    int process_id;
    int socket_descriptor;
    time_t arrival_time;
} Message;

typedef struct {
    Message* messages;
    int front;
    int rear;
} MessageQueue;

typedef struct {
    int process_id;
    int access_count;
} ProcessStats;

typedef struct {
    MessageQueue queue;
    ProcessStats stats[MAX_CLIENTS];
    int this_pid;
    int terminate;
    pthread_mutex_t queue_mutex;
    pthread_mutex_t stats_mutex;
} CoordinatorData;

typedef struct {
    int socket_descriptor;
    CoordinatorData* coordinator_data;
} ThreadArgs;


int initializeQueue(MessageQueue* queue) {
    queue->messages = (Message*)malloc(MAX_QUEUE_SIZE * sizeof(Message));
    if (queue->messages == NULL) {
        perror("Error allocating memory for Message Queue");
        return 1;
    }
    queue->front = -1;
    queue->rear = -1;
    return 0;
}

int initializeCoordinatorData(CoordinatorData* data) {
    int i;
    // Initializing Process Stats Vector
    for (i = 0; i < MAX_CLIENTS; i++){
        data->stats[i].process_id = -1;
        data->stats[i].access_count = 0;
    }
    initializeQueue(&data->queue);
    data->this_pid = getpid();
    data->terminate = 0;
    pthread_mutex_init(&data->queue_mutex, NULL);
    pthread_mutex_init(&data->stats_mutex, NULL);
    return 0;
}

int isQueueFull(MessageQueue* queue) {
    return queue->rear == MAX_QUEUE_SIZE - 1;
}

int isQueueEmpty(MessageQueue* queue) {
    return queue->front == -1;
}

Message headQueue(MessageQueue* queue){
    return queue->messages[queue->front];
}

void enqueue(MessageQueue* queue, Message message) {
    if (isQueueFull(queue)) {
        printf("Error: Queue is full\n");
        return;
    }
    if (isQueueEmpty(queue)) {
        queue->front = 0;
    }
    queue->rear++;
    queue->messages[queue->rear] = message;
}

Message dequeue(MessageQueue* queue) {
    Message message;
    if (isQueueEmpty(queue)) {
        printf("Error: Queue is empty\n");
        message.message_id = -1;
        return message;
    }
    message = queue->messages[queue->front];
    if (queue->front == queue->rear) {
        free(queue->messages);
        initializeQueue(queue);
    } else {
        queue->front++;
    }
    return message;
}

void printQueue(MessageQueue* queue) {
    int i;
    printf("Requests Queue:\n");
    if (!isQueueEmpty(queue)){
        for (i = queue->front; i <= queue->rear; i++) {
            printf("Process %d waiting access\n", queue->messages[i].process_id);
        }
    }
    printf("\n");
}

void printStats(ProcessStats* stats) {
    int i;
    printf("Access Statistics:\n");
    for (i = 0; i < MAX_CLIENTS; i++) {
        if(stats[i].process_id != -1){
            printf("Process %d: %d access requests granted\n", stats[i].process_id, stats[i].access_count);
        }
    }
    printf("\n");
}

void logMessage(FILE* log_file, Message* message) {
    // Verifies if file exists
    if (log_file == NULL) {
        printf("Error: log file is not open\n");
        return;
    }
    fprintf(log_file, "%d-%d-%ld\n", message->message_id, message->process_id, message->arrival_time);
}

/* 
    Search coordinator data for a process: 
    - If process is not found, adds it
    - If increment = 0, increments process access count
*/
void updateProcessStats(int process_id, CoordinatorData* data, int increment){
    pthread_mutex_lock(&data->stats_mutex);
    // Search for specific Process in data
    for(int i; i < MAX_CLIENTS; i++) {
        // Process already in statistics
        if(data->stats[i].process_id == process_id){
            if (increment) data->stats[i].access_count++;
            break;
        }
        // Process not in statistics -> adds process
        else{
            if(data->stats[i].process_id == -1){
                data->stats[i].process_id = process_id;
                if (increment) data->stats[i].access_count++;
                break;
            }
        }
    }
    pthread_mutex_unlock(&data->stats_mutex);
}

void grantMessage(int process_id, int socket){
    char buffer[MESSAGE_SIZE + 1];
    int padding; // padding length
    padding = MESSAGE_SIZE - snprintf(NULL, 0, "%d|%d|", GRANT_MESSAGE_ID, process_id);
    snprintf(buffer, MESSAGE_SIZE + 1, "%d|%d|%0*d", GRANT_MESSAGE_ID, process_id, padding, 0);
    if (send(socket, buffer, MESSAGE_SIZE, 0) < 0) {
        perror("Error sending GRANT message");
        return;
    }
}

void* clientThread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int socket_descriptor = args->socket_descriptor;
    CoordinatorData* data = args->coordinator_data;
    char message_buffer[MESSAGE_SIZE+1];
    Message received_message, head_queue_message, grant_message;
    FILE* log_file = fopen("log.txt", "a");

    if (log_file == NULL) {
        perror("Error trying to open log file");
        return NULL;
    }

    // Accept and handle new client messages
    while (1) {
        // Clean buffer up before each read
        memset(message_buffer, 0, MESSAGE_SIZE+1);

        // Receive message from client process
        int bytes_received = recv(socket_descriptor, message_buffer, MESSAGE_SIZE, 0);

        printf("Message received through client socket %d: %s \n", socket_descriptor, message_buffer); // DEBUG

        received_message.arrival_time = time(NULL); // Register instant of receipt of message
        received_message.socket_descriptor = socket_descriptor; // Register socket that possibly will be used to send Grant later

        if (bytes_received < 0) {
            perror("Error receiving message from client");
            break;
        } else if (bytes_received == 0) {
            // Connection closed
            break;
        } else { // Processing message

            sscanf(message_buffer, "%d|%d|", &received_message.message_id, &received_message.process_id);

            printf("Stored message: time is %ld, socket is %d, message id is %d, process id is %d. \n", received_message.arrival_time, received_message.socket_descriptor, received_message.message_id, received_message.process_id); // DEBUG

            // Adds process to coordinator data if it doesn't exist
            updateProcessStats(received_message.process_id, data, 1);

            // Request message
            if(received_message.message_id == REQUEST_MESSAGE_ID){
                pthread_mutex_lock(&data->queue_mutex);
                printf("Entered critical zone, process %d\n", received_message.process_id); // DEBUG
                if (isQueueEmpty(&data->queue)) {
                    // Send Grant message to process
                    grantMessage(data->this_pid, socket_descriptor);

                    //Log Grant Message
                    grant_message.arrival_time = time(NULL); // Register message sending time
                    grant_message.message_id = GRANT_MESSAGE_ID;
                    grant_message.process_id = data->this_pid;
                    logMessage(log_file, &grant_message);

                    printf("Queue Empty: Sent GRANT to process %d\n", received_message.process_id); // DEBUG

                    // Updates process access counter
                    updateProcessStats(received_message.process_id, data, 0);
                }
                // Enqueue message
                enqueue(&data->queue, received_message);
                pthread_mutex_unlock(&data->queue_mutex);

                printf("Message of request from process %d is enqueued, out of critical zone\n", received_message.process_id); // DEBUG
            }

            // Release message
            if(received_message.message_id == RELEASE_MESSAGE_ID){
                pthread_mutex_lock(&data->queue_mutex);
                dequeue(&data->queue); // Removes from queue

                printf("Release message from process %d, out of queue.\n", received_message.process_id); // DEBUG
                
                if (!isQueueEmpty(&data->queue)){
                    // Picks next process in queue
                    head_queue_message = headQueue(&data->queue);

                    // Send grant to next process waiting in queue
                    grantMessage(data->this_pid, head_queue_message.socket_descriptor);

                    printf("Next in Queue: Sent GRANT to process %d\n", head_queue_message.process_id); // DEBUG

                    //Log Grant Message
                    grant_message.arrival_time = time(NULL); // Register message sending time
                    grant_message.message_id = GRANT_MESSAGE_ID;
                    grant_message.process_id = data->this_pid;
                    logMessage(log_file, &grant_message);

                    // Updates process access counter
                    updateProcessStats(head_queue_message.process_id, data, 0);
                }
                pthread_mutex_unlock(&data->queue_mutex);
            }

            // Log received message
            logMessage(log_file, &received_message);

            printf("Logged message of id %d, from process %d\n", received_message.message_id, received_message.process_id); // DEBUG
        }
    }
    close(socket_descriptor);
    printf("Connection closed on socket %d by process %d\n", socket_descriptor, received_message.process_id); // DEBUG
    pthread_exit(NULL);  
}

void* coordinatorThread(void* arg) {
    CoordinatorData* data = (CoordinatorData*)arg;
    pthread_t client_threads[MAX_CLIENTS];
    ThreadArgs thread_args[MAX_CLIENTS];
    int client_sockets[MAX_CLIENTS];
    int coordinator_socket, new_socket, client_address_size, i;
    struct sockaddr_in coordinator_addr, client_addr;
    int max_sd;
    fd_set readfds;
    
    // Initializing client sockets vector
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // Create coordinator socket
    coordinator_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (coordinator_socket < 0) {
        perror("Error creating coordinator socket");
        return NULL;
    }
    
    // Bind coordinator socket to port 8888
    coordinator_addr.sin_family = AF_INET;
    coordinator_addr.sin_addr.s_addr = INADDR_ANY;
    coordinator_addr.sin_port = htons(PORT);
    
    if (bind(coordinator_socket, (struct sockaddr*)&coordinator_addr, sizeof(coordinator_addr)) < 0) {
        perror("Error binding coordinator socket");
        return NULL;
    }
    
    // Listen for client connections, maximum of 5 at a time
    if (listen(coordinator_socket, 5) < 0) {
        perror("Error listening for connections");
        return NULL;
    }

    printf("Coordinator listening on port %d...\n", PORT);

    // Accept and handle client connections
    while (!data->terminate) {
        FD_ZERO(&readfds);
        FD_SET(coordinator_socket, &readfds);
        max_sd = coordinator_socket;

        struct timeval timeout;
        timeout.tv_sec = 1; // Time limit set to 1 second
        timeout.tv_usec = 0;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0) {
            perror("Error in select");
            break;
        }

        if (activity == 0) {
            // Timeout with no activity, verifies terminate condition
            continue;
        }

        // Connection through main socket - new connection
        if (FD_ISSET(coordinator_socket, &readfds)) {
            client_address_size = sizeof(client_addr);
            new_socket = accept(coordinator_socket, (struct sockaddr*)&client_addr, (socklen_t*)&client_address_size);
            if (new_socket < 0) {
                perror("Error accepting client connection");
                continue;
            }
            
            printf("Accepted new connection through socket %d\n", new_socket); //DEBUG

            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    // Add new socket to client sockets vector
                    client_sockets[i] = new_socket;

                    // Create client thread and send socket descriptor and coordinator data as argument
                    thread_args[i].socket_descriptor = new_socket;
                    thread_args[i].coordinator_data = data;
                    if (pthread_create(&client_threads[i], NULL, clientThread, &thread_args[i]) != 0) {
                        perror("Error creating client thread");
                        return NULL;
                    }

                    break;
                }
            }
        }     
    }
    close(coordinator_socket);
    pthread_exit(NULL);
}

void* interfaceThread(void* arg) {
    CoordinatorData* data = (CoordinatorData*)arg;
    char command[10];
    
    while (!data->terminate) {
        printf("Enter command (1: Print queue, 2: Print stats, 3: Terminate): ");
        scanf("%s", command);
        if (strcmp(command, "1") == 0) {
            pthread_mutex_lock(&data->queue_mutex);
            printQueue(&data->queue);
            pthread_mutex_unlock(&data->queue_mutex);
        } else if (strcmp(command, "2") == 0) {
            pthread_mutex_lock(&data->stats_mutex);
            printStats(data->stats);
            pthread_mutex_unlock(&data->stats_mutex);
        } else if (strcmp(command, "3") == 0) {
            printf("Exiting...\n");
            data->terminate = 1;
        } else {
            printf("Invalid command\n");
        }
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t coordinator_thread, interface_thread;
    CoordinatorData data;

    // Initialize coordinator data
    initializeCoordinatorData(&data);

    // Create coordinator thread
    if (pthread_create(&coordinator_thread, NULL, coordinatorThread, &data) != 0) {
        perror("Error creating coordinator thread");
        return 1;
    }
    sleep(0.5); // To maintain the correct order of messages

    // Create interface thread
    if (pthread_create(&interface_thread, NULL, interfaceThread, &data) != 0) {
        perror("Error creating interface thread");
        return 1;
    }
    
    // Wait for coordinator thread to finish
    if (pthread_join(coordinator_thread, NULL) != 0) {
        perror("Error joining coordinator thread");
        return 1;
    }

    // Wait for interface thread to finish
    if (pthread_join(interface_thread, NULL) != 0) {
        perror("Error joining interface thread");
        return 1;
    }
    
    pthread_mutex_destroy(&data.queue_mutex);
    pthread_mutex_destroy(&data.stats_mutex); 

    return 0;
}