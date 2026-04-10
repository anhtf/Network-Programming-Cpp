![alt text](<Screenshot From 2026-04-09 11-05-51.png>)

Bài 1: Kiến trúc Publish/Subscribe & Vai trò của Broker
Khi mới học lập trình mạng, chúng ta thường quen với giao thức HTTP (dùng trên trình duyệt web). HTTP hoạt động theo mô hình Request/Response (Yêu cầu/Phản hồi): Khách hàng hỏi thì máy chủ mới trả lời.

Tại sao HTTP lại "thảm họa" đối với IoT?
Giả sử bạn có 1 cái ứng dụng điện thoại muốn xem nhiệt độ từ bo mạch Raspberry Pi ở nhà.

Nếu dùng HTTP, ứng dụng cứ mỗi 1 giây lại phải gõ cửa mạng: "Có nhiệt độ mới chưa?". Pi trả lời: "Chưa, vẫn 30 độ". Lại hỏi: "Có chưa?" - "Chưa".

Việc này cực kỳ tốn pin, tốn băng thông và làm nghẽn mạng (gọi là Polling).

MQTT (Message Queuing Telemetry Transport) ra đời để đập tan mô hình đó, thay bằng mô hình Publish/Subscribe (Xuất bản/Đăng ký).

1. Mô hình Publish/Subscribe hoạt động như thế nào?
Hãy tưởng tượng MQTT giống hệt như mạng xã hội YouTube.

Publisher (Người xuất bản): Cảm biến nhiệt độ giống như một YouTuber. Khi nào có video mới (nhiệt độ thay đổi), YouTuber tự động tải video đó lên kênh của mình.

Subscriber (Người đăng ký): Ứng dụng điện thoại của bạn giống như một người xem. Bạn bấm "Subscribe" kênh nhiệt độ đó.

Broker (Máy chủ trung tâm): Chính là nền tảng YouTube.

Điểm kỳ diệu ở đây là Publisher và Subscriber không hề biết đến sự tồn tại của nhau.
Bo mạch đo nhiệt độ không cần biết IP của cái điện thoại là gì. Cái điện thoại cũng không cần kết nối trực tiếp với bo mạch. Tất cả mọi người chỉ cần kết nối đến một người duy nhất: Broker.

Khi bo mạch gửi (Publish) 1 bản tin lên Broker, Broker sẽ lật sổ ra xem có bao nhiêu người đang Đăng ký (Subscribe) cái bản tin đó, và tự động photocopy bản tin đó ra, phân phát đến tận tay từng người. Điện thoại của bạn chỉ việc nằm im, khi nào có nhiệt độ mới, dữ liệu sẽ tự động "ting ting" đẩy về máy!

2. Những ưu điểm biến MQTT thành "Vua của IoT"
Siêu nhẹ (Lightweight): Phần Header (tiêu đề gói tin) của MQTT chỉ bé đúng 2 Bytes. Nó cực kỳ tiết kiệm 3G/4G cho các thiết bị chạy bằng pin.

Chống chịu mạng kém: Mạng chập chờn, rớt mạng? Không sao cả, MQTT thiết kế riêng cho các môi trường bất ổn.

Bất đồng bộ (Decoupled): Bo mạch (Publisher) bị mất điện tắt ngúm? Ứng dụng điện thoại (Subscriber) vẫn không bị lỗi crash, nó chỉ đơn giản là đứng chờ ở Broker cho đến khi bo mạch có điện và gửi tin nhắn tiếp theo.

Bài 2: Topic & Wildcards – Nghệ thuật tổ chức luồng dữ liệu.

Ở bài trước, chúng ta đã biết Broker đóng vai trò như một bưu điện trung tâm. Vậy làm thế nào để bưu điện biết bức thư nào cần gửi cho ai? Câu trả lời nằm ở Topic. Nếu không có một hệ thống Topic khoa học, hệ thống IoT của bạn sẽ sớm trở thành một mớ hỗn độn khi số lượng thiết bị tăng lên.

1. Topic là gì? Cấu trúc phân cấp (Hierarchy)
Topic trong MQTT không cần phải được tạo trước trên Broker. Nó được tạo ra ngay khi có một Client xuất bản (Publish) dữ liệu vào đó.

Topic được định nghĩa dưới dạng một chuỗi ký tự, phân tách bằng dấu gạch chéo / để tạo ra cấu trúc phân cấp, tương tự như đường dẫn thư mục trong máy tính.

