# Shared Memory & Semaphore - Producer-Consumer System

## 📋 Tổng Quan Hệ Thống

Hệ thống này minh họa **Producer-Consumer Pattern** sử dụng **POSIX Shared Memory** và **Semaphore** để đồng bộ hóa dữ liệu giữa các process.

### Các Thành Phần:
- **logger**: Khởi tạo shared memory và semaphore
- **writer**: Producer - nhập dữ liệu và ghi vào buffer chia sẻ
- **reader**: Consumer - đọc dữ liệu từ buffer và ghi vào log file

---

## 🎯 Kiến Trúc Hệ Thống

```
┌─────────────┐         ┌──────────────────┐         ┌─────────────┐
│   LOGGER    │         │  SHARED MEMORY   │         │   LOG FILE  │
│             │         │  (10 buffers)    │         │             │
│ • Tạo SHM   │────────▶│ ┌──────────────┐ │◀────────│ /tmp/       │
│ • Tạo SEM   │         │ │ buf[0...9]   │ │         │ example.log │
│ • Init      │         │ └──────────────┘ │         │             │
└─────────────┘         │ buffer_index: 0  │         └─────────────┘
      ▲                 │ print_index: 0   │              ▲
      │                 └──────────────────┘              │
      │                         ▲                         │
      │          ┌──────────────┼──────────────┐          │
      │          │              │              │          │
      │   [mutex_sem=1] [buffer_count=10] [spool_signal=0]
      │          │              │              │
      │    ┌─────▼─────┐    ┌───▼────┐    ┌───▼────┐
      │    │  WRITER   │    │ READER │    │ READER │
      └────┤           │    │        │    │        │
           │ Nhập tin  │    │ Đọc    │    │ Ghi    │
           │ Khóa SEM  │    │ Khóa   │    │ Log    │
           │ Ghi buf   │    │ SEM    │    │        │
           │ Mở SEM    │    │ Mở SEM │    │ Post   │
           │ Post SEM  │    │ Ghi    │    │ Buffer │
           └───────────┘    │ Log    │    │ Count  │
                            │ Post   │    │        │
                            │ Buffer │    └────────┘
                            │ Count  │
                            └────────┘
```

---

## 📚 CHI TIẾT CÁC HÀM

### 1. SEMAPHORE FUNCTIONS

#### `sem_wait()` - P (Lock/Decrement)
```c
if (sem_wait(buffer_count_sem) == -1)
    error("sem_wait");
```

**Ý Nghĩa:**
- Giảm giá trị semaphore đi 1
- Nếu giá trị > 0: Giảm ngay và tiếp tục
- Nếu giá trị = 0: **BỊ CHẶN** cho tới khi có process gọi `sem_post()`

**Ví Dụ:**
```
Writer chờ buffer trống:
  Khởi đầu: buffer_count_sem = 10
  Writer 1: sem_wait() → 10 → 9 (tiếp tục)
  Writer 2: sem_wait() → 9 → 8 (tiếp tục)
  ...
  Writer 10: sem_wait() → 1 → 0 (tiếp tục)
  Writer 11: sem_wait() → 0 → CAN'T DEC! (BỊ CHẶN)
             (chờ Reader gọi sem_post() để tăng lại)
```

#### `sem_post()` - V (Unlock/Increment)
```c
if (sem_post(spool_signal_sem) == -1)
    error("sem_post");
```

**Ý Nghĩa:**
- Tăng giá trị semaphore lên 1
- Nếu có process bị chặn: **THỨC DẬY** 1 process
- Nếu không có: Giá trị semaphore tăng bình thường

**Ví Dụ:**
```
Reader báo có data:
  Trước: spool_signal_sem = 0 (Writer chặn)
  Reader: sem_post() → 0 → 1
  Kết quả: Writer TỰ ĐỘNG THỨC DẬY!
```

#### `sem_open()` - Tạo/Kết nối Semaphore
```c
// LOGGER - Tạo mới
if ((sem = sem_open("/sem-name", O_CREAT, 0666, 1)) == SEM_FAILED)
    error("sem_open");

// WRITER/READER - Chỉ kết nối
if ((sem = sem_open("/sem-name", 0)) == SEM_FAILED)
    error("sem_open");
```

