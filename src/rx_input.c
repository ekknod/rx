/*
 * =====================================================================================
 *
 *       Filename:  rx_input.c
 *
 *    Description:  handles keyboard/mouse
 *
 *        Version:  1.0
 *        Created:  27.09.2018 17:00:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  github.com/ekknod 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "../include/rx_input.h"
#include <sys/time.h>
#include <linux/types.h>
#include <linux/input-event-codes.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>


extern int snprintf ( char * s, size_t n, const char * format, ... );

struct input_event {
    struct timeval time;
    __u16 type, code;
    __s32 value;
} ;

struct input_parameters {
    RX_INPUT_TYPE type;
    RX_INPUT_MODE mode;
} ;

struct rx_input {
    int       fd;
    pthread_t thread;
    rx_bool   keys[RX_KEYCODE_LAST];
    vec2_i    axis;
} ;

static ssize_t
send_input(rx_handle input, __u16 type, __u16 code, __s32 value);

static int
open_input(rx_handle, void*);

static void
close_input(rx_handle);

rx_handle
rx_open_input(
    _in_  RX_INPUT_TYPE type,
    _in_  RX_INPUT_MODE mode
    )
{
    struct input_parameters parameters;

    parameters.type = type;
    parameters.mode = mode;
    return rx_initialize_object(open_input, close_input, &parameters, sizeof(struct rx_input));
}

rx_bool
rx_key_down(
    _in_  rx_handle      input,
    _in_  RX_KEYCODE     key
    )
{
    struct rx_input *self = input;
    return self->keys[key];
}

vec2_i
rx_input_axis(
    _in_  rx_handle      mouse_input
    )
{
    struct rx_input *self = mouse_input;
    return self->axis;
}

void
rx_send_input_axis(
    _in_  rx_handle      mouse_input,
    _in_  RX_MOUSE_AXIS  axis,
    _in_  int            px
    )
{
    send_input(mouse_input, EV_REL, axis, px);
}

void
rx_send_input_key(
    _in_  rx_handle      input,
    _in_  RX_KEYCODE     key,
    _in_  rx_bool        down
    )
{
    send_input(input, EV_KEY, key, (__s32)down);
}

static ssize_t
send_input(rx_handle input, __u16 type, __u16 code, __s32 value)
{
    struct rx_input     *self = input;
    struct input_event  start, end;
    ssize_t             wfix;

    gettimeofday(&start.time, 0);
    start.type  = type;
    start.code  = code;
    start.value = value;
    
    gettimeofday(&end.time, 0);
    end.type  = EV_SYN;
    end.code  = SYN_REPORT;
    end.value = 0;
    wfix = write(self->fd, &start, sizeof(start));
    wfix = write(self->fd, &end, sizeof(end));
    return wfix;
}

static size_t
next_event(int fd, char *buffer, size_t length)
{
    char   b[2];
    size_t len;

    while (read(fd, b, sizeof(b))) {
        if (b[0] != 'H' || b[1] != ':') {
            while (read(fd, &b[0], 1) != -1) {
                if (b[0] == '\n') {
                    return next_event(fd, buffer, length);
                }
            }
            break;
        }
        lseek(fd, 1, SEEK_CUR);
        len = 0;
        while (--length > 0 && read(fd, &buffer[len], 1)) {
            if (buffer[len] == '\n') {
                buffer[len] = '\0';
                return len;
            }
            len++;
        }
    }
    return 0;
}

static int
open_device(const char *name, int access_mask)
{
    int  dev = -1;
    int  fd  = open("/proc/bus/input/devices", O_RDONLY);
    char b[260], *p, *t;

    while (next_event(fd, b, sizeof(b))) {
        if ((p = strchr(b, '=')) == (char*)0)
            break;
        p += 1;
        if ( (t = strchr(p, ' ')) == 0)
            break;
        *t = '\0';
        if (strcmp(p, name) == 0) {
            if ((p = strrchr(t + 1, ' ')) == 0)
                break;
            *p = '\0';
            p = strchr(t + 1, 'v') - 1;
            snprintf(b, 260, "/dev/input/%s", p);
            dev = open(b, access_mask);
            break;
        }
    }
    close(fd);
    return dev;
}

static void *
mouse_thread(void *input)
{
    struct rx_input    *self = input;
    struct input_event event;

    while (1) {
        if (read(self->fd, &event, sizeof(event)) == -1)
            continue;
        switch (event.type) {
            case EV_REL:
                ((int*)(&self->axis))[event.code] = event.value;
                break;
            case EV_KEY:
                self->keys[event.code] = (rx_bool)event.value;
                break;
            default:
                break;
        }
    }
    return 0;
}

static void *
keyboard_thread(void *input)
{
    struct rx_input    *self = input;
    struct input_event event;

    while ( 1 ) {
        if (read(self->fd, &event, sizeof(event)) == -1)
            continue;
        if (event.type == EV_KEY) {
            self->keys[event.code] = (rx_bool)event.value;
        }
    }
    return 0;
}

static int
open_input(rx_handle input, void *parameters)
{
    struct rx_input         *self   = input;
    struct input_parameters *params = parameters;
    const char              *name;
    static void             *(*thread)(void *);

    switch (params->type) {
        case RX_INPUT_TYPE_MOUSE:
            name = "mouse0";
            thread = mouse_thread;
            break;
        case RX_INPUT_TYPE_KEYBOARD:
            name = "sysrq";
            thread = keyboard_thread;
            break;
        default:
            return -1;
    }

    switch (params->mode) {
        case RX_INPUT_MODE_RECEIVE:
        case RX_INPUT_MODE_ALL:
            self->fd = open_device(name, params->mode);
            if (self->fd == -1) {
                return -1;
            }
            pthread_create(&self->thread, 0, thread, input);
            break;
        case RX_INPUT_MODE_SEND:
            self->fd = open_device(name, params->mode);
            if (self->fd == -1)
                return -1;
            self->thread = 0;
            break;
        default:
            return -1;
    }
    memset(self->keys, 0, sizeof(self->keys));
    memset(&self->axis, 0, sizeof(self->axis));
    return 0;
}

static void
close_input(rx_handle input)
{
    struct rx_input *self = input;
    if (self->thread) {
        pthread_cancel(self->thread);
    }
    close(self->fd);
}

