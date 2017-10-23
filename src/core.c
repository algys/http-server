#include "core.h"
#include <sys/epoll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>

#define MAX_CONNS 10000
#define MAX_EVENTS 100
#define BUF_SIZE 1024

static int epoll_fd = -1;
static int listen_sock = -1;
static int listen_port = 8080;

static conn_t *conns = NULL;
static uint32_t conns_count = 0;
static uint32_t conns_total = 0;

static struct epoll_event *events = NULL;

static on_accept_cb_t on_accept_cb = NULL;
static on_read_cb_t on_read_cb = NULL;
static on_destroy_cb_t on_destroy_cb = NULL;

static FILE * log_file = NULL;

uint64_t now() {
    struct timeval te;
    gettimeofday(&te, NULL);
    uint64_t milliseconds = (uint64_t)te.tv_sec*1000 + te.tv_usec/1000;
    return milliseconds;
}

static int realloc_data(data_t *data) {
    if (!(data->ptr = (char *) realloc(data->ptr, data->cap * 2)))
        return -1;
    data->cap *= 2;

    return 0;
}

void write_to_data(data_t * dest, char const * src, uint32_t len) {
    if (dest->cap <= dest->size + len)
        realloc_data(dest);

    strncpy(dest->ptr + dest->size, src, len);
    dest->size += len;
}

static void _init() {
    conns = (conn_t *) malloc(sizeof(conn_t) * MAX_CONNS);

    events = (struct epoll_event *) malloc(sizeof(struct epoll_event) * MAX_EVENTS * 10);
}

void init(int port, on_accept_cb_t accept_cb, on_read_cb_t read_cb, on_destroy_cb_t destroy_cb) {
    listen_port = port;
    on_accept_cb = accept_cb;
    on_read_cb = read_cb;
    on_destroy_cb = destroy_cb;

    _init();
}

static int start_epoll() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
        return -1;

    return 0;
}

static int start_listen() {
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0)
        return -1;

    fcntl(listen_sock, F_SETFL, O_NONBLOCK);

    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0)
        return -1;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listen_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        return -1;

    if (listen(listen_sock, 1) < 0)
        return -1;
}

void need_destroy(uint32_t conn_idx)
{
    conns[conn_idx].need_destroy = 1;
}

static int delete_conn(uint32_t conn_idx) {
    conns[conn_idx].inp_data.size = 0;
    conns[conn_idx].inp_data.cur = 0;

    conns[conn_idx].out_data.size = 0;
    conns[conn_idx].out_data.cur = 0;
    conns[conn_idx].udata = NULL;
}

static int create_conn(int sock_fd) {
    if (conns_count >= MAX_CONNS)
        return -1;

    if (!conns[sock_fd].inp_data.cap) {
        if (!(conns[sock_fd].inp_data.ptr = (char *) malloc(BUF_SIZE)))
            return -1;
        conns[sock_fd].inp_data.cap = BUF_SIZE;
    }

    if (!conns[sock_fd].out_data.cap) {
        if (!(conns[sock_fd].out_data.ptr = (char *) malloc(BUF_SIZE)))
            return -1;
        conns[sock_fd].out_data.cap = BUF_SIZE;
    }

    conns[sock_fd].fd = sock_fd;
    conns[sock_fd].need_destroy = 0;
    conns[sock_fd].need_read = 0;

    return sock_fd;
}

static int add_epoll_ev(uint32_t conn_idx, uint32_t events) {
    static struct epoll_event ev;

    ev.events = events;
    ev.data.fd = conns[conn_idx].fd;
    ev.data.u32 = conn_idx;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conns[conn_idx].fd, &ev) == -1)
        return -1;

    return 0;
}

static int mod_epoll_ev(uint32_t conn_idx, uint32_t events) {
    static struct epoll_event ev;

    ev.events = events;
    ev.data.fd = conns[conn_idx].fd;
    ev.data.u32 = conn_idx;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conns[conn_idx].fd, &ev) == -1)
        return -1;

    return 0;
}

static int del_epoll_ev(uint32_t conn_idx, uint32_t events) {
    static struct epoll_event ev;

    ev.events = events;
    ev.data.fd = conns[conn_idx].fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conns[conn_idx].fd, &ev) == -1)
        return -1;

    return 0;
}