| Tham số | Ý Nghĩa |
|--------|---------|
| `/sem-name` | Tên định danh (bắt đầu bằng /) |
| `O_CREAT` | Tạo mới nếu chưa tồn tại |
| `0666` | Quyền file (rw- rw- rw-) |
| Giá trị ban đầu | Giá trị khởi tạo (Logger dùng) |
| `0` | Chỉ kết nối, không tạo (Writer/Reader dùng) |

---

### 2. SHARED MEMORY FUNCTIONS

#### `shm_open()` - Tạo/Kết nối Bộ Nhớ Chia Sẻ
```c
// LOGGER - Tạo mới
if ((fd_shm = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666)) == -1)
    error("shm_open");

// WRITER/READER - Chỉ kết nối
if ((fd_shm = shm_open(SHARED_MEM_NAME, O_RDWR, 0)) == -1)
    error("shm_open");
```

**Tham Số:**
- `SHARED_MEM_NAME`: "/posix-shared-mem-example"
- `O_CREAT | O_RDWR`: Tạo mới + quyền đọc/ghi
- `0666`: Quyền file
- Trả về: File descriptor (như file thường)

#### `ftruncate()` - Cấp Phát Kích Thước
```c
if (ftruncate(fd_shm, sizeof(struct shared_memory)) == -1)
    error("ftruncate");
```

**Ý Nghĩa:**
- Cấp phát bộ nhớ vật lý cho shared memory
- Cần thiết trước khi `mmap()`
- Chỉ Logger gọi

**Ví Dụ:**
```
ftruncate(fd_shm, sizeof(struct shared_memory))
    ↓
Cấp phát: 10 * 256 + 2*sizeof(int) = 2560 + 8 = 2568 bytes
```

#### `mmap()` - Ánh Xạ Bộ Nhớ Chia Sẻ
```c
if ((shared_mem_ptr = mmap(NULL, sizeof(struct shared_memory),
                           PROT_READ | PROT_WRITE, MAP_SHARED,
                           fd_shm, 0)) == MAP_FAILED)
    error("mmap");
```

| Tham Số | Ý Nghĩa |
|--------|---------|
| `NULL` | Hệ thống tự chọn địa chỉ |
| `sizeof(...)` | Kích thước cần ánh xạ |
| `PROT_READ \| PROT_WRITE` | Quyền đọc/ghi |
| `MAP_SHARED` | **QUAN TRỌNG**: Chia sẻ với process khác |
| `fd_shm` | File descriptor |
| `0` | Offset bắt đầu |

**Kết Quả:**
- Con trỏ `shared_mem_ptr` chỉ tới bộ nhớ chia sẻ
- Tất cả process có thể truy cập qua con trỏ này

---

## 🔄 LUỒNG HOẠT ĐỘNG CHI TIẾT

### BƯỚC 1: LOGGER Khởi Tạo

```c
[LOGGER] Xóa resources cũ
├─ shm_unlink(SHARED_MEM_NAME)           // Xóa shared memory cũ
├─ sem_unlink(SEM_MUTEX_NAME)            // Xóa semaphore cũ
├─ sem_unlink(SEM_BUFFER_COUNT_NAME)
└─ sem_unlink(SEM_SPOOL_SIGNAL_NAME)

[LOGGER] Tạo Shared Memory
├─ shm_open() → fd_shm
├─ ftruncate() → cấp phát bộ nhớ
└─ mmap() → shared_mem_ptr
   └─ shared_mem_ptr:
      ├─ buf[10][256]        // 10 buffers, mỗi 256 bytes
      ├─ buffer_index = 0    // Chỉ số ghi (writer)
      └─ buffer_print_index = 0  // Chỉ số đọc (reader)

[LOGGER] Tạo 3 Semaphore
├─ sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1)
│  └─ Giá trị = 1 → Bảo vệ vùng tới hạn
│
├─ sem_open(SEM_BUFFER_COUNT_NAME, O_CREAT, 0666, 10)
│  └─ Giá trị = 10 → Đếm buffer trống
│
└─ sem_open(SEM_SPOOL_SIGNAL_NAME, O_CREAT, 0666, 0)
   └─ Giá trị = 0 → Báo có dữ liệu mới
```

