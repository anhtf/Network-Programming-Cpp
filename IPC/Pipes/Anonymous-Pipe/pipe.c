/*
Bản chất của Pipe:

Nó là một đường ống dẫn nước một chiều (Half-duplex). Dữ liệu chảy vào ở một đầu và chảy ra ở đầu kia.

Nó hoạt động theo nguyên tắc FIFO (First In First Out) - Dữ liệu nào bơm vào trước sẽ được đọc ra trước.

Nhược điểm chí mạng: Pipe vô danh không có tên tuổi trên hệ thống file. 
Do đó, nó CHỈ dùng được giữa các tiến trình có quan hệ huyết thống với nhau (Cha - Con, tạo ra bằng lệnh fork()).

Quy tắc trong Code C:
Khi bạn gọi hàm pipe(fd), hệ điều hành sẽ cấp cho bạn mảng 2 số nguyên:

fd[0]: Đầu dùng để Đọc (Read) - Giống như vòi xả nước ra.

fd[1]: Đầu dùng để Ghi (Write) - Giống như phễu đổ nước vào.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pid_t pid;
    char *father_buffer  = "Message from father!";
    char children_buffer[100];
    
    if (pipe(fd) == -1)
    {
        perror("Failed to create PIPE\n");
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        perror("Error while creating child process\n");
        return 1;
    }

    if (pid > 0)
    {
        close(fd[0]);
        printf("[Father] Sending message to child...\n");
        write(fd[1], father_buffer, strlen(father_buffer) + 1);
        close(fd[1]);
        wait(NULL);
    }
    else
    {
        close(fd[1]);
        read(fd[0], children_buffer, sizeof(children_buffer));
        printf("[Child] Received message: %s\n", children_buffer);
        close(fd[0]);
    }
    return 0;
}