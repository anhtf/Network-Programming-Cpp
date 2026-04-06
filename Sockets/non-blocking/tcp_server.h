#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdint.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TCP_SERVER_MAX_CLIENTS   100
#define TCP_SERVER_RECV_BUF_SIZE 1024

typedef struct tcp_server tcp_server_t;

/* =========================
 * Callback definitions
 * ========================= */
typedef void (*tcp_server_on_connect_cb)(tcp_server_t *server,
                                         int client_fd,
                                         const char *client_ip,
                                         uint16_t client_port);

typedef void (*tcp_server_on_disconnect_cb)(tcp_server_t *server,
                                            int client_fd,
                                            const char *client_ip,
                                            uint16_t client_port);

typedef void (*tcp_server_on_data_cb)(tcp_server_t *server,
                                      int client_fd,
                                      const char *data,
                                      int data_len);

/* =========================
 * Config structure
 * ========================= */
typedef struct
{
    uint16_t port;
    uint32_t max_clients;
    int reuse_addr;
} tcp_server_config_t;

/* =========================
 * Callback bundle
 * ========================= */
typedef struct
{
    tcp_server_on_connect_cb    on_connect;
    tcp_server_on_disconnect_cb on_disconnect;
    tcp_server_on_data_cb       on_data;
} tcp_server_callbacks_t;

/* =========================
 * Public API
 * ========================= */
tcp_server_t* tcp_server_create(const tcp_server_config_t *config,
                                const tcp_server_callbacks_t *callbacks);

void tcp_server_destroy(tcp_server_t *server);

int tcp_server_start(tcp_server_t *server);

void tcp_server_stop(tcp_server_t *server);

int tcp_server_send(tcp_server_t *server, int client_fd, const void *data, int len);

int tcp_server_broadcast(tcp_server_t *server, const void *data, int len, int exclude_fd);

/* Optional helper */
uint16_t tcp_server_get_port(const tcp_server_t *server);

#ifdef __cplusplus
}
#endif

#endif /* TCP_SERVER_H */