### BƯỚC 2: WRITER Ghi Dữ Liệu

```c
User nhập: "Xin chào"
    ↓

A. CHỜ BUFFER TRỐNG
   sem_wait(buffer_count_sem)  // P()
   ├─ Giảm: 10 → 9
   ├─ Nếu = 0 → BỊ CHẶN (chờ Reader)
   └─ Tiếp tục...

B. KHÓA VÙNG TỚI HẠN
   sem_wait(mutex_sem)  // P()
   ├─ Giảm: 1 → 0
   ├─ Writer khác BỊ CHẶN
   └─ Vào vùng tới hạn

C. VÙNG TỚI HẠN (chỉ 1 writer vào)
   ├─ timestamp = time(NULL)
   │  └─ "Sun Jan 12 15:30:45 2026"
   │
   ├─ sprintf() tạo message:
   │  └─ "12345: Sun Jan 12 15:30:45 2026 Xin chào\n"
   │
   ├─ Ghi vào buffer:
   │  └─ shared_mem_ptr→buf[buffer_index] = message
   │
   └─ Cập nhật chỉ số:
      ├─ buffer_index++
      └─ Nếu buffer_index >= 10 → reset = 0

D. MỞ KHÓA VÙNG TỚI HẠN
   sem_post(mutex_sem)  // V()
   ├─ Tăng: 0 → 1
   └─ Writer khác (nếu chặn) THỨC DẬY

E. BÁO CÓ DỮ LIỆU MỚI
   sem_post(spool_signal_sem)  // V()
   ├─ Tăng: 0 → 1
   └─ Reader (nếu chặn) THỨC DẬY

Trạng thái sau:
├─ buf[0] = "12345: Sun Jan 12 15:30:45 2026 Xin chào\n"
├─ buffer_index = 1
├─ buffer_count_sem = 9
└─ spool_signal_sem = 1
```

### BƯỚC 3: READER Đọc Dữ Liệu

```c
A. CHỜ DỮ LIỆU MỚI
   sem_wait(spool_signal_sem)  // P()
   ├─ Giảm: 1 → 0
   ├─ Nếu = 0 → BỊ CHẶN (chờ Writer)
   └─ Tiếp tục...

B. KHÓA VÙNG TỚI HẠN
   sem_wait(mutex_sem)  // P()
   ├─ Giảm: 1 → 0
   ├─ Reader khác BỊ CHẶN
   └─ Vào vùng tới hạn

C. VÙNG TỚI HẠN (chỉ 1 reader vào)
   ├─ Đọc dữ liệu:
   │  └─ strcpy(buffer, buf[buffer_print_index])
   │     └─ buffer = "12345: Sun Jan 12 15:30:45 2026 Xin chào\n"
   │
   └─ Cập nhật chỉ số:
      ├─ buffer_print_index++
      └─ Nếu >= 10 → reset = 0

D. MỞ KHÓA VÙNG TỚI HẠN
   sem_post(mutex_sem)  // V()
   ├─ Tăng: 0 → 1
   └─ Reader khác (nếu chặn) THỨC DẬY

E. GHI VÀO LOG FILE
   write(log_fd, buffer, strlen(buffer))
   └─ /tmp/example.log:
      "12345: Sun Jan 12 15:30:45 2026 Xin chào\n"

F. BÁO BUFFER TRỐNG
   sem_post(buffer_count_sem)  // V()
   ├─ Tăng: 9 → 10
   └─ Writer (nếu chặn) THỨC DẬY

Trạng thái sau:
├─ buffer_print_index = 1
├─ buffer_count_sem = 10
├─ spool_signal_sem = 0
└─ Log file được cập nhật
```

---

## 🔒 SYNCHRONIZATION - ĐỒng BỘ HÓA

### Tình Huống 1: Buffer Đầy (WRITER CHẶN)

