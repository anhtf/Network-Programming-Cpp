#include "tcp_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

struct tcp_server
{
    int server_fd;
    int running;

    struct sockaddr_in server_addr;
    socklen_t addr_len;

    tcp_server_config_t config;
    tcp_server_callbacks_t callbacks;

    int client_fds[TCP_SERVER_MAX_CLIENTS];
};

static void tcp_server_clear_clients(tcp_server_t *server)
{
    uint32_t i;
    for (i = 0; i < TCP_SERVER_MAX_CLIENTS; i++)
    {
        server->client_fds[i] = 0;
    }
}

static int tcp_server_add_client(tcp_server_t *server, int client_fd)
{
    uint32_t i;
    uint32_t limit = server->config.max_clients;

    if (limit > TCP_SERVER_MAX_CLIENTS)
    {
        limit = TCP_SERVER_MAX_CLIENTS;
    }

    for (i = 0; i < limit; i++)
    {
        if (server->client_fds[i] == 0)
        {
            server->client_fds[i] = client_fd;
            return 0;
        }
    }

    return -1;
}

static void tcp_server_remove_client(tcp_server_t *server, int client_fd)
{
    uint32_t i;
    uint32_t limit = server->config.max_clients;

    if (limit > TCP_SERVER_MAX_CLIENTS)
    {
        limit = TCP_SERVER_MAX_CLIENTS;
    }

    for (i = 0; i < limit; i++)
    {
        if (server->client_fds[i] == client_fd)
        {
            server->client_fds[i] = 0;
            return;
        }
    }
}

static void tcp_server_handle_disconnect(tcp_server_t *server, int client_fd)
{
    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    char ip_str[INET_ADDRSTRLEN] = {0};
    uint16_t port = 0;

    memset(&peer_addr, 0, sizeof(peer_addr));

    if (getpeername(client_fd, (struct sockaddr *)&peer_addr, &peer_len) == 0)
    {
        inet_ntop(AF_INET, &peer_addr.sin_addr, ip_str, sizeof(ip_str));
        port = ntohs(peer_addr.sin_port);
    }

    if (server->callbacks.on_disconnect != NULL)
    {
        server->callbacks.on_disconnect(server, client_fd, ip_str, port);
    }

    close(client_fd);
    tcp_server_remove_client(server, client_fd);
}

tcp_server_t* tcp_server_create(const tcp_server_config_t *config,
                                const tcp_server_callbacks_t *callbacks)
{
    tcp_server_t *server;
    int opt;

    if (config == NULL)
    {
        return NULL;
    }

    if ((config->max_clients == 0) || (config->max_clients > TCP_SERVER_MAX_CLIENTS))
    {
        return NULL;
    }

    server = (tcp_server_t *)malloc(sizeof(tcp_server_t));
    if (server == NULL)
    {
        return NULL;
    }

    memset(server, 0, sizeof(tcp_server_t));

    server->config = *config;
    if (callbacks != NULL)
    {
        server->callbacks = *callbacks;
    }
    else
    {
        memset(&server->callbacks, 0, sizeof(server->callbacks));
    }

    server->addr_len = sizeof(server->server_addr);
    tcp_server_clear_clients(server);

    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd < 0)
    {
        free(server);
        return NULL;
    }

    opt = (server->config.reuse_addr != 0) ? 1 : 0;
    if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(server->server_fd);
        free(server);
        return NULL;
    }

    memset(&server->server_addr, 0, sizeof(server->server_addr));
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(server->config.port);

    if (bind(server->server_fd,
             (struct sockaddr *)&server->server_addr,
             sizeof(server->server_addr)) < 0)
    {
        close(server->server_fd);
        free(server);
        return NULL;
    }

    if (listen(server->server_fd, (int)server->config.max_clients) < 0)
    {
        close(server->server_fd);
        free(server);
        return NULL;
    }

    server->running = 0;
    return server;
}

