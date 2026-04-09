/*
Giải pháp: Named Pipe (FIFO).
Hệ điều hành cho phép bạn tạo ra một file ảo ngay trên ổ cứng (ví dụ: /tmp/my_fifo).

Chương trình A chỉ cần mở file đó ra và chạy hàm write().

Chương trình B mở file đó ra và chạy hàm read().

Mặc dù trông như đang ghi vào ổ cứng, nhưng thực chất dữ liệu truyền thẳng qua RAM trong Kernel với tốc độ cực nhanh! Lệnh C để tạo là mkfifo("/tmp/my_fifo", 0666);.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    char *myfifo = "/tmp/my_fifo";
    char buffer[100];

    // Tạo FIFO (Named Pipe)
    mkfifo(myfifo, 0666);

    while (1)
    {
        printf("Waiting for data...\n");
        // Mở FIFO để đọc
        int fd = open(myfifo, O_RDONLY);
        read(fd, buffer, sizeof(buffer));
        printf("Received: %s\n", buffer);
        close(fd);
    }

    // Xóa FIFO khi không còn sử dụng
    unlink(myfifo);
    
    return 0;
}