```
Buffer có 10 vị trí, tất cả đầy
buffer_count_sem = 0

┌─────────────────────────────────────────┐
│ Writer nhập tin mới                      │
└─────────────────────────────────────────┘
  ↓
A. sem_wait(buffer_count_sem)
   ├─ Giá trị = 0 → CAN'T DECREASE
   └─ WRITER BỊ CHẶN (ngủ, không dùng CPU)
      (đợi Reader)

                    [... Reader đang xử lý ...]

F. Reader xong, gọi:
   sem_post(buffer_count_sem)
   ├─ Tăng: 0 → 1
   └─ WRITER TỰ ĐỘNG THỨC DẬY!
      (hệ thống OS gọi sem_wait() hoàn thành)
      
Writer tiếp tục:
  ├─ buffer_count_sem = 0 (giảm lại)
  └─ Tiếp tục ghi dữ liệu
```

### Tình Huống 2: Không Có Data (READER CHẶN)

```
Không có data mới
spool_signal_sem = 0

┌─────────────────────────────────────────┐
│ Reader chờ dữ liệu mới                   │
└─────────────────────────────────────────┘
  ↓
A. sem_wait(spool_signal_sem)
   ├─ Giá trị = 0 → CAN'T DECREASE
   └─ READER BỊ CHẶN (ngủ, không dùng CPU)
      (đợi Writer)

                    [... Writer đang nhập ...]

E. Writer xong, gọi:
   sem_post(spool_signal_sem)
   ├─ Tăng: 0 → 1
   └─ READER TỰ ĐỘNG THỨC DẬY!
      (hệ thống OS gọi sem_wait() hoàn thành)
      
Reader tiếp tục:
  ├─ spool_signal_sem = 0 (giảm lại)
  └─ Bắt đầu đọc dữ liệu
```

### Tình Huống 3: Vùng Tới Hạn (Mutual Exclusion)

```
Giả sử có 2 writers cùng lúc

❌ SAI (không có mutex):
Writer 1:
  └─ Ghi buf[0]: buf[0] = "Message 1"
Writer 2 (cùng lúc):
  └─ Ghi buf[0]: buf[0] = "Message 2"
Kết quả: buffer_index đồng thời cập nhật
         → RACE CONDITION! Dữ liệu hỏng!

✅ ĐÚNG (có mutex_sem):
Writer 1:
  B. sem_wait(mutex_sem)  → 1 → 0 (VÀO)
  C. Ghi buf[0]: buf[0] = "Message 1"
  D. sem_post(mutex_sem)  → 0 → 1 (RA)

Writer 2 (cùng lúc):
  B. sem_wait(mutex_sem)  → CAN'T! (CHẶN)
     (chờ Writer 1 giải phóng)
     
  (Writer 1 ở D. post() xong)
  
  B. sem_wait(mutex_sem)  → 1 → 0 (THỨC DẬY, VÀO)
  C. Ghi buf[1]: buf[1] = "Message 2"
  D. sem_post(mutex_sem)  → 0 → 1 (RA)

Kết quả: Dữ liệu an toàn! ✓
```

---

## 📊 BẢNG TÓM TẮT 3 SEMAPHORE

| Semaphore | Ban Đầu | Tác Dụng | Wait Khi | Post Khi |
|-----------|---------|---------|---------|----------|
| **mutex_sem** | 1 | Bảo vệ vùng tới hạn (mutual exclusion) | Khi vào vùng tới hạn | Khi rời vùng tới hạn |
| **buffer_count_sem** | 10 | Đếm số buffer trống | Writer muốn ghi (chặn nếu đầy) | Reader xong (báo 1 buffer trống) |
| **spool_signal_sem** | 0 | Báo có dữ liệu mới | Reader chờ data (chặn nếu không có) | Writer ghi xong (báo có data) |

---

## 🔧 Cấu Trúc Dữ Liệu

```c
struct shared_memory
{
    char buf[MAX_BUFFERS][256];  // 10 buffers, mỗi 256 bytes
    int buffer_index;            // Writer ghi vào buf[buffer_index]
    int buffer_print_index;      // Reader đọc từ buf[buffer_print_index]
};
```

**Ví Dụ:**
```
Khởi đầu:
  buffer_index = 0
  buffer_print_index = 0

Writer 1 ghi xong:
  buf[0] = "Message 1\n"
  buffer_index = 1 (next write position)

Reader đọc xong:
  buffer_print_index = 1 (next read position)
```

---

