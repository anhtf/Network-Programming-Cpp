#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>

/* ===== ĐỊNH NGHĨA CÁC HẰNG SỐ ===== */
#define SEM_MUTEX_NAME "/sem-mutex"                    /* Semaphore bảo vệ vùng tới hạn */
#define SEM_BUFFER_COUNT_NAME "/sem-buffer-count"       /* Semaphore đếm buffer trống */
#define SEM_SPOOL_SIGNAL_NAME "/sem-spool-signal"       /* Semaphore báo có dữ liệu mới */
#define SHARED_MEM_NAME "/posix-shared-mem-example"    /* Tên bộ nhớ chia sẻ */

#define LOGFILE "/tmp/example.log"                      /* Tệp log để ghi dữ liệu */
#define MAX_BUFFERS 10                                   /* Số buffer tối đa */

/* ===== CẤU TRÚC BỘ NHỚ CHIA SẺ ===== */
struct shared_memory
{
    char buf[MAX_BUFFERS][256];  /* Mảng 10 buffer, mỗi buffer 256 byte */
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

int main(int argc, char **argv)
{
    struct shared_memory *shared_mem_ptr;  /* Con trỏ tới bộ nhớ chia sẻ */
    sem_t *mutex_sem, *buffer_count_sem, *spool_signal_sem;  /* Con trỏ semaphore */
    int fd_shm;  /* File descriptor của bộ nhớ chia sẻ */

    /* ===== BƯỚC 1: XÓA CÁC RESOURCES CŨ ===== */
    printf("[LOGGER] Xóa các resources cũ...\n");
    shm_unlink(SHARED_MEM_NAME);
    sem_unlink(SEM_MUTEX_NAME);
    sem_unlink(SEM_BUFFER_COUNT_NAME);
    sem_unlink(SEM_SPOOL_SIGNAL_NAME);

    /* ===== BƯỚC 2: TẠO BỘ NHỚ CHIA SẺ ===== */
    printf("[LOGGER] Tạo bộ nhớ chia sẻ...\n");
    if ((fd_shm = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666)) == -1)
    {
        error("shm_open failed");
    }

    /* Cấp phát kích thước cho bộ nhớ chia sẻ */
    if (ftruncate(fd_shm, sizeof(struct shared_memory)) == -1)
    {
        error("ftruncate failed");
    }

    /* ===== BƯỚC 3: ÁNH XẠ BỘ NHỚ CHIA SẺ VÀO ĐỊA CHỈ ẢO ===== */
    if ((shared_mem_ptr = mmap(NULL, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED,
                               fd_shm, 0)) == MAP_FAILED)
    {
        error("mmap failed");
    }

    /* Khởi tạo các chỉ số buffer */
    shared_mem_ptr->buffer_index = 0;
    shared_mem_ptr->buffer_print_index = 0;
    printf("[LOGGER] Bộ nhớ chia sẻ được khởi tạo\n");

    /* ===== BƯỚC 4: TẠO SEMAPHORE ===== */
    printf("[LOGGER] Tạo các semaphore...\n");

    /* Tạo mutex semaphore (giá trị ban đầu = 1)
       Dùng để bảo vệ vùng tới hạn khi cập nhật buffer_index */
    if ((mutex_sem = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1)) == SEM_FAILED)
    {
        error("sem_open mutex failed");
    }

    /* Tạo buffer_count semaphore (giá trị ban đầu = MAX_BUFFERS)
       Dùng để đếm số buffer trống. Writer sẽ chờ nếu buffer đầy */
    if ((buffer_count_sem = sem_open(SEM_BUFFER_COUNT_NAME, O_CREAT, 0666, MAX_BUFFERS)) == SEM_FAILED)
    {
        error("sem_open buffer count failed");
    }

    /* Tạo spool_signal semaphore (giá trị ban đầu = 0)
       Dùng để báo có dữ liệu mới. Reader sẽ chờ trên semaphore này */
    if ((spool_signal_sem = sem_open(SEM_SPOOL_SIGNAL_NAME, O_CREAT, 0666, 0)) == SEM_FAILED)
    {
        error("sem_open spool signal failed");
    }

    /* Xóa nội dung tệp log cũ */
    if (open(LOGFILE, O_CREAT | O_WRONLY | O_TRUNC, 0666) == -1)
    {
        error("log file open failed");
    }

    printf("[LOGGER] Khởi tạo xong! Chờ reader và writer kết nối...\n\n");

    /* ===== BƯỚC 5: CHẠY VÒNG LẶP VÔ TẬN ===== */
    /* Duy trì chương trình để giữ các semaphore và bộ nhớ chia sẻ */
    while (1)
    {
        sleep(1);
    }

    return 0;

    munmap(shared_mem_ptr, sizeof(struct shared_memory));
    close(fd_shm);

    return 0;
}