void tcp_server_destroy(tcp_server_t *server)
{
    uint32_t i;
    uint32_t limit;

    if (server == NULL)
    {
        return;
    }

    limit = server->config.max_clients;
    if (limit > TCP_SERVER_MAX_CLIENTS)
    {
        limit = TCP_SERVER_MAX_CLIENTS;
    }

    for (i = 0; i < limit; i++)
    {
        if (server->client_fds[i] > 0)
        {
            close(server->client_fds[i]);
            server->client_fds[i] = 0;
        }
    }

    if (server->server_fd > 0)
    {
        close(server->server_fd);
        server->server_fd = -1;
    }

    free(server);
}

void tcp_server_stop(tcp_server_t *server)
{
    if (server == NULL)
    {
        return;
    }

    server->running = 0;
}

int tcp_server_send(tcp_server_t *server, int client_fd, const void *data, int len)
{
    (void)server;

    if ((client_fd <= 0) || (data == NULL) || (len <= 0))
    {
        return -1;
    }

    return (int)send(client_fd, data, (size_t)len, 0);
}

int tcp_server_broadcast(tcp_server_t *server, const void *data, int len, int exclude_fd)
{
    uint32_t i;
    uint32_t limit;
    int success_count = 0;

    if ((server == NULL) || (data == NULL) || (len <= 0))
    {
        return -1;
    }

    limit = server->config.max_clients;
    if (limit > TCP_SERVER_MAX_CLIENTS)
    {
        limit = TCP_SERVER_MAX_CLIENTS;
    }

    for (i = 0; i < limit; i++)
    {
        int fd = server->client_fds[i];
        if ((fd > 0) && (fd != exclude_fd))
        {
            if (send(fd, data, (size_t)len, 0) >= 0)
            {
                success_count++;
            }
        }
    }

    return success_count;
}

uint16_t tcp_server_get_port(const tcp_server_t *server)
{
    if (server == NULL)
    {
        return 0;
    }

    return server->config.port;
}

int tcp_server_start(tcp_server_t *server)
{
    fd_set readfds;
    int max_sd;
    int activity;
    uint32_t i;
    uint32_t limit;
    char recv_buf[TCP_SERVER_RECV_BUF_SIZE + 1];

    if (server == NULL)
    {
        return -1;
    }

    server->running = 1;
    limit = server->config.max_clients;
    if (limit > TCP_SERVER_MAX_CLIENTS)
    {
        limit = TCP_SERVER_MAX_CLIENTS;
    }

    printf("TCP server listening on port %u\n", server->config.port);

    while (server->running)
    {
        FD_ZERO(&readfds);
        FD_SET(server->server_fd, &readfds);
        max_sd = server->server_fd;

        for (i = 0; i < limit; i++)
        {
            int fd = server->client_fds[i];
            if (fd > 0)
            {
                FD_SET(fd, &readfds);
                if (fd > max_sd)
                {
                    max_sd = fd;
                }
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror("select");
            return -1;
        }

        if (FD_ISSET(server->server_fd, &readfds))
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_fd;
            char ip_str[INET_ADDRSTRLEN] = {0};
            uint16_t client_port;

            memset(&client_addr, 0, sizeof(client_addr));

            new_fd = accept(server->server_fd,
                            (struct sockaddr *)&client_addr,
                            &client_len);
            if (new_fd < 0)
            {
                perror("accept");
            }
            else
            {
                inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
                client_port = ntohs(client_addr.sin_port);

                if (tcp_server_add_client(server, new_fd) == 0)
                {
                    if (server->callbacks.on_connect != NULL)
                    {
                        server->callbacks.on_connect(server, new_fd, ip_str, client_port);
                    }
                }
                else
                {
                    printf("Server full, reject client %s:%u\n", ip_str, client_port);
                    close(new_fd);
                }
            }
        }

        for (i = 0; i < limit; i++)
        {
            int fd = server->client_fds[i];
            if ((fd > 0) && FD_ISSET(fd, &readfds))
            {
                int valread;

                memset(recv_buf, 0, sizeof(recv_buf));
                valread = (int)read(fd, recv_buf, TCP_SERVER_RECV_BUF_SIZE);

                if (valread <= 0)
                {
                    tcp_server_handle_disconnect(server, fd);
                }
                else
                {
                    recv_buf[valread] = '\0';

                    if (server->callbacks.on_data != NULL)
                    {
                        server->callbacks.on_data(server, fd, recv_buf, valread);
                    }
                }
            }
        }
    }

    return 0;
}