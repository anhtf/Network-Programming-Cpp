#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <libgen.h> 

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 4096

void* receive_messages(void* socket_desc);
void send_file(int sock, const char* filename);

int main() {
    int sock;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to server.\n");
    printf("You can now start chatting. Type '/send <filename>' to send a file or '/quit' to exit.\n\n");
    
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, (void*)&sock) < 0) {
        perror("could not create thread");
        return 1;
    }

    char input_buffer[BUFFER_SIZE];
    while (1) {
        fgets(input_buffer, BUFFER_SIZE, stdin);
        input_buffer[strcspn(input_buffer, "\n")] = 0; 

        if (strncmp(input_buffer, "/send ", 6) == 0) {
            send_file(sock, input_buffer + 6);
        } else if (strcmp(input_buffer, "/quit") == 0) {
            send(sock, "/quit", strlen("/quit"), 0);
            break;
        }
        else {
            send(sock, input_buffer, strlen(input_buffer), 0);
        }
    }

    close(sock);
    return 0;
}

void receive_file(int sock, const char* control_message) {
    char filename[256];
    long file_size;
    sscanf(control_message, "FILE_REQ:%[^:]:%ld", filename, &file_size);

    printf("Incoming file: %s, Size: %ld bytes. Receiving...\n", filename, file_size);

    char received_filename[512];
    sprintf(received_filename, "received_%s", basename((char*)filename));

    FILE* fp = fopen(received_filename, "wb");
    if (fp == NULL) {
        perror("Failed to open file");
        return;
    }

    char buffer[BUFFER_SIZE];
    long bytes_received = 0;
    int len;

    while (bytes_received < file_size) {
        len = recv(sock, buffer, BUFFER_SIZE, 0);
        if (len <= 0) break;
        fwrite(buffer, 1, len, fp);
        bytes_received += len;
    }

    fclose(fp);
    printf("File '%s' received successfully.\n> ", received_filename);
    fflush(stdout);
}

void* receive_messages(void* socket_desc) {
    int sock = *(int*)socket_desc;
    char server_reply[BUFFER_SIZE];
    int read_size;

    while ((read_size = recv(sock, server_reply, BUFFER_SIZE, 0)) > 0) {
        server_reply[read_size] = '\0';

        if (strncmp(server_reply, "FILE_REQ:", 9) == 0) {
            receive_file(sock, server_reply);
        } else if (strcmp(server_reply, "/quit") == 0) {
            printf("\nServer has disconnected. Exiting.\n");
            close(sock);
            exit(0);
        }
        else {
            printf("Server: %s\n> ", server_reply);
            fflush(stdout); 
        }
        memset(server_reply, 0, sizeof(server_reply));
    }

    if (read_size == 0) {
        printf("Server disconnected.\n");
    } else if (read_size == -1) {
        perror("recv failed");
    }

    return 0;
}

void send_file(int sock, const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error: Could not open file '%s'.\n> ", filename);
        fflush(stdout);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char control_message[512];
    sprintf(control_message, "FILE_REQ:%s:%ld", filename, file_size);
    send(sock, control_message, strlen(control_message), 0);
    
    usleep(100000); 

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    printf("Sending file: %s (%ld bytes)...\n", filename, file_size);
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        if (send(sock, buffer, bytes_read, 0) < 0) {
            perror("File send failed");
            break;
        }
    }

    fclose(fp);
    printf("File sent successfully.\n> ");
    fflush(stdout);
}