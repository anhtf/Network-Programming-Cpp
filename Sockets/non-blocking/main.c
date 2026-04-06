#include <stdio.h>
#include <string.h>
#include "tcp_server.h"

static void app_on_connect(tcp_server_t *server,
                           int client_fd,
                           const char *client_ip,
                           uint16_t client_port)
{
    const char *msg = "Welcome to TCP server!\n";

    printf("[CONNECT] fd=%d ip=%s port=%u\n", client_fd, client_ip, client_port);
    tcp_server_send(server, client_fd, msg, (int)strlen(msg));
}

static void app_on_disconnect(tcp_server_t *server,
                              int client_fd,
                              const char *client_ip,
                              uint16_t client_port)
{
    (void)server;
    printf("[DISCONNECT] fd=%d ip=%s port=%u\n", client_fd, client_ip, client_port);
}

static void app_on_data(tcp_server_t *server,
                        int client_fd,
                        const char *data,
                        int data_len)
{
    char msg[1200];

    printf("[DATA] fd=%d len=%d data=%.*s\n", client_fd, data_len, data_len, data);

    /* Echo lại chính client gửi */
    tcp_server_send(server, client_fd, data, data_len);

    /* Broadcast cho các client khác */
    snprintf(msg, sizeof(msg), "[Client %d] %.*s", client_fd, data_len, data);
    tcp_server_broadcast(server, msg, (int)strlen(msg), client_fd);
}

int main(void)
{
    tcp_server_t *server;
    tcp_server_config_t config;
    tcp_server_callbacks_t callbacks;

    config.port = 8080;
    config.max_clients = 30;
    config.reuse_addr = 1;

    callbacks.on_connect = app_on_connect;
    callbacks.on_disconnect = app_on_disconnect;
    callbacks.on_data = app_on_data;

    server = tcp_server_create(&config, &callbacks);
    if (server == NULL)
    {
        printf("Create server failed\n");
        return -1;
    }

    tcp_server_start(server);
    tcp_server_destroy(server);

    return 0;
}