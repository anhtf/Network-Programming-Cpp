#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // Chứa read, write, close
#include <arpa/inet.h>   // Chứa inet_pton, htons
#include <sys/socket.h>  // Chứa core socket API

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *hello = "Hello tu Server! Da nhan duoc tin nhan.";

    // 1. Dọn sạch bộ nhớ của struct để tránh 'rác' (Fix lỗi Bind failed)
    memset(&address, 0, sizeof(address));

    // 2. Tạo socket - Kiểm tra < 0 (Hàm socket trả về -1 nếu lỗi)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 3. Cấu hình socket: Chỉ dùng SO_REUSEADDR để tương thích mọi hệ điều hành
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT);

    // 4. Bind: Gắn socket với địa chỉ và Port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 5. Listen: Bắt đầu lắng nghe
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server dang lang nghe tren cong %d...\n", PORT);

    // 6. Accept: Chặn (block) chương trình, chờ Client kết nối
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Da co Client ket noi!\n");

    // 7. Giao tiếp qua socket mới tạo (new_socket)
    read(new_socket, buffer, BUFFER_SIZE);
    printf("Client noi: %s\n", buffer);
    
    send(new_socket, hello, strlen(hello), 0);
    printf("Da gui phan hoi cho Client.\n");

    // 8. Đóng kết nối
    close(new_socket);
    close(server_fd);
    
    return 0;
}