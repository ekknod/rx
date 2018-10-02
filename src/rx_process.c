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

typedef int (*rx_init_fn)(rx_handle, void *);
typedef void (*rx_close_fn)(rx_handle);
rx_handle
rx_initialize_handle(
    _in_     rx_init_fn  on_start,
    _in_     rx_close_fn on_close,
    _in_opt_ void        *start_parameters,
    _in_     size_t      size
    ) ;

typedef struct {
    int            pid;
    RX_ACCESS_MASK access_mask;
} process_parameters ;

struct link_map {
    uintptr_t l_addr;
    char *l_name;
    void *l_ld;
    struct link_map *l_next, *l_prev;
} ;

typedef struct _rx_handle {
    int            value[2];
    char           dir[17];
    rx_bool        wow64;
    uintptr_t      map;
} *rx_handle ;

extern struct rtld_global *_rtld_global;
extern int snprintf ( char * s, size_t n, const char * format, ... );

static int
initialize_process(rx_handle process);

static int
open_process(rx_handle process, process_parameters *parameters)
{
    snprintf(process->dir, sizeof(process->dir), "/proc/%d/mem", parameters->pid);
    process->value[0] = open(process->dir, parameters->access_mask);
    process->value[1] = parameters->pid;
    return initialize_process(process);
}

static void
close_process(rx_handle process)
{
    close(process->value[0]);
}

rx_handle
rx_open_process(
    _in_     int               pid,
    _in_     RX_ACCESS_MASK    access_mask
    )
{
    process_parameters parameters;

    parameters.pid         = pid;
    parameters.access_mask = access_mask;
    return rx_initialize_handle((rx_init_fn)open_process, close_process, &parameters, sizeof(struct _rx_handle));
}

rx_bool
rx_process_exists(
    _in_      rx_handle        process
    )
{
    return access(process->dir, F_OK ) + 1;
}

rx_bool
rx_wow64_process(
    _in_      rx_handle        process
    )
{
    return process->wow64;
}

int
rx_process_id(
    _in_      rx_handle        process
    )
{
    return process->value[1];
}

//
// shared private
//
uintptr_t
rx_process_map_address(
    _in_      rx_handle        process
    )
{
    return process->map;
}

__ssize_t
rx_read_process(
    _in_     rx_handle         process,
    _in_     uintptr_t         address,
    _out_    void              *buffer,
    _in_     size_t            length
    )
{
    return pread(process->value[0], buffer, length, address);
}

__ssize_t
rx_write_process(
    _in_     rx_handle         process,
    _in_     uintptr_t         address,
    _out_    void              *buffer,
    _in_     size_t            length
    )
{
    return pwrite(process->value[0], buffer, length, address);
}

static uintptr_t
get_lmap_offset(void)
{
    struct link_map *map;

    map = (void*)_rtld_global;
    while (map->l_next)
        map = map->l_next;
    return (uintptr_t)_rtld_global - (uintptr_t)map->l_addr;
}

static int
initialize_process(rx_handle process)
{
    int              status;
    rx_handle        snap;
    RX_LIBRARY_ENTRY entry;


    status = -1;
    if (process->value[0] == -1)
        return status;

    snap = rx_create_snapshot(RX_SNAP_TYPE_LIBRARY, rx_process_id(process));
    if (!rx_next_library(snap, &entry))
        goto end;

    if (rx_read_process(process, entry.start + 0x12, &process->wow64, sizeof(rx_bool)) == -1)
        goto end;

    if (process->wow64 == 62) process->wow64 = 0; else process->wow64 = 1;
    while (rx_next_library(snap, &entry)) {
        if (strcasecmp(entry.name, "ld") >> 5 == 1) {
            process->map = entry.start + get_lmap_offset();
            status = 0;
            break;
        }
    }
end:
    rx_close_handle(snap);
    return status;
}

int
rx_find_process_id(
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

