/*
Signal không dùng để gửi dữ liệu (như chuỗi ký tự hay hình ảnh). Nó giống như việc bạn kéo chuông báo cháy

SIGINT (Signal Interrupt): Gửi khi bạn bấm Ctrl+C trên terminal. Mặc định nó sẽ giết chết chương trình.

SIGKILL (Signal Kill): Gửi khi bạn gõ lệnh kill -9 <PID>. Đây là "án tử hình" không thể trốn tránh, ép chương trình phải dừng ngay lập tức.

SIGSEGV (Segmentation Fault): Kernel tự động gửi cho chương trình của bạn khi bạn code sai con trỏ (truy cập vào vùng RAM không được phép).

*/

/*
Lab1: Bắt sự kiện SIGINT khi Ctrl+C
*/

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "signal.h"

void signal_handle(int _signal)
{
    if (_signal == SIGINT)
    {
        printf("User press Ctrl + C\n");
        printf("Wait for clean up !\n");
        sleep(2);
        printf("Exiting\n");
        exit(0);
    }
}

int main()
{
    signal(SIGINT, signal_handle);

    while(1)
    {
        printf("System is running\n");
        sleep(1);
    }
    return 0;
}

