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
#include "../include/rx_list.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


extern struct  rtld_global *_rtld_global;
extern ssize_t pread (int __fd, void *__buf, size_t __nbytes, __off_t __offset);
extern ssize_t pwrite (int __fd, const void *__buf, size_t __n, __off_t __offset);
extern int     snprintf ( char * s, size_t n, const char * format, ... );

static int
cmp(const char *s, const char *c);

static int
open_process(rx_handle, void *);

static void
close_process(rx_handle);

struct link_map {
    uintptr_t l_addr;
    char *l_name;
    void *l_ld;
    struct link_map *l_next, *l_prev;
} ;

struct process_parameters {
    int            pid;
    RX_ACCESS_MASK access_mask;
} ;

struct rx_process {
    int            value[2];
    char           dir[17];
    rx_bool        wow64;
    uintptr_t      map;
} ;

rx_handle
rx_open_process(
    _in_     int               pid,
    _in_     RX_ACCESS_MASK    access_mask
    )
{
    struct process_parameters parameters;

    parameters.pid         = pid;
    parameters.access_mask = access_mask;
    return rx_initialize_object(open_process, close_process, &parameters, sizeof(struct rx_process));
}

rx_bool
rx_process_exists(
    _in_      rx_handle        process
    )
{
    struct rx_process *self = process;
    return access(self->dir, F_OK ) + 1;
}

rx_bool
rx_wow64_process(
    _in_      rx_handle        process
    )
{
    struct rx_process *self = process;
    return self->wow64;
}

int
rx_process_id(
    _in_      rx_handle        process
    )
{
    struct rx_process *self = process;
    return self->value[1];
}

uintptr_t
rx_process_map_address(
    _in_      rx_handle        process
    )
{
    struct rx_process *self = process;
    return self->map;
}

__ssize_t
rx_read_process(
    _in_     rx_handle         process,
    _in_     uintptr_t         address,
    _out_    void              *buffer,
    _in_     size_t            length
    )
{
    struct rx_process *self = process;
    return pread(self->value[0], buffer, length, address);
}

__ssize_t
rx_write_process(
    _in_     rx_handle         process,
    _in_     uintptr_t         address,
    _out_    void              *buffer,
    _in_     size_t            length
    )
{
    struct rx_process *self = process;
    return pwrite(self->value[0], buffer, length, address);
}

int
rx_find_process_id(
    _in_     const char*       process_name
    )
{
    int              p = -1;
    rx_handle        s = rx_create_snapshot(RX_SNAP_TYPE_PROCESS, 0);
    RX_PROCESS_ENTRY e;

    while (rx_next_process(s, &e)) {
        if (cmp(e.name, process_name) == 0) {
            p = e.pid;
            break;
        }
    }
    rx_close_handle(s);
    return p;
}

static int
cmp(const char *s, const char *c)
{
    while (*s && *s == *c) s++, c++ ;
    return *s - *c;
}

static uintptr_t
get_lmap_offset(void)
{
    struct link_map *map = (struct link_map *)_rtld_global;
    while (map->l_next) map = map->l_next;
    return (uintptr_t)_rtld_global - map->l_addr;
}

static int
open_process(rx_handle process, void *parameters)
{
    int                       status  = -1;
    struct rx_process         *self   = process;
    struct process_parameters *params = parameters;
    rx_handle                 snap;
    RX_LIBRARY_ENTRY          entry;

    snprintf(self->dir, sizeof(self->dir), "/proc/%d/mem", params->pid);
    self->value[0] = open(self->dir, params->access_mask);
    self->value[1] = params->pid;
    self->wow64    = 0;

    if (self->value[0] == status)
        goto end;

    snap = rx_create_snapshot(RX_SNAP_TYPE_LIBRARY, rx_process_id(process));
    if (!rx_next_library(snap, &entry))
        goto end_all;
    
    if (rx_read_process(process, entry.start + 0x12, &self->wow64, 1) == -1)
        goto end_all;

    if (self->wow64 == 62)
        self->wow64 = 0;
    else if (self->wow64 == 3)
        self->wow64 = 1;
    else
        goto end_all;

    while (rx_next_library(snap, &entry)) {
        if (cmp(entry.name, "ld") >> 5 == 1) {
            self->map = entry.start + get_lmap_offset();
            status    = 0;
            goto end_snap;
        }
    }
end_all:
    close(self->value[0]);
end_snap:
    rx_close_handle(snap);
end:
    return status;
}

static void
close_process(rx_handle process)
{
    struct rx_process *self = process;
    close(self->value[0]);
}

