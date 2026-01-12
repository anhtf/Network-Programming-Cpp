#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>

/* ===== ĐỊNH NGHĨA CÁC HẰNG SỐ ===== */
#define MAX_BUFFERS 10                                   /* Số buffer tối đa */
#define LOGFILE "/tmp/example.log"                      /* Tệp log */

#define SEM_MUTEX_NAME "/sem-mutex"                    /* Bảo vệ vùng tới hạn */
#define SEM_BUFFER_COUNT_NAME "/sem-buffer-count"       /* Đếm buffer trống */
#define SEM_SPOOL_SIGNAL_NAME "/sem-spool-signal"       /* Báo có dữ liệu mới */
#define SHARED_MEM_NAME "/posix-shared-mem-example"    /* Tên bộ nhớ chia sẻ */

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
void error(char *msg)
{
    perror(msg);
    exit(1);
}

/* ===== HÀM CHÍNH ===== */
/**
 * Chương trình writer - Ghi dữ liệu vào bộ nhớ chia sẻ
 * Công việc:
 *   1. Kết nối với bộ nhớ chia sẻ
 *   2. Kết nối với 3 semaphore
 *   3. Nhập dữ liệu từ stdin
 *   4. Ghi dữ liệu vào buffer chia sẻ
 *   5. Báo cho reader có dữ liệu mới
 */
int main(int argc, char **argv)
{
    struct shared_memory *shared_mem_ptr;  /* Con trỏ tới bộ nhớ chia sẻ */
    sem_t *mutex_sem, *buffer_count_sem, *spool_signal_sem;  /* Con trỏ semaphore */
    int fd_shm;  /* File descriptor của bộ nhớ chia sẻ */

    printf("[WRITER] Kết nối tới các resources...\n");

    /* ===== BƯỚC 1: KẾT NỐI SEMAPHORE MUTEX ===== */
    /* Dùng flag 0 nghĩa là chỉ kết nối, không tạo mới */
    if ((mutex_sem = sem_open(SEM_MUTEX_NAME, 0)) == SEM_FAILED)
        error("sem_open mutex");

    /* ===== BƯỚC 2: KẾT NỐI BỘ NHỚ CHIA SẺ ===== */
    if ((fd_shm = shm_open(SHARED_MEM_NAME, O_RDWR, 0)) == -1)
        error("shm_open");

    /* Ánh xạ bộ nhớ chia sẻ vào địa chỉ ảo */
    if ((shared_mem_ptr = mmap(NULL, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED,
                               fd_shm, 0)) == MAP_FAILED)
        error("mmap");

    /* ===== BƯỚC 3: KẾT NỐI SEMAPHORE BUFFER_COUNT ===== */
    /* Đếm số buffer trống. Writer sẽ chờ nếu buffer đầy */
    if ((buffer_count_sem = sem_open(SEM_BUFFER_COUNT_NAME, 0)) == SEM_FAILED)
        error("sem_open buffer_count");

    /* ===== BƯỚC 4: KẾT NỐI SEMAPHORE SPOOL_SIGNAL ===== */
    /* Báo có dữ liệu mới cho reader */
    if ((spool_signal_sem = sem_open(SEM_SPOOL_SIGNAL_NAME, 0)) == SEM_FAILED)
        error("sem_open spool_signal");

    printf("[WRITER] Kết nối thành công!\n\n");

    char buf[200], *cp;

    printf("[WRITER] Nhập tin nhắn (Ctrl+D để thoát):\n");
    
    /* ===== VÒNG LẶP CHÍNH - NHẬP DỮ LIỆU TỪ NGƯỜI DÙNG ===== */
    while (fgets(buf, 198, stdin))
    {
        /* Loại bỏ ký tự xuống dòng */
        int length = strlen(buf);
        if (buf[length - 1] == '\n')
            buf[length - 1] = '\0';

        printf("[WRITER] Nhập: %s\n", buf);

        /* ===== BƯỚC A: CHỜ BUFFER TRỐNG ===== */
        /* P(buffer_count_sem) - Giảm giá trị semaphore
           Nếu buffer đầy (giá trị = 0), sẽ chặn ở đây cho tới khi có buffer trống */
        printf("[WRITER]   -> Chờ buffer trống...\n");
        if (sem_wait(buffer_count_sem) == -1)
            error("sem_wait: buffer_count_sem");

        /* ===== BƯỚC B: KHÓA VÙNG TỚI HẠN ===== */
        /* P(mutex_sem) - Chỉ cho phép 1 writer cập nhật buffer_index cùng lúc */
        printf("[WRITER]   -> Khóa vùng tới hạn...\n");
        if (sem_wait(mutex_sem) == -1)
            error("sem_wait: mutex_sem");

        /* ===== BƯỚC C: GHI DỮ LIỆU VÀO BUFFER (VÙNG TỚI HẠN) ===== */
        time_t now = time(NULL);
        cp = ctime(&now);
        int len = strlen(cp);
        if (*(cp + len - 1) == '\n')
            *(cp + len - 1) = '\0';
        
        /* Định dạng: [PID]: [Thời gian] [Tin nhắn]\n */
        sprintf(shared_mem_ptr->buf[shared_mem_ptr->buffer_index], "%d: %s %s\n", getpid(),
                cp, buf);
        
        printf("[WRITER]   -> Ghi vào buffer[%d]\n", shared_mem_ptr->buffer_index);
        
        /* Cập nhật chỉ số buffer cho lần ghi tiếp theo */
        (shared_mem_ptr->buffer_index)++;
        if (shared_mem_ptr->buffer_index == MAX_BUFFERS)
            shared_mem_ptr->buffer_index = 0;

        /* ===== BƯỚC D: MỞ KHÓA VÙNG TỚI HẠN ===== */
        /* V(mutex_sem) - Giải phóng khóa để writer khác có thể ghi */
        printf("[WRITER]   -> Mở khóa vùng tới hạn\n");
        if (sem_post(mutex_sem) == -1)
            error("sem_post: mutex_sem");

        /* ===== BƯỚC E: BÁO CHO READER CÓ DỮ LIỆU MỚI ===== */
        /* V(spool_signal_sem) - Tăng giá trị semaphore, báo cho reader có dữ liệu */
        printf("[WRITER]   -> Báo cho reader có dữ liệu mới\n\n");
        if (sem_post(spool_signal_sem) == -1)
            error("sem_post: spool_signal_sem");

        printf("[WRITER] Nhập tin nhắn (Ctrl+D để thoát):\n");
    }

    printf("[WRITER] Thoát chương trình.\n");
    return 0;

    if (munmap(shared_mem_ptr, sizeof(struct shared_memory)) == -1)
        error("munmap");
    exit(0);
}
