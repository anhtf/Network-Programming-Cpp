#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "ThietBiNhung_Sub_01"
#define TOPIC       "home/room/temp"

// --- Hàm Callback: Tự động được gọi khi mất kết nối ---
void connectionLost(void *context, char *cause) {
    printf("\n[Canh bao] Mat ket noi voi Broker! Nguyen nhan: %s\n", cause);
}

// --- Hàm Callback: TỰ ĐỘNG ĐƯỢC GỌI KHI CÓ TIN NHẮN MỚI ---
int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    printf("\n[Callback] Nhan duoc tin nhan tu Topic: %s\n", topicName);
    printf("[Callback] Noi dung: %.*s\n", message->payloadlen, (char*)message->payload);
    
    // Bắt buộc phải giải phóng bộ nhớ của Paho sau khi đọc xong
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1; // Trả về 1 báo hiệu đã nhận thành công
}

int main() {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // Đăng ký các hàm Callback cho Paho
    MQTTClient_setCallbacks(client, NULL, connectionLost, messageArrived, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
        printf("Ket noi that bai!\n");
        exit(EXIT_FAILURE);
    }
    printf("Da ket noi! Dang cho du lieu...\n");

    // Đăng ký lắng nghe Topic
    MQTTClient_subscribe(client, TOPIC, 1);

    // Vòng lặp chính giả lập thiết bị đang làm việc khác (Chớp LED)
    // Nó không hề bị block bởi việc chờ tin nhắn!
    while(1) {
        printf("."); 
        fflush(stdout);
        sleep(1); 
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return 0;
}