## 🚀 Cách Chạy

### 1. Biên Dịch
```bash
cd Shared_Memory
gcc -o logger logger.c -lpthread -lrt
gcc -o writer writer.c -lpthread -lrt
gcc -o reader reader.c -lpthread -lrt
```

### 2. Chạy trong 3 Terminal Riêng Biệt

**Terminal 1: Logger (khởi tạo resources)**
```bash
./logger
# Output:
# [LOGGER] Xóa các resources cũ...
# [LOGGER] Tạo bộ nhớ chia sẻ...
# [LOGGER] Tạo các semaphore...
# [LOGGER] Khởi tạo xong! Chờ reader và writer kết nối...
```

**Terminal 2: Reader (chờ dữ liệu)**
```bash
./reader
# Output:
# [READER] Kết nối tới các resources...
# [READER] Kết nối thành công!
# [READER] Chờ dữ liệu từ writer...
```

**Terminal 3: Writer (nhập dữ liệu)**
```bash
./writer
# Output:
# [WRITER] Kết nối tới các resources...
# [WRITER] Kết nối thành công!
# [WRITER] Nhập tin nhắn (Ctrl+D để thoát):
# Xin chào
# [WRITER] Nhập: Xin chào
# [WRITER]   -> Chờ buffer trống...
# [WRITER]   -> Khóa vùng tới hạn...
# [WRITER]   -> Ghi vào buffer[0]
# [WRITER]   -> Mở khóa vùng tới hạn
# [WRITER]   -> Báo cho reader có dữ liệu mới
```

### 3. Xem Log File
```bash
cat /tmp/example.log
# Output:
# 12345: Sun Jan 12 15:30:45 2026 Xin chào
# 12345: Sun Jan 12 15:30:47 2026 Test message
```

---

## 📝 File Descriptions

### logger.c
- **Khởi tạo**: Shared Memory + 3 Semaphore
- **Xóa resources cũ**: Tránh lỗi khi chạy lại
- **Duy trì process**: Giữ resources tồn tại

### writer.c
- **Nhập dữ liệu**: Từ stdin
- **Chờ buffer trống**: `sem_wait(buffer_count_sem)`
- **Ghi vào buffer**: Vào vùng tới hạn
- **Báo Reader**: `sem_post(spool_signal_sem)`

### reader.c
- **Chờ dữ liệu**: `sem_wait(spool_signal_sem)`
- **Đọc từ buffer**: Vào vùng tới hạn
- **Ghi log file**: Write buffer vào /tmp/example.log
- **Báo Writer**: `sem_post(buffer_count_sem)`

---

## 🎓 Kiến Thức Chính

1. **Shared Memory**: Cho phép multiple processes chia sẻ bộ nhớ
2. **Semaphore**: Dùng để đồng bộ hóa giữa processes
3. **Mutex**: Loại semaphore bảo vệ vùng tới hạn
4. **Circular Buffer**: Dữ liệu quay vòng (0→9→0)
5. **Producer-Consumer**: Pattern cơ bản trong lập trình concurrent
6. **Race Condition**: Khi không có synchronization → dữ liệu hỏng
7. **Deadlock Prevention**: Thứ tự sem_wait/sem_post rất quan trọng

---

## ⚠️ Lưu Ý Quan Trọng

1. **Logger phải chạy trước**: Để khởi tạo resources
2. **Ctrl+C để thoát**: Nếu muốn xóa resources, chạy logger lại
3. **Không chia sẻ semaphore**: Tất cả processes dùng cùng tên
4. **Flag khác nhau**: Logger dùng `O_CREAT`, Writer/Reader dùng flag 0
5. **Không cùng vùng tới hạn**: Writer và Reader có vùng tới hạn riêng (có thể ghi/đọc xen kẽ nếu cần)

---

## 📚 Tài Liệu Tham Khảo

- POSIX Shared Memory: `man shm_open`
- POSIX Semaphore: `man sem_open`, `man sem_wait`, `man sem_post`
- Memory Mapping: `man mmap`
- System V IPC: `man ipcs`, `man ipcrm`

---

Được tạo: January 12, 2026
Ngôn ngữ: C
Standard: POSIX
Platform: Linux
