#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *hello = "Xin chào, tôi là Client!";
    char buffer[BUFFER_SIZE] = {0};

    // 1. Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Lỗi khởi tạo Socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Chuyển đổi địa chỉ IP từ dạng Text (127.0.0.1) sang dạng nhị phân
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\n Địa chỉ không hợp lệ / Không được hỗ trợ \n");
        return -1;
    }

    // 2. Connect: Kết nối tới Server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Kết nối thất bại \n");
        return -1;
    }

    // 3. Write/Read: Gửi tin và chờ phản hồi
    send(sock, hello, strlen(hello), 0);
    printf("Đã gửi lời chào tới Server.\n");
    
    read(sock, buffer, BUFFER_SIZE);
    printf("Server phản hồi: %s\n", buffer);

    // 4. Close
    close(sock);
    
    return 0;
}