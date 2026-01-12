#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdint.h>

/* ===== ĐỊNH NGHĨA CÁC HẰNG SỐ ===== */
#define SEM_MUTEX_NAME "/sem-mutex"                    /* Bảo vệ vùng tới hạn */
#define SEM_BUFFER_COUNT_NAME "/sem-buffer-count"       /* Đếm buffer trống */
#define SEM_BUFFER_SIGNAL_NAME "/sem-spool-signal"      /* Báo có dữ liệu mới */
#define SHARED_MEM_NAME "/posix-shared-mem-example"    /* Tên bộ nhớ chia sẻ */

#define LOGFILE "/tmp/example.log"                      /* Tệp log để ghi dữ liệu */
#define MAX_BUFFERS 10                                   /* Số buffer tối đa (phải match với writer) */

/* ===== CẤU TRÚC BỘ NHỚ CHIA SẺ ===== */
struct shared_memory
{
    char buf[MAX_BUFFERS][256];  /* Mảng 10 buffer */
    int buffer_index;            /* Chỉ số buffer cho writer ghi */
    int buffer_print_index;      /* Chỉ số buffer cho reader đọc */
};

/* ===== HÀM XỬ LÝ LỖI ===== */
/**
 * Hiển thị thông báo lỗi và thoát chương trình
 * @param msg: Thông báo lỗi
 */
void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/* ===== HÀM CHÍNH ===== */
/**
 * Chương trình reader - Đọc dữ liệu từ bộ nhớ chia sẻ và ghi vào log
 * Công việc:
 *   1. Kết nối với bộ nhớ chia sẻ
 *   2. Kết nối với 3 semaphore
 *   3. Chờ dữ liệu mới từ writer
 *   4. Đọc dữ liệu từ buffer chia sẻ
 *   5. Ghi dữ liệu vào tệp log
 *   6. Báo cho writer có buffer trống
 */
int main(int argc, char **argv)
{
    struct shared_memory *shared_mem_pointer;  /* Con trỏ tới bộ nhớ chia sẻ */
    sem_t *sem_mutex, *sem_buffer_count, *sem_buffer_signal;  /* Con trỏ semaphore */
    int shm_fd, log_fd;  /* File descriptor */

    char buffer[256];  /* Buffer tạm để lưu dữ liệu đọc được */

    printf("[READER] Kết nối tới các resources...\n");

    /* ===== BƯỚC 1: KẾT NỐI BỘ NHỚ CHIA SẺ ===== */
    /* Dùng flag 0 nghĩa là chỉ kết nối, không tạo mới */
    if ((shm_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0)) == -1)
    {
        error("shm_open failed");
    }

    /* ===== BƯỚC 2: MỞ TỆP LOG ===== */
    if ((log_fd = open(LOGFILE, O_CREAT | O_WRONLY | O_APPEND, 0666)) == -1)
    {
        error("log file open failed");
    }

    /* ===== BƯỚC 3: KẾT NỐI SEMAPHORE ===== */
    /* Kết nối semaphore mutex (dùng flag 0 để không tạo mới) */
    if ((sem_mutex = sem_open(SEM_MUTEX_NAME, 0)) == SEM_FAILED)
    {
        error("sem_open mutex failed");
    }

    /* Ánh xạ bộ nhớ chia sẻ vào địa chỉ ảo */
    if ((shared_mem_pointer = mmap(0, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
    {
        error("mmap failed");
    }

    /* Kết nối semaphore buffer_count */
    if ((sem_buffer_count = sem_open(SEM_BUFFER_COUNT_NAME, 0)) == SEM_FAILED)
    {
        error("sem_open buffer count failed");
    }

    /* Kết nối semaphore spool_signal */
    if ((sem_buffer_signal = sem_open(SEM_BUFFER_SIGNAL_NAME, 0)) == SEM_FAILED)
    {
        error("sem_open buffer signal failed");
    }

    printf("[READER] Kết nối thành công!\n");
    printf("[READER] Chờ dữ liệu từ writer...\n\n");

    /* ===== VÒNG LẶP CHÍNH - ĐỌC VÀ GHI LOG ===== */
    while (1)
    {
        /* ===== BƯỚC A: CHỜ DỮ LIỆU MỚI ===== */
        /* P(sem_buffer_signal) - Chờ cho tới khi writer ghi dữ liệu mới */
        printf("[READER] Chờ dữ liệu mới...\n");
        if (sem_wait(sem_buffer_signal) == -1)
        {
            error("sem_wait buffer signal failed");
        }
        printf("[READER]   -> Nhận được tín hiệu từ writer\n");

        /* ===== BƯỚC B: KHÓA VÙNG TỚI HẠN ===== */
        /* P(mutex_sem) - Chỉ cho phép 1 reader đọc buffer_print_index cùng lúc */
        printf("[READER]   -> Khóa vùng tới hạn...\n");
        if (sem_wait(sem_mutex) == -1)
        {
            error("sem_wait mutex failed");
        }

        /* ===== BƯỚC C: ĐỌC DỮ LIỆU TỪ BUFFER (VÙNG TỚI HẠN) ===== */
        strcpy(buffer, shared_mem_pointer->buf[shared_mem_pointer->buffer_print_index]);
        printf("[READER]   -> Đọc từ buffer[%d]: %s", shared_mem_pointer->buffer_print_index, buffer);
        
        shared_mem_pointer->buffer_print_index++;
        if (shared_mem_pointer->buffer_print_index >= MAX_BUFFERS)
        {
            shared_mem_pointer->buffer_print_index = 0;
        }

        /* ===== BƯỚC D: MỞ KHÓA VÙNG TỚI HẠN ===== */
        /* V(mutex_sem) - Giải phóng khóa */
        printf("[READER]   -> Mở khóa\n");
        if (sem_post(sem_mutex) == -1)
        {
            error("sem_post mutex failed");
        }

        /* ===== BƯỚC E: GHI DỮ LIỆU VÀO LOG ===== */
        printf("[READER]   -> Ghi vào log file\n");
        if (write(log_fd, buffer, strlen(buffer)) == -1)
        {
            error("log file write failed");
        }

        /* ===== BƯỚC F: BÁO CHO WRITER CÓ BUFFER TRỐNG ===== */
        /* V(sem_buffer_count) - Tăng giá trị, báo buffer trống */
        printf("[READER]   -> Báo cho writer có buffer trống\n\n");
        if (sem_post(sem_buffer_count) == -1)
        {
            error("sem_post buffer count failed");
        }
    }

    return 0;
}