Ví dụ về cấu trúc chuẩn cho một tòa nhà:
nha_cua/tang_1/phong_khach/nhiet_do

Quy tắc vàng khi đặt tên Topic:

Phân biệt chữ hoa - chữ thường: Home/Light khác hoàn toàn với home/light.

Không bắt đầu bằng dấu /: Mặc dù /home/light hợp lệ nhưng nó sẽ tạo ra một cấp độ trống ở đầu, gây lãng phí tài nguyên.

Tránh dấu cách và ký tự đặc biệt: Chỉ nên dùng chữ cái, số và dấu gạch dưới _.

2. Wildcards – Sức mạnh của sự linh hoạt
Giả sử bạn có 100 căn phòng và muốn nhận dữ liệu nhiệt độ của tất cả các phòng. Thay vì phải Subscribe 100 lần, MQTT cung cấp hai ký tự đại diện (Wildcards) cực kỳ mạnh mẽ:

a. Single-level Wildcard (Dấu cộng +)
Dấu + đại diện cho duy nhất một cấp độ trong Topic.

Ví dụ: nha_cua/+/nhiet_do

Kết quả: Sẽ khớp với nha_cua/phong_khach/nhiet_do, nha_cua/phong_ngu/nhiet_do.

Không khớp: nha_cua/tang_1/phong_khach/nhiet_do (vì ở đây có tới 2 cấp độ nằm giữa nha_cua và nhiet_do).