static void conn_destroy(uint32_t conn_idx) {
    del_epoll_ev(conn_idx, EPOLLOUT | EPOLLIN);
    (*on_destroy_cb)(conns[conn_idx].udata);
    close(conns[conn_idx].fd);
    conns[conn_idx].finish_time = now();
    uint64_t dx = conns[conn_idx].finish_time - conns[conn_idx].start_time;
    delete_conn(conn_idx);
    conns_total++;
    printf("destroyed %d %d dx = %ld\n", conn_idx, conns_total, dx);
    conns_count--;
}

static int accept_conn() {
    int sock_fd = accept(listen_sock, NULL, NULL);
    if (sock_fd < 0)
        return -1;

    fcntl(sock_fd, F_SETFL, O_NONBLOCK);

    int conn_idx = create_conn(sock_fd);
    if (conn_idx < 0)
        return -1;

    conns_count++;
    printf("accepted %d\n", conn_idx);
    conns[sock_fd].start_time = now();

    (*on_accept_cb)(conn_idx, &conns[conn_idx].inp_data, &conns[conn_idx].out_data, &conns[conn_idx].udata);
    if (conns[conn_idx].need_destroy) {
        conn_destroy(conn_idx);
        return -1;
    }

    return conn_idx;
}

static int read_from_conn(uint32_t conn_idx) {
    static char buffer[BUF_SIZE];
    static int bytes;

    while ((bytes = read(conns[conn_idx].fd, buffer, BUF_SIZE))) {
        if (bytes == -1) {
            if (errno == EAGAIN)
                return DO_AGAIN;
            else
                return ERROR;
        }

        if (conns[conn_idx].inp_data.size + bytes >= conns[conn_idx].inp_data.cap)
            realloc_data(&conns[conn_idx].inp_data);

        memcpy(conns[conn_idx].inp_data.ptr + conns[conn_idx].inp_data.size, buffer, bytes);
        conns[conn_idx].inp_data.size += bytes;
    }

    return DO_AGAIN;
}

static int write_to_conn(uint32_t conn_idx) {
    static char *start;
    static int bytes, left;

    start = conns[conn_idx].out_data.ptr + conns[conn_idx].out_data.cur;
    left = conns[conn_idx].out_data.size - conns[conn_idx].out_data.cur;
    while (left > 0) {
        bytes = write(conns[conn_idx].fd, start, left);

        if (bytes == -1) {
            if (errno == EAGAIN)
                return DO_AGAIN;
            else
                return ERROR;
        }

        start += bytes;
        left -= bytes;
        conns[conn_idx].out_data.cur += bytes;
    }

    return END;
}

void set_need_read(uint32_t conn_idx, int val)
{
    conns[conn_idx].need_read = val;
}

int loop() {
    if (start_listen())
        return -1;

    if (start_epoll())
        return -1;

    static struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
        return -1;

    int count = 1;
    int need_wait = 0;
    int enabled = 1;
    for (; enabled;) {
        count = epoll_wait(epoll_fd, events, MAX_EVENTS, need_wait ? -1 : 0);

        for (int i = 0; i < count; ++i) {
            if (events[i].data.fd == listen_sock) {
                int conn_idx = accept_conn();
                if (conn_idx >= 0) {
                    add_epoll_ev(conn_idx, EPOLLIN );
                    set_need_read(conn_idx, 1);
                }
                continue;
            }

            if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                conn_destroy(events[i].data.u32);
                continue;
            }

            if ((events[i].events & EPOLLIN)) {
                switch ((read_from_conn(events[i].data.u32))) {
                    case DO_AGAIN:
                        (*on_read_cb)(conns[events[i].data.u32].udata);

                        if (!conns[events[i].data.u32].need_read)
                            mod_epoll_ev(events[i].data.u32, EPOLLOUT);

                        break;
                    case ERROR:
                        conn_destroy(events[i].data.u32);
                        break;
                }
            }

            if ((events[i].events & EPOLLOUT)) {
                switch ((write_to_conn(events[i].data.u32))) {
                    case ERROR:
                        conn_destroy(events[i].data.u32);
                        break;
                    case END:
                        conn_destroy(events[i].data.u32);
                        break;
                }
            }
        }
    }

    return 0;
}

int main(int argc, char ** argv) {
    extern void app_init(int, char **);

    app_init(argc, argv);

    loop();
}