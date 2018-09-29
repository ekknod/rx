/*
 * =====================================================================================
 *
 *       Filename:  rx_process.c
 *
 *    Description:  remote process memory operations
 *
 *        Version:  1.0
 *        Created:  27.09.2018 17:32:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  github.com/ekknod 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "../include/rx_process.h"
#include "../include/rx_handle.h"
#include "../include/rx_list.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


struct link_map {
    uintptr_t l_addr;
    char *l_name;
    void *l_ld;
    struct link_map *l_next, *l_prev;
} ;

typedef struct _rx_handle {
    rx_handle_head head;
    int            value[2];
    char           dir[17];
    uintptr_t      map;
} *rx_handle ;

extern struct rtld_global *_rtld_global;
extern int snprintf ( char * s, size_t n, const char * format, ... );
extern void *malloc(size_t);
extern void free(void *);
static uintptr_t get_lmap_address(int pid);

int rx_find_process_id(
    _in_     const char*       process_name
    )
{
    int              p;
    rx_handle        s;
    RX_PROCESS_ENTRY e;

    p = 0;
    s = rx_create_snapshot(RX_SNAP_TYPE_PROCESS, 0);
    while (rx_next_process(s, &e)) {
        if (strcmp(e.name, process_name) == 0) {
            p = e.pid;
            break;
        }
    }
    rx_close_handle(s);
    return p;
}

static void close_handle(rx_handle process)
{
    close(process->value[0]);
    free(process);
}

rx_handle rx_open_process(
    _in_     int               pid,
    _in_     RX_ACCESS_MASK    access_mask
    )
{
    rx_handle a0;

    a0 = (rx_handle)malloc(sizeof(struct _rx_handle));
    a0->head.close = close_handle;
    a0->head.self  = a0;
    snprintf(a0->dir, sizeof(a0->dir), "/proc/%d/mem", pid);
    a0->value[0]  = open(a0->dir, access_mask);
    a0->value[1]  = pid;
    if (a0->value[0] == -1) {
        free(a0);
        return 0;
    }
    a0->map = get_lmap_address(pid);
    return (rx_handle)a0;
}

rx_bool rx_process_exists(
    _in_      rx_handle        process
    )
{
    return access(process->dir, F_OK ) + 1;
}


int rx_process_id(
    _in_      rx_handle        process
    )
{
    return process->value[1];
}

__ssize_t rx_read_process(
    _in_     rx_handle         process,
    _in_     uintptr_t         address,
    _out_    void              *buffer,
    _in_     size_t            length
    )
{
    return pread(process->value[0], buffer, length, address);
}

__ssize_t rx_write_process(
    _in_     rx_handle         process,
    _in_     uintptr_t         address,
    _out_    void              *buffer,
    _in_     size_t            length
    )
{
    return pwrite(process->value[0], buffer, length, address);
}

static uintptr_t get_lmap_offset(void)
{
    struct link_map *map;

    map = (void*)_rtld_global;
    while (map->l_next)
        map = map->l_next;
    return (uintptr_t)_rtld_global - (uintptr_t)map->l_addr;
}

static uintptr_t get_lmap_address(int pid)
{
    rx_handle        s;
    RX_LIBRARY_ENTRY e;

    s = rx_create_snapshot(RX_SNAP_TYPE_LIBRARY, pid);
    while (rx_next_library(s, &e) && strcasecmp(e.name, "ld") >> 5 != 1);
    rx_close_handle(s);
    return e.start + get_lmap_offset();
}