b. Multi-level Wildcard (Dấu thăng #)
Dấu # đại diện cho tất cả các cấp độ còn lại tính từ vị trí của nó. Nó phải luôn nằm ở cuối cùng của chuỗi Topic.

Ví dụ: nha_cua/#

Kết quả: Khớp với mọi thứ bắt đầu bằng nha_cua/, bao gồm cả nha_cua/tang_1/phong_khach/anh_sang, nha_cua/den_san_vuon,...
3. Lời khuyên khi thiết kế hệ thống thực tế
Càng cụ thể càng tốt: Đừng bao giờ Subscribe # trên một Broker công cộng, bạn sẽ bị ngập lụt trong hàng triệu bản tin rác.

Đưa ID thiết bị vào Topic: Ví dụ sensor/v1/device_001/status. Điều này giúp bạn quản lý từng thiết bị riêng lẻ một cách chính xác.

Sử dụng Topic cho lệnh (Command): Một cấu trúc phổ biến là .../status để thiết bị gửi dữ liệu đi và .../set để gửi lệnh điều khiển xuống thiết bị.

Bài 3: QoS (Quality of Service) – Lựa chọn giữa tốc độ và độ tin cậy.

Trong môi trường IoT, các thiết bị thường kết nối qua Wi-Fi yếu, mạng 3G/4G chập chờn hoặc di chuyển liên tục. Nếu một cảm biến gửi dữ liệu báo cháy mà mạng bị lag đúng lúc đó, bản tin có bị mất vĩnh viễn không? Để giải quyết vấn đề này, MQTT định nghĩa 3 cấp độ chuyển phát tin nhắn, gọi là QoS.

1. Ba cấp độ QoS trong MQTT
QoS 0: At most once (Tối đa một lần - "Gửi và quên")
Đây là cấp độ thấp nhất. Broker/Client gửi tin nhắn đi và không quan tâm nó có đến đích hay không. Không có phản hồi (ACK).

Ưu điểm: Nhanh nhất, tốn ít năng lượng và băng thông nhất.

Nhược điểm: Tin nhắn có thể bị mất nếu mạng lỗi.

Ứng dụng: Dữ liệu không quan trọng và gửi liên tục (ví dụ: nhiệt độ gửi 5 giây/lần, mất 1 bản tin cũng không sao).

QoS 1: At least once (Ít nhất một lần - "Chắc chắn đến")
Người gửi sẽ lưu tin nhắn lại cho đến khi nhận được gói tin xác nhận PUBACK từ người nhận. Nếu sau một khoảng thời gian không thấy PUBACK, nó sẽ gửi lại.

Ưu điểm: Đảm bảo tin nhắn sẽ đến đích.

Nhược điểm: Có thể xảy ra hiện tượng trùng lặp tin nhắn (Duplicate). Ví dụ: Tin nhắn đã đến nhưng gói PUBACK bị lạc trên đường về, người gửi sẽ gửi lại tin nhắn đó một lần nữa.

Ứng dụng: Điều khiển thiết bị (ví dụ: lệnh "Bật đèn" cần chắc chắn đến nơi).

QoS 2: Exactly once (Đúng một lần duy nhất)
Đây là cấp độ cao nhất và an toàn nhất. Nó sử dụng một quy trình bắt tay 4 bước (PUBREC, PUBREL, PUBCOMP) để đảm bảo tin nhắn không chỉ đến đích mà còn chỉ đến đúng một lần duy nhất.

Ưu điểm: Loại bỏ hoàn toàn việc mất tin hoặc trùng lặp.

Nhược điểm: Chậm nhất và tốn nhiều tài nguyên hệ thống nhất do phải bắt tay nhiều lần.

Ứng dụng: Các giao dịch cực kỳ quan trọng (ví dụ: thanh toán tiền điện, hệ thống y tế, báo động khẩn cấp).

Lưu ý về QoS trong Publish/Subscribe
Một điểm cực kỳ quan trọng cần nhớ: QoS là thỏa thuận giữa Client và Broker, không phải trực tiếp giữa hai Client.

Khi Publisher gửi tin nhắn đến Broker với QoS 2, tin nhắn đó được đảm bảo nằm an toàn trên Broker.

Nếu Subscriber đăng ký topic đó với QoS 0, Broker sẽ chỉ gửi tin nhắn đó cho Subscriber với QoS 0 (có thể mất).

Kết luận: Độ tin cậy cuối cùng của một bản tin phụ thuộc vào giá trị nhỏ nhất của QoS khi Publish và QoS khi Subscribe.

Bài 4: Retained Messages & LWT (Last Will and Testament) – Xử lý thiết bị "đột tử" và cập nhật trạng thái tức thì.

Trong bài học này, chúng ta sẽ khám phá hai tính năng cực kỳ quan trọng giúp hệ thống MQTT hoạt động thông minh và tin cậy hơn, đặc biệt là khi đối mặt với các vấn đề về kết nối mạng và thiết bị hoạt động không ổn định.

1. Retained Messages (Tin nhắn lưu giữ) – "Biết ngay trạng thái hiện tại"
Hãy tưởng tượng bạn vừa cài đặt một ứng dụng điều khiển nhà thông minh và mở nó lên lần đầu tiên. Làm sao ứng dụng biết được hiện tại đèn đang Bật hay Tắt?

Thông thường, ứng dụng phải đợi đến khi thiết bị gửi (Publish) bản tin trạng thái tiếp theo. Nếu thiết bị chỉ gửi trạng thái mỗi 1 giờ/lần, bạn sẽ phải chờ 1 giờ để thấy dữ liệu.

Retained Message giải quyết việc này. Khi một bản tin được gửi với cờ Retain = true, Broker sẽ ghi nhớ bản tin cuối cùng của Topic đó.

Đặc điểm:

Khi một Subscriber mới đăng ký vào Topic đó, Broker sẽ ngay lập tức gửi bản tin lưu giữ này cho họ.

Mỗi Topic chỉ có duy nhất một Retained Message (bản tin mới nhất sẽ đè lên bản tin cũ).

Ứng dụng: Lưu trạng thái hiện tại của thiết bị (On/Off, nhiệt độ hiện tại), cấu hình thiết bị.

2. LWT (Last Will and Testament - Di chúc) – "Nếu tôi chết, hãy báo cho mọi người"
Trong IoT, một thiết bị có thể bị mất kết nối bất cứ lúc nào (hết pin, mất mạng, cháy nổ). Làm sao các thiết bị khác hoặc ứng dụng người dùng biết được thiết bị đó đã Ngoại tuyến (Offline)?

LWT cho phép Client gửi một "bản di chúc" cho Broker ngay lúc mới kết nối: "Này Broker, nếu tôi bị mất kết nối đột ngột mà không kịp chào tạm biệt, hãy thay mặt tôi Publish bản tin này vào Topic 'trang_thai' nhé!"

Cơ chế hoạt động:

Kết nối: Client kết nối và đăng ký LWT (ví dụ: Topic device/status, nội dung Offline).

Sự cố: Client bị rớt mạng đột ngột (Broker không nhận được gói tin Keep-alive).

Thực thi: Broker nhận thấy Client "mất tích", nó sẽ tự động lấy bản tin Offline và Publish vào topic device/status cho tất cả mọi người cùng biết.

Kết nối lại: Khi thiết bị Online trở lại, nó thường sẽ Publish một bản tin Online (có kèm cờ Retain) để cập nhật lại trạng thái.