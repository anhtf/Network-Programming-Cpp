#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <libgen.h> 

#define PORT 8080
#define BUFFER_SIZE 4096

void* receive_messages(void* socket_desc);
void send_file(int sock, const char* filename);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    printf("Waiting for a client to connect...\n");

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");
    printf("You can now start chatting. Type '/send <filename>' to send a file or '/quit' to exit.\n\n");

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, (void*)&new_socket) < 0) {
        perror("could not create thread");
        return 1;
    }

    char input_buffer[BUFFER_SIZE];
    while (1) {
        fgets(input_buffer, BUFFER_SIZE, stdin);
        input_buffer[strcspn(input_buffer, "\n")] = 0; 

        if (strncmp(input_buffer, "/send ", 6) == 0) {
            send_file(new_socket, input_buffer + 6);
        } else if (strcmp(input_buffer, "/quit") == 0) {
             send(new_socket, "/quit", strlen("/quit"), 0);
             break;
        } else {
            send(new_socket, input_buffer, strlen(input_buffer), 0);
        }
    }

    close(new_socket);
    shutdown(server_fd, SHUT_RDWR);
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
            printf("\nClient has disconnected. Exiting.\n");
            close(sock);
            exit(0);
        }
        else {
            printf("Client: %s\n> ", server_reply);
            fflush(stdout); 
        }
        memset(server_reply, 0, sizeof(server_reply));
    }

    if (read_size == 0) {
        printf("Client disconnected.\n");
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