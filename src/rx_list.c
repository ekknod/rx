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
#include "../include/rx_handle.h"
#include <dirent.h>
#include <string.h>
#include <unistd.h>

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
    RX_SNAP_TYPE type;
    int          pid;
} list_parameters ;

typedef struct _rx_handle {
    char value[12];
} *rx_handle ;

#define PID_MAX 32768
#define CDIR(snapshot) *(DIR**)&snapshot->value
#define CFILE(snapshot) ((int*)&snapshot->value)[0]
#define CPID(snapshot) ((int*)&snapshot->value)[1]
#define CTYPE(snapshot) ((int*)&snapshot->value)[2]

static int
snap_create(rx_handle snapshot, list_parameters *parameters);

static void
snap_destroy(rx_handle);

rx_handle
rx_create_snapshot(
    _in_     RX_SNAP_TYPE      type,
    _in_opt_ int               pid
    )
{
    list_parameters parameters;

    parameters.type = type;
    parameters.pid  = pid;
    return rx_initialize_handle((rx_init_fn)snap_create, snap_destroy, &parameters, sizeof(struct _rx_handle));
}

extern int snprintf ( char * s, size_t n, const char * format, ... );
extern int atoi(const char *nptr);

rx_bool
rx_next_process(
    _in_     rx_handle         snapshot,
    _out_    PRX_PROCESS_ENTRY entry
    )
{
    struct dirent *a0;
    ssize_t       a1;

    while ((a0 = readdir(CDIR(snapshot)))) {
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
read_line(rx_handle snapshot, char *buffer, size_t length)
{
    size_t pos;

    pos = 0;
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
    char *a0;

    while (read_line(snapshot, entry->data, sizeof(entry->data))) {
        entry->path  = strchr(entry->data, '/');
        entry->name  = strrchr(entry->data, '/');
        a0           = entry->data;
        entry->start = strtoul(entry->data, (char**)&a0, 16);
        entry->end   = strtoul(a0+1, (char**)&a0, 16);
        entry->pid   = CPID(snapshot);
        if (entry->path == 0 || entry->name++ == 0 || entry->end < entry->start)
            continue;
        return 1;
    }
    return 0;
}

extern int open(const char *path, int oflag, ... );

static int
snap_create(rx_handle snapshot, list_parameters *parameters)
{
    char a0[17];

    CTYPE(snapshot) = parameters->type;
    switch (CTYPE(snapshot)) {
        case RX_SNAP_TYPE_PROCESS:
            if ((CDIR(snapshot) = opendir("/proc/")) == 0)
                return -1;
            break;
        case RX_SNAP_TYPE_LIBRARY:
            snprintf(a0, sizeof(a0), "/proc/%d/maps", parameters->pid);
            CFILE(snapshot) = open(a0, RX_READ_ACCESS);
            CPID(snapshot)  = parameters->pid;
            if (CFILE(snapshot) == -1)
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
    switch (CTYPE(snapshot)) {
        case RX_SNAP_TYPE_PROCESS:
            closedir(CDIR(snapshot));
            break;
        case RX_SNAP_TYPE_LIBRARY:
            close(CFILE(snapshot));
            break;
        default:
            break;
    }
}

