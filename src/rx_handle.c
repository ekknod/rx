#include "../include/rx_handle.h"

extern void *malloc(size_t);
extern void free(void *);

typedef struct _rx_handle {
    void (*close)(rx_handle);
} *rx_handle ;

typedef int (*rx_init_fn)(rx_handle, void *);
typedef void (*rx_close_fn)(rx_handle);
rx_handle
rx_initialize_handle(
    _in_     rx_init_fn  on_start,
    _in_     rx_close_fn on_close,
    _in_opt_ void        *start_parameters,
    _in_     size_t      size
    )
{
    rx_handle p;

    p = malloc(sizeof(void*) + size);
    p->close = on_close;
    p++;
    if (on_start(p, start_parameters) < 0) {
        p--;
        free(p);
        return 0;
    }
    return p;
}

void
rx_close_handle(
    _in_  rx_handle  handle
    )
{
    if (handle != 0) {
        handle--;
        handle->close(handle + 1);
        free(handle);
    }
}

