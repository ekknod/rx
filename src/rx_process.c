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
    rx_bool        wow64;
    uintptr_t      map;
} *rx_handle ;

extern struct rtld_global *_rtld_global;
extern int snprintf ( char * s, size_t n, const char * format, ... );
extern void *malloc(size_t);
extern void free(void *);

static rx_bool initialize_process(rx_handle process);

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

static int open_memory(int pid, char *dir, size_t length, int access_mask)
{
    snprintf(dir, length, "/proc/%d/mem", pid);
    return open(dir, access_mask);
}

rx_handle rx_open_process(
    _in_     int               pid,
    _in_     RX_ACCESS_MASK    access_mask
    )
{
    rx_handle a0;

    a0             = malloc(sizeof(struct _rx_handle));
    a0->head.close = close_handle;
    a0->head.self  = a0;
    a0->value[0]   = open_memory(pid, a0->dir, sizeof(a0->dir), access_mask);
    a0->value[1]   = pid;
    if (a0->value[0] == -1 || !initialize_process(a0)) {
        free(a0);
        return 0;
    }
    return a0;
}

rx_bool rx_process_exists(
    _in_      rx_handle        process
    )
{
    return access(process->dir, F_OK ) + 1;
}

rx_bool rx_wow64_process(
    _in_      rx_handle        process
    )
{
    return process->wow64;
}

int rx_process_id(
    _in_      rx_handle        process
    )
{
    return process->value[1];
}

//
// shared private
//
uintptr_t rx_process_map_address(
    _in_      rx_handle        process
    )
{
    return process->map;
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

static rx_bool initialize_process(rx_handle process)
{
    rx_bool          status;
    rx_handle        snap;
    RX_LIBRARY_ENTRY entry;


    status = 0;
    snap   = rx_create_snapshot(RX_SNAP_TYPE_LIBRARY, rx_process_id(process));
    if (!rx_next_library(snap, &entry))
        goto end;

    if (rx_read_process(process, entry.start + 0x12, &process->wow64, sizeof(rx_bool)) == -1)
        goto end;

    if (process->wow64 == 62) process->wow64 = 0; else process->wow64 = 1;
    while (rx_next_library(snap, &entry)) {
        if (strcasecmp(entry.name, "ld") >> 5 == 1) {
            process->map = entry.start + get_lmap_offset();
            status = 1;
            break;
        }
    }
end:
    rx_close_handle(snap);
    return status;
}

