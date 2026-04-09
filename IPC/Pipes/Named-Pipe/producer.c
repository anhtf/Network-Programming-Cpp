/*
Producer demo for named pipe
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
        printf("Enter a message to send: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Loại bỏ newline

        // Mở FIFO để ghi
        int fd = open(myfifo, O_WRONLY);
        write(fd, buffer, strlen(buffer) + 1);
        close(fd);
    }

    // Xóa FIFO khi không còn sử dụng
    unlink(myfifo);
    
    return 0;
}