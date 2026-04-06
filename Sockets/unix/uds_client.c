#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/my_embedded_socket"

int main() {
    int sockfd;
    struct sockaddr_un name;
    char buffer[256];
    char *command = "BAT_LED";

    // 1. Tạo socket
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_PATH, sizeof(name.sun_path) - 1);

    // 2. Kết nối tới file socket của Server
    if (connect(sockfd, (const struct sockaddr *) &name, sizeof(name)) < 0) {
        perror("Connect failed (Server da chay chua?)");
        exit(EXIT_FAILURE);
    }

    // 3. Gửi lệnh và nhận phản hồi
    write(sockfd, command, strlen(command));
    printf("Da gui lenh: %s\n", command);

    read(sockfd, buffer, sizeof(buffer));
    printf("Hardware (Server) tra loi: %s\n", buffer);

    close(sockfd);
    return 0;
}