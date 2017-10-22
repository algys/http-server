#pragma once
#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct data_t
{
    char * ptr;
    uint32_t size;
    uint32_t cap;
    uint32_t cur;
} data_t;

typedef struct conn_t
{
    int fd;
    data_t inp_data;
    data_t out_data;
    void * udata;
    int need_destroy;
    int need_read;
    uint64_t start_time;
    uint64_t finish_time;
} conn_t;

typedef void (*on_accept_cb_t)(uint32_t id, data_t * inp_data, data_t * out_data, void ** udata);
typedef void (*on_destroy_cb_t)(void * udata);
typedef void (*on_read_cb_t)(void * udata);

enum {
    DO_AGAIN    = 1,
    END         = 2,
    ERROR       = 3,
};

void write_to_data(data_t * dest, char const * src, uint32_t len);

void init(on_accept_cb_t, on_read_cb_t, on_destroy_cb_t);
void need_destroy(uint32_t conn_idx);
void set_need_read(uint32_t conn_idx, int val);