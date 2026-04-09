/*
System V (đọc là System Five) là một trong những phiên bản Unix thương mại đầu tiên từ những năm 1980.

Triết lý định danh: Nó KHÔNG dùng tên file để nhận diện hộp thư. Thay vào đó, nó dùng một Key (Chìa khóa - là một số nguyên) sinh ra từ hàm ftok().

Điểm yếu chí mạng: Vì nó không được coi là một "File" trong hệ thống Linux, nó không có File Descriptor (FD). 
Vì không có FD, bạn KHÔNG THỂ dùng hàm select() hay poll() để chờ tin nhắn từ System V MQ cùng lúc với việc chờ dữ liệu từ Socket được. 

Cách quản lý: Nếu chương trình crash, hộp thư System V vẫn kẹt trong RAM. Bạn phải mở terminal gõ lệnh ipcs -q để xem danh sách và ipcrm -q <ID> để xóa thủ công.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>

struct msg_buffer {
    long mtype;       
    char mtext[100];  
};

int main() {
    key_t key;
    int msgid;
    struct msg_buffer message;

    key = ftok("progfile", 65);

    msgid = msgget(key, 0666 | IPC_CREAT);

    message.mtype = 1; 
    strcpy(message.mtext, "Giao thuc System V day!");

    msgsnd(msgid, &message, sizeof(message.mtext), 0);
    printf("Da gui tin nhan: %s\n", message.mtext);

    // Xóa Queue 
    // msgctl(msgid, IPC_RMID, NULL); 

    return 0;
}