/*
 * =====================================================================================
 *
 *       Filename:  rx_list.c
 *
 *    Description:  process / process library listing
 *
 *        Version:  1.0
 *        Created:  27.09.2018 16:44:16
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  github.com/ekknod 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "../include/rx_list.h"
#include <dirent.h>
#include <string.h>
#include <unistd.h>

struct list_parameters {
    RX_SNAP_TYPE type;
    int          pid;
} ;

struct rx_list {
    char value[12];
} ;

#define PID_MAX 32768
#define CDIR(snapshot) *(DIR**)&snapshot->value
#define CFILE(snapshot) ((int*)&snapshot->value)[0]
#define CPID(snapshot) ((int*)&snapshot->value)[1]
#define CTYPE(snapshot) ((int*)&snapshot->value)[2]

static int
snap_create(rx_handle snapshot, void *parameters);

static void
snap_destroy(rx_handle);

rx_handle
rx_create_snapshot(
    _in_     RX_SNAP_TYPE      type,
    _in_opt_ int               pid
    )
{
    struct list_parameters parameters;

    parameters.type = type;
    parameters.pid  = pid;
    return rx_initialize_object(snap_create, snap_destroy, &parameters, sizeof(struct rx_list));
}

extern int snprintf ( char * s, size_t n, const char * format, ... );
extern int atoi(const char *nptr);
extern ssize_t readlink(const char *path, char *buf, size_t bufsiz); 

rx_bool
rx_next_process(
    _in_     rx_handle         snapshot,
    _out_    PRX_PROCESS_ENTRY entry
    )
{
    struct rx_list *self = snapshot;
    struct dirent  *a0;
    ssize_t        a1;

    while ((a0 = readdir(CDIR(self)))) {
        if (a0->d_type != 4)
            continue;
        
        snprintf(entry->exe, sizeof(entry->exe), "/proc/%s/exe", a0->d_name);
        if ((a1 = readlink(entry->exe, entry->data, sizeof(entry->data))) == -1)
            continue;

        entry->data[a1] = '\0', entry->path = entry->data;
        if ((entry->name = strrchr(entry->path, '/') + 1) == (const char *)1)
            continue;

        entry->pid = atoi(a0->d_name);
        if (entry->pid < 1 || entry->pid > PID_MAX)
            continue;

        return 1;
    }
    return 0;
}

static size_t
read_line(struct rx_list *snapshot, char *buffer, size_t length)
{
    size_t pos = 0;

    while (--length > 0 && read(CFILE(snapshot), &buffer[pos], 1)) {
        if (buffer[pos] == '\n') {
            buffer[pos] = '\0';
            return pos;
        }
        pos++;
    }
    return 0;
}

extern unsigned long int strtoul (const char* str, char** endptr, int base);

rx_bool
rx_next_library(
    _in_     rx_handle         snapshot,
    _out_    PRX_LIBRARY_ENTRY entry
    )
{
    struct rx_list *self = snapshot;
    char *a0;

    while (read_line(self, entry->data, sizeof(entry->data))) {
        entry->path  = strchr(entry->data, '/');
        entry->name  = strrchr(entry->data, '/');
        a0           = entry->data;
        entry->start = strtoul(entry->data, (char**)&a0, 16);
        entry->end   = strtoul(a0+1, (char**)&a0, 16);
        entry->pid   = CPID(self);
        if (entry->path == 0 || entry->name++ == 0 || entry->end < entry->start)
            continue;
        return 1;
    }
    return 0;
}

extern int open(const char *path, int oflag, ... );

static int
snap_create(rx_handle snapshot, void *parameters)
{
    struct rx_list         *self   = snapshot;
    struct list_parameters *params = parameters;
    char a0[17];

    CTYPE(self) = params->type;
    switch (CTYPE(self)) {
        case RX_SNAP_TYPE_PROCESS:
            if ((CDIR(self) = opendir("/proc/")) == 0)
                return -1;
            break;
        case RX_SNAP_TYPE_LIBRARY:
            snprintf(a0, sizeof(a0), "/proc/%d/maps", params->pid);
            CFILE(self) = open(a0, RX_READ_ACCESS);
            CPID(self)  = params->pid;
            if (CFILE(self) == -1)
                return -1;
            break;
        default:
            return -1;
    }
    return 0;
}

static void
snap_destroy(rx_handle snapshot)
{
    struct rx_list *self = snapshot;
    switch (CTYPE(self)) {
        case RX_SNAP_TYPE_PROCESS:
            closedir(CDIR(self));
            break;
        case RX_SNAP_TYPE_LIBRARY:
            close(CFILE(self));
            break;
        default:
            break;
    }
}

