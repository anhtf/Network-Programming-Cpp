#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           
#include <sys/stat.h>        
#include <mqueue.h>

#define QUEUE_NAME  "/thiet_bi_sensor"
#define MAX_SIZE    1024

int main() {
    mqd_t mq;
    char buffer[MAX_SIZE];
    unsigned int priority;

    // Mở hàng đợi để đọc
    mq = mq_open(QUEUE_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("Lỗi mở Message Queue (Receiver)");
        exit(1);
    }

    printf("Receiver dang cho tin nhan (Bam Ctrl+C de thoat)...\n");

    while(1) {
      
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, &priority);
        
        if (bytes_read >= 0) {
            printf("Nhan duoc: '%s' (Do uu tien: %u)\n", buffer, priority);
        } else {
            perror("Lỗi mq_receive");
            break;
        }
    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME); 
    return 0;
}