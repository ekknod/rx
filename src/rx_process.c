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

static int
open_process(rx_handle, void *);

static void
close_process(rx_handle);

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

ssize_t
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

ssize_t
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
    rx_next_library(snap, &entry);

    if (rx_read_process(process, entry.start + 0x12, &self->wow64, 1) == -1) {
        close(self->value[0]);
        goto end_snap;
    }

    while (rx_next_library(snap, &entry))
        self->map = entry.start;
    
    if (self->wow64 == 62) {
        self->wow64 = 0;
        status = rx_read_process(process, self->map + 0x60, &self->map, 8) != -1;
    } else {
        self->wow64 = 1;
        status = rx_read_process(process, self->map + 0x40, &self->map, 4) != -1;
    }
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

