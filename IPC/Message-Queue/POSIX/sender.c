#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>           
#include <sys/stat.h>        
#include <mqueue.h> 

#define QUEUE_NAME  "/thiet_bi_sensor"
#define MAX_SIZE    1024

int main() {
    mqd_t mq;
    char buffer[MAX_SIZE];
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;          
    attr.mq_msgsize = MAX_SIZE;   
    attr.mq_curmsgs = 0;


    mq = mq_open(QUEUE_NAME, O_WRONLY | O_CREAT, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("Lỗi tạo Message Queue (Sender)");
        exit(1);
    }

    strcpy(buffer, "Nhiet do: 30C");
    if (mq_send(mq, buffer, strlen(buffer) + 1, 1) == -1) perror("Lỗi gửi tin 1");
    else printf("Da gui tin nhan thuong.\n");

    strcpy(buffer, "CANH BAO: QUA NHIET!");
    if (mq_send(mq, buffer, strlen(buffer) + 1, 10) == -1) perror("Lỗi gửi tin 2");
    else printf("Da gui tin nhan KHAN CAP!\n");

    mq_close(mq);
    return 0;
}