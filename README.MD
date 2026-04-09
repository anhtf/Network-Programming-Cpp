# Tổng Quan Kiến Thức Lập Trình Mạng C++

## Giới Thiệu

Kho lưu trữ này chứa các ví dụ và bài học tự học về lập trình mạng và lập trình hệ thống song song trong ngôn ngữ C++. Các chủ đề chính bao gồm giao tiếp giữa các tiến trình (IPC), tín hiệu, giao tiếp nối tiếp, bộ nhớ chia sẻ, socket, và luồng (threads). Mỗi phần đều có mã nguồn minh họa và tài liệu giải thích chi tiết.

## Mục Lục

- [IPC (Inter-Process Communication)](#ipc-inter-process-communication)
  - [Message Queues](#message-queues)
  - [Pipes](#pipes)
- [Signals](#signals)
- [Serial Communication](#serial-communication)
- [Shared Memory](#shared-memory)
- [Sockets](#sockets)
  - [TCP](#tcp)
  - [UDP](#udp)
  - [Unix Domain Sockets](#unix-domain-sockets)
  - [Non-blocking Sockets](#non-blocking-sockets)
- [Threads](#threads)

## IPC (Inter-Process Communication)

IPC là các cơ chế cho phép các tiến trình (processes) giao tiếp và đồng bộ hóa với nhau. Trong kho này, chúng ta học về Message Queues và Pipes.

### Message Queues

Message Queues cho phép gửi và nhận tin nhắn giữa các tiến trình. Có hai loại chính:

- **System V Message Queues**: Sử dụng key số, các hàm như `msgget`, `msgsnd`, `msgrcv`. Không hỗ trợ file descriptor, không có thông báo.
- **POSIX Message Queues**: Sử dụng đường dẫn ảo, các hàm như `mq_open`, `mq_send`, `mq_receive`. Hỗ trợ file descriptor, thông báo, và ưu tiên.

[Xem chi tiết và mã nguồn](IPC/Message-Queue/)

### Pipes

Pipes là kênh giao tiếp một chiều giữa các tiến trình liên quan.

- **Anonymous Pipes**: Tạo pipe ẩn danh, chỉ dùng cho tiến trình cha-con.
- **Named Pipes (FIFOs)**: Tạo pipe có tên, có thể dùng cho các tiến trình không liên quan.

[Xem mã nguồn](IPC/Pipes/)

## Signals

Signals là cách hệ thống gửi tín hiệu đến tiến trình để xử lý các sự kiện bất đồng bộ như ngắt (interrupt) hoặc lỗi.

Học về cách cài đặt handler cho signals, như SIGINT, SIGTERM, và cách sử dụng `signal()` hoặc `sigaction()`.

[Xem mã nguồn](Signals/)

## Serial Communication

Giao tiếp nối tiếp qua cổng serial, sử dụng UART. Học về cấu hình cổng serial, đọc/ghi dữ liệu, và xử lý lỗi.

[Xem mã nguồn](Serial/)

## Shared Memory

Shared Memory cho phép nhiều tiến trình chia sẻ vùng nhớ chung, tăng hiệu suất so với IPC khác.

Trong ví dụ Producer-Consumer, sử dụng POSIX Shared Memory và Semaphore để đồng bộ hóa:

- **Logger**: Khởi tạo shared memory và semaphore.
- **Writer**: Producer ghi dữ liệu vào buffer.
- **Reader**: Consumer đọc dữ liệu và ghi log.

Các hàm semaphore: `sem_init`, `sem_wait`, `sem_post`, v.v.

[Xem chi tiết và mã nguồn](Shared_Memory/)

## Sockets

Sockets là giao diện lập trình mạng, cho phép giao tiếp qua mạng.

### TCP

Giao thức hướng kết nối, đảm bảo độ tin cậy. Ví dụ client-server.

[Xem mã nguồn](Sockets/tcp/)

### UDP

Giao thức không kết nối, nhanh nhưng không đảm bảo. Ví dụ client-server.

[Xem mã nguồn](Sockets/udp/)

### Unix Domain Sockets

Sockets cho giao tiếp cục bộ, không qua mạng.

[Xem mã nguồn](Sockets/unix/)

### Non-blocking Sockets

Sockets không chặn, cho phép xử lý nhiều kết nối đồng thời.

[Xem mã nguồn](Sockets/non-blocking/)

## Threads

Threads là đơn vị thực thi nhỏ hơn trong một tiến trình, chia sẻ bộ nhớ.

Học về:

- Sự khác biệt giữa Process và Thread.
- Tạo và quản lý threads với Pthreads.
- Đồng bộ hóa với Mutex và Condition Variables để tránh race conditions.

Ví dụ minh họa Producer-Consumer với threads.

[Xem chi tiết và mã nguồn](Threads/)

## Kết Luận

Kho này tổng hợp kiến thức tự học về lập trình hệ thống và mạng trong C++. Mỗi phần có mã nguồn thực tế và tài liệu để hiểu sâu hơn. Để chạy các ví dụ, cần môi trường Linux với GCC và các thư viện cần thiết.</content>
<parameter name="filePath">/home/anhtdh/Documents/GitHub/Network-Programming-Cpp/README.MD
