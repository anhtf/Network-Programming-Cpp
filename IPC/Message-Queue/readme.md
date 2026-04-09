Tiêu chí                System V MQ (Cũ)                           POSIX MQ (Mới)
Định danh               Dùng số Key (ftok)                         Dùng đường dẫn ảo (/ten_queue)
Hàm tạo/mở              msgget                                     ()mq_open()
Hàm gửi/nhận            msgsnd(), msgrcv()                         mq_send(), mq_receive()
Có File Descriptor?     Không (Không dùng được select/poll)        Có (Kết hợp hoàn hảo với Socket)
Thông báo(Notify)       Không có                                   Có (mq_notify)
Gắn cờ Ưu tiên          Phải tự chế qua biến mtype                 Hỗ trợ Native Priority (1-32768)
Thư viện khi dịch       Không cần thêm cờ                          Phải thêm -lrt (gcc -lrt)
Sử dụng thực tế         Duy trì hệ thống Legacy                    Thiết kế kiến trúc mới hiện nay