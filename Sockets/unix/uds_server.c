#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h> // Header bắt buộc cho Unix Domain Socket

#define SOCKET_PATH "/tmp/my_embedded_socket" // Đường dẫn file thay cho IP:Port
#define BUFFER_SIZE 256

int main() {
    int server_fd, client_socket;
    struct sockaddr_un name;
    char buffer[BUFFER_SIZE];

    // 1. Khởi tạo socket với AF_UNIX
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Dọn dẹp: Xóa file socket cũ nếu nó bị sót lại từ lần chạy trước
    // (Đây là lỗi cực kỳ phổ biến khiến bind() thất bại với UDS)
    unlink(SOCKET_PATH); 

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_PATH, sizeof(name.sun_path) - 1);

    // 3. Bind: Gắn socket với file hệ thống
    if (bind(server_fd, (const struct sockaddr *) &name, sizeof(name)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 4. Listen & Accept (Giống hệt TCP)
    listen(server_fd, 5);
    printf("UDS Server dang cho lenh tai %s...\n", SOCKET_PATH);

    if ((client_socket = accept(server_fd, NULL, NULL)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // 5. Đọc dữ liệu
    read(client_socket, buffer, BUFFER_SIZE);
    printf("Nhan duoc lenh tu Client: %s\n", buffer);

    if(strcmp(buffer, "BAT_LED") == 0) {
        char *resp = "OK! Da bat den.";
        write(client_socket, resp, strlen(resp));
    }

    // 6. Dọn dẹp
    close(client_socket);
    close(server_fd);
    unlink(SOCKET_PATH); // Xóa file khi xong việc

    return 0;
}