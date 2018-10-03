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

typedef struct _rx_process {
    int            value[2];
    char           dir[17];
    rx_bool        wow64;
    uintptr_t      map;
} *rx_process ;

extern struct rtld_global *_rtld_global;
extern int snprintf ( char * s, size_t n, const char * format, ... );

static int
initialize_process(rx_handle process);

static int
open_process(rx_handle process, void *parameters)
{
    rx_process self            = process;
    process_parameters *params = parameters;
    snprintf(self->dir, sizeof(self->dir), "/proc/%d/mem", params->pid);
    self->value[0] = open(self->dir, params->access_mask);
    self->value[1] = params->pid;
    if (self->value[0] != -1) {
        return initialize_process(process);
    }
    return -1;
}

static void
close_process(rx_handle process)
{
    rx_process self = process;
    close(self->value[0]);
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
    return rx_initialize_handle(open_process, close_process, &parameters, sizeof(struct _rx_process));
}

rx_bool
rx_process_exists(
    _in_      rx_handle        process
    )
{
    rx_process self = process;
    return access(self->dir, F_OK ) + 1;
}

rx_bool
rx_wow64_process(
    _in_      rx_handle        process
    )
{
    rx_process self = process;
    return self->wow64;
}

int
rx_process_id(
    _in_      rx_handle        process
    )
{
    rx_process self = process;
    return self->value[1];
}

/* shared private */
uintptr_t
rx_process_map_address(
    _in_      rx_handle        process
    )
{
    rx_process self = process;
    return self->map;
}


extern ssize_t pread (int __fd, void *__buf, size_t __nbytes, __off_t __offset);
__ssize_t
rx_read_process(
    _in_     rx_handle         process,
    _in_     uintptr_t         address,
    _out_    void              *buffer,
    _in_     size_t            length
    )
{
    rx_process self = process;
    return pread(self->value[0], buffer, length, address);
}


extern ssize_t pwrite (int __fd, const void *__buf, size_t __n, __off_t __offset);
__ssize_t
rx_write_process(
    _in_     rx_handle         process,
    _in_     uintptr_t         address,
    _out_    void              *buffer,
    _in_     size_t            length
    )
{
    rx_process self = process;
    return pwrite(self->value[0], buffer, length, address);
}

static uintptr_t
get_lmap_offset(void)
{
    struct link_map *map = (struct link_map *)_rtld_global;
    while (map->l_next)
        map = map->l_next;
    return (uintptr_t)_rtld_global - (uintptr_t)map->l_addr;
}

static int cmp(const char *s, const char *c)
{
    while (*s && *s == *c) s++, c++ ;
    return *s - *c;
}

static int
initialize_process(rx_handle process)
{
    rx_process       self   = process;
    int              status = -1;
    RX_LIBRARY_ENTRY entry;

    rx_handle snap = rx_create_snapshot(RX_SNAP_TYPE_LIBRARY, rx_process_id(process));
    if (!rx_next_library(snap, &entry))
        goto end;
    
    if (rx_read_process(process, entry.start + 0x12, &self->wow64, sizeof(rx_bool)) == -1)
        goto end;

    if (self->wow64 == 62) self->wow64 = 0; else self->wow64 = 1;
    while (rx_next_library(snap, &entry)) {
        if (cmp(entry.name, "ld") >> 5 == 1) {
            self->map = entry.start + get_lmap_offset();
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
    RX_PROCESS_ENTRY e;
    int       p = 0;
    rx_handle s = rx_create_snapshot(RX_SNAP_TYPE_PROCESS, 0);
    while (rx_next_process(s, &e)) {
        if (strcmp(e.name, process_name) == 0) {
            p = e.pid;
            break;
        }
    }
    rx_close_handle(s);
    return p;
}

