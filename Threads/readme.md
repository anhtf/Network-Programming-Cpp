1. Tiến trình (Process) = Một nhà máy độc lập
Khi bạn chạy một chương trình trên Linux (ví dụ chạy file ./server), hệ điều hành sẽ tạo ra một Process.

Hãy coi Process là một Nhà máy.

Nhà máy này được cấp đất đai riêng (Bộ nhớ RAM ảo độc lập), có mã số thuế riêng (PID - Process ID), có kho chứa vật liệu riêng.

Multi-process (Đa tiến trình): Nếu bạn muốn làm 2 chiếc xe cùng lúc, bạn xây 2 nhà máy.

Ưu điểm: An toàn tuyệt đối. Nhà máy A cháy thì nhà máy B không sao. Chương trình A lỗi crash thì chương trình B vẫn chạy bình thường.

Nhược điểm: Tốn quá nhiều tiền và tài nguyên để xây nhà máy mới. Nếu nhân viên nhà máy A muốn đưa một cái lốp xe sang nhà máy B, họ phải đóng gói, thuê xe tải chở đi (Đây chính là IPC - Inter Process Communication mà chúng ta sẽ học ở trạm cuối). Rất chậm và cồng kềnh!

2. Luồng (Thread) = Công nhân trong nhà máy
Thread chính là các công nhân làm việc bên trong cái nhà máy (Process) đó.

Một nhà máy (Process) khi mới mở ra luôn có ít nhất 1 công nhân (gọi là Main Thread).

Multi-thread (Đa luồng): Thay vì xây nhà máy mới, bạn chỉ việc tuyển thêm công nhân thứ 2, thứ 3 vào làm chung trong cùng một nhà máy.

Ưu điểm cực lớn: Tạo ra một công nhân mới (Thread) rẻ và nhanh hơn gấp ngàn lần so với việc xây nhà máy mới (Process). Các công nhân dùng chung một kho chứa hàng (Share chung bộ nhớ RAM, chung các biến toàn cục - global variables). Công nhân A chỉ cần đặt cái lốp xe xuống sàn, công nhân B có thể lấy lắp vào xe ngay lập tức mà không cần "thuê xe tải".

Nhược điểm (Rủi ro): Vì dùng chung không gian, nếu công nhân A làm nổ bình gas (lỗi bộ nhớ - Segmentation Fault), toàn bộ nhà máy sẽ nổ tung (cả chương trình bị crash). Hoặc nếu không có quy định rõ ràng, hai công nhân có thể cùng tranh nhau lấy một cái cờ lê gây ra đánh lộn (Race condition).

3. Giải pháp: Mutex (Mutual Exclusion) - Chiếc chìa khóa nhà vệ sinh
Để giải quyết vấn đề này, Pthread cung cấp một khái niệm gọi là Mutex (Viết tắt của Mutual Exclusion - Loại trừ lẫn nhau).

Hãy coi biến counter là cái bồn cầu, và Mutex là chiếc chìa khóa duy nhất của phòng vệ sinh đó.

Luồng 1 muốn dùng bồn cầu, nó phải lấy chìa khóa (pthread_mutex_lock).

Luồng 1 vào trong, chốt cửa lại.

Luồng 2 muốn dùng bồn cầu, nhưng không thấy chìa khóa đâu -> Nó bắt buộc phải đứng ngoài cửa chờ (Bị Block).

Luồng 1 đi xong (counter++), mở cửa và trả lại chìa khóa (pthread_mutex_unlock).

Lúc này Luồng 2 mới vồ lấy chìa khóa, khóa cửa và thực hiện việc của mình

4. Trong thời gian 1 luồng (Thread 1) đang ở trong "nhà vệ sinh" và chốt cửa bằng Mutex, thì tất cả các luồng khác (Thread 2, 3, 4...) khi chạy đến dòng pthread_mutex_lock sẽ lập tức bị hệ điều hành chuyển sang trạng thái "Blocked" (Bị nghẽn/Chờ đợi).

Nhược điểm chí mạng ở đây là:

Mất đi tính song song (Loss of Concurrency): Dù bạn có tạo ra 100 luồng để chạy cho nhanh, nhưng khi tất cả cùng đâm đầu vào một "nút thắt cổ chai" (bottleneck) được khóa bằng Mutex, chương trình của bạn lại vô tình biến thành chương trình... đơn luồng (chạy tuần tự từng cái một).
Lãng phí tài nguyên CPU (Context Switch Overhead): CPU phải thực hiện thao tác cất giữ trạng thái của luồng đang chạy, đưa nó vào hàng đợi chờ, rồi khi cửa mở lại phải "đánh thức" nó dậy, tải lại trạng thái. Việc chuyển đổi liên tục này làm giảm hiệu năng hệ thống đáng kể.

5. Tối ưu hóa CPU với Condition Variable

pthread_cond_wait(&cond, &mutex): Lệnh đi ngủ. (Hàm này bắt buộc phải đi kèm với Mutex. 
                                                Khi luồng bắt đầu ngủ, hàm này sẽ tự động MỞ KHÓA Mutex để Producer có thể vào bỏ ảnh. 
                                                Ngay khi luồng được đánh thức, nó sẽ tự động KHÓA LẠI Mutex trước khi chạy tiếp).

pthread_cond_signal(&cond): Đánh thức 1 luồng đang ngủ.

pthread_cond_broadcast(&cond): Đánh thức TẤT CẢ các luồng đang ngủ

6. Deadlock

Nguyên tắc 1: Lock Ordering (Thứ tự khóa tĩnh)
Đây là cách phổ biến và an toàn nhất (đã được mô phỏng ở Kịch bản An toàn phía trên).
Bất cứ khi nào một luồng cần nhiều hơn 1 ổ khóa, hệ thống bắt buộc tất cả các luồng phải xin khóa theo một thứ tự duy nhất được quy định trước.

Ví dụ: Quy định luôn phải lấy Khóa A rồi mới đến Khóa B. Dù Luồng 2 muốn chuyển tiền từ B sang A, nó cũng phải xin Khóa A trước! Nhờ vậy, sẽ không có chuyện "Mỗi thằng giữ một nửa".

Nguyên tắc 2: Dùng pthread_mutex_trylock()
Thay vì dùng pthread_mutex_lock() (sẽ bị block mù quáng nếu khóa đã bị người khác giữ), ta dùng trylock. Hàm này chỉ thử vặn khóa, nếu cửa khóa đang đóng, nó sẽ trả về ngay một mã lỗi (thường là EBUSY) thay vì đứng chờ.

Luồng xử lý: "Tôi đã có Khóa A. Tôi thử nghiệm trylock Khóa B. Ồ, Khóa B đang bận! Vậy tôi sẽ MỞ KHÓA A ra, lui lại một bước, đi pha cốc cà phê (sleep một lát), rồi quay lại xin lại Khóa A từ đầu". Nhờ việc chịu nhả khóa ra, Luồng 2 sẽ có cơ hội hoàn thành công việc.

Nguyên tắc 3: Hạn chế tối đa thời gian giữ khóa
Chỉ đặt hàm lock() và unlock() bao quanh đúng dòng code nào dùng chung dữ liệu. Đừng bao giờ gọi các hàm tốn nhiều thời gian (như đọc file, in ra màn hình, tải file qua mạng, dùng sleep) trong lúc đang giữ Mutex.