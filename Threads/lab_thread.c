#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/*
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);

thread: "Thẻ nhân viên". Đây là một biến kiểu pthread_t để lưu ID của luồng. Chúng ta dùng ID này để quản lý luồng đó sau này.

attr: "Yêu cầu đặc biệt". Thường chúng ta để NULL để dùng cấu hình mặc định của hệ thống.

start_routine: "Mô tả công việc". Đây là tên của một hàm mà luồng này sẽ nhảy vào thực hiện ngay khi được tạo ra.

arg: "Công cụ/Dữ liệu". Đây là tham số duy nhất bạn có thể truyền vào hàm xử lý của luồng. Nếu muốn truyền nhiều thứ, bạn phải đóng gói chúng vào một struct.

Hàm pthread_join
Trong lập trình nhúng, nếu luồng chính (Main Thread) kết thúc, toàn bộ "nhà máy" sẽ đóng cửa và tất cả các luồng con sẽ bị tiêu diệt ngay lập tức, bất kể chúng đã làm xong việc hay chưa.

Hàm pthread_join dùng để ra lệnh cho luồng chính: "Hãy đứng đợi ở đây cho đến khi công nhân có ID này làm xong việc thì mới được đi tiếp".
*/ 

void* cong_viec_1(void* arg) {
    printf("Cong nhan 1: Bat dau lam viec...\n");
    sleep(2);
    printf("Cong nhan 1: Da hoan thanh!\n");
    return NULL;
}

void* cong_viec_2(void* arg) {
    printf("Cong nhan 2: Dang quet don...\n");
    sleep(1);
    printf("Cong nhan 2: Xong roi!\n");
    return NULL;
}

int main() {
    pthread_t id1, id2; 

    printf("Main: Bat dau thue cong nhan.\n");

    pthread_create(&id1, NULL, cong_viec_1, NULL);
    pthread_create(&id2, NULL, cong_viec_2, NULL);

    pthread_join(id1, NULL);
    pthread_join(id2, NULL);

    printf("Main: Tat ca cong nhan da xong viec. Dong cua nha may!\n");

    return 0;
}