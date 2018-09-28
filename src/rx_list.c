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

typedef struct _rx_handle {
    rx_handle_head head;
    char           value[8];
} *rx_handle ;

extern int snprintf ( char * s, size_t n, const char * format, ... );
extern int atoi(const char *nptr);
extern unsigned long int strtoul (const char* str, char** endptr, int base);
extern void *malloc(size_t);
extern void free(void*);
extern int open(const char *path, int oflag, ... );

#define PID_MAX 32768

static rx_handle open_process_list(void);
static rx_handle open_module_list(int);

rx_handle rx_create_snapshot(
    _in_     RX_SNAP_TYPE      type,
    _in_opt_ int               pid
    )
{
    rx_handle s;

    switch (type) {
        case RX_SNAP_TYPE_PROCESS:
            s = open_process_list();
            break;
        case RX_SNAP_TYPE_LIBRARY:
            s = open_module_list(pid);
            break;
        default:
            s = 0;
    }
    return s;
}

rx_bool rx_next_process(
    _in_     rx_handle         snapshot,
    _out_    PRX_PROCESS_ENTRY entry
    )
{
    struct dirent *a0;
    ssize_t       a1;

    while ((a0 = readdir( *(DIR**)&snapshot->value ))) {
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

static size_t read_line(rx_handle snapshot, char *buffer, size_t length) ;

rx_bool rx_next_library(
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
        entry->pid   = snapshot->value[1];
        if (entry->path == 0 || entry->name++ == 0 || entry->end < entry->start)
            continue;
        return 1;
    }
    return 0;
}

static size_t read_line(rx_handle snapshot, char *buffer, size_t length)
{
    size_t pos;

    pos = 0;
    while (--length > 0 && read(((int*)&snapshot->value)[0], &buffer[pos], 1)) {
        if (buffer[pos] == '\n') {
            buffer[pos] = '\0';
            return pos;
        }
        pos++;
    }
    return 0;
}

static int open_proc_maps(int pid)
{
    char a0[17];
    snprintf(a0, sizeof(a0), "/proc/%d/maps", pid);
    return open(a0, RX_READ_ACCESS);
}

static void close_phandle(rx_handle snapshot)
{
    closedir(*(DIR**)&snapshot->value );
    free(snapshot);
}

static void close_mhandle(rx_handle snapshot)
{
    close(((int*)&snapshot->value)[0]);
    free(snapshot);
}

static rx_handle open_process_list(void)
{
    rx_handle a0;

    a0                    = malloc(sizeof(struct _rx_handle));
    a0->head.close        = close_phandle;
    a0->head.self         = a0;
    *(DIR**)&a0->value[0] = opendir("/proc/");
    return a0;
}

static rx_handle open_module_list(int pid)
{
    rx_handle a0;

    a0                    = malloc(sizeof(struct _rx_handle));
    a0->head.close        = close_mhandle;
    a0->head.self         = a0;
    ((int*)&a0->value)[0] = open_proc_maps(pid);
    ((int*)&a0->value)[1] = pid;
    if (((int*)&a0->value)[0] == -1) {
        free(a0);
        return 0;
    }
    return a0;
}

