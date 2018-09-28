/*
 * =====================================================================================
 *
 *       Filename:  rx_input.c
 *
 *    Description:  handles kbd/mouse input
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
#include "../include/rx_handle.h"
#include <sys/time.h>
#include <linux/types.h>
#include <linux/input-event-codes.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

static ssize_t send_input(rx_handle input, __u16 type, __u16 code, __s32 value);

struct input_event {
    struct timeval time;
    __u16 type, code;
    __s32 value;
} ;

typedef struct _rx_handle {
    rx_handle_head     head;
    int                value[2];
    pthread_t          thread;
    rx_bool            buttons[7];
    vec2_i             axis;
} *rx_handle ;

extern int snprintf ( char * s, size_t n, const char * format, ... );
extern void *malloc(size_t);
extern void free(void *);

static rx_handle open_input(const char *name, RX_INPUT_MODE mode);

rx_handle rx_open_input(
    _in_  RX_INPUT_TYPE type,
    _in_  RX_INPUT_MODE mode
    )
{
    rx_handle v;

    switch (type) {
        case RX_INPUT_TYPE_MOUSE:
            v = open_input("mouse0", mode);
            break;
        case RX_INPUT_TYPE_KEYBOARD:
            // v = open_input("kbd", mode); // not implemented yet
            v = 0;
            break;
        default:
            v = 0;
            break;
    } ;
    return v;
}

rx_bool rx_button_down(
    _in_  rx_handle      mouse_input,
    _in_  RX_BUTTON_CODE button
    )
{
    return mouse_input->buttons[button];
}

vec2_i rx_mouse_axis(
    _in_  rx_handle      mouse_input
    )
{
    return mouse_input->axis;
}

void rx_send_input_axis(
    _in_  rx_handle      mouse_input,
    _in_  RX_MOUSE_AXIS  axis,
    _in_  int            px
    )
{
    send_input(mouse_input, EV_REL, axis, px);
}

void rx_send_input_button(
    _in_  rx_handle      mouse_input,
    _in_  RX_BUTTON_CODE button,
    _in_  rx_bool        down
    )
{
    send_input(mouse_input, EV_KEY, button + 0x110, (__s32)down);
}

static ssize_t send_input(rx_handle input, __u16 type, __u16 code, __s32 value)
{
    struct  input_event start, end;
    ssize_t warning_fix;

    gettimeofday(&start.time, 0);
    start.type  = type;
    start.code  = code;
    start.value = value;
    
    gettimeofday(&end.time, 0);
    end.type  = EV_SYN;
    end.code  = SYN_REPORT;
    end.value = 0;
    warning_fix = write(input->value[0], &start, sizeof(start));
    warning_fix = write(input->value[0], &end, sizeof(end));
    return warning_fix;
}

static size_t next_event(int fd, char *buffer, size_t length)
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

static int open_device(const char *name, int access_mask)
{
    int d, f; char b[260], *p, *t;

    d = -1;
    f = open("/proc/bus/input/devices", O_RDONLY);
    while (next_event(f, b, sizeof(b))) {
        if ((p = strchr(b, '=')) == (char*)0)
            break;
        p += 1;
        if ( (t = strchr(p, ' ')) == 0)
            break;
        *t = '\0';
        if (strcmp(p, name) == 0) {
            if ((p = strchr(t + 1, ' ')) == 0)
                break;
            *p = '\0';
            snprintf(b, 260, "/dev/input/%s", t + 1);
            d = open(b, access_mask);
            break;
        }
    }
    close(f);
    return d;
}

static void close_handle(rx_handle handle)
{
    if (handle->value[1] == RX_INPUT_MODE_RECEIVE || handle->value[1] == RX_INPUT_MODE_ALL) {
        pthread_cancel(handle->thread);
    }
    close(handle->value[0]);
    free(handle);
}

static void *update_thread(void *input)
{
    struct input_event event;
    rx_handle          self;

    self = input;
    while (1) {
        if (read(self->value[0], &event, sizeof(event)) == -1)
            continue;
        switch (event.type) {
            case EV_REL:
                ((int*)(&self->axis))[event.code] = event.value;
                break;
            case EV_KEY:
                self->buttons[event.code - 0x110] = (rx_bool)event.value;
                break;
            default:
                break;
        }
    }
    return 0;
}

static rx_handle open_input(const char *name, RX_INPUT_MODE mode)
{
    rx_handle v;

    v             = malloc(sizeof(struct _rx_handle));
    v->head.close = close_handle;
    v->head.self  = v;
    v->value[0]   = open_device(name, mode);
    v->value[1]   = mode;

    if (v->value[0] == -1) {
        free(v);
        return 0;
    }
    if (mode == RX_INPUT_MODE_RECEIVE || mode == RX_INPUT_MODE_ALL) {
        pthread_create(&v->thread, 0, update_thread, v);
    }
    memset(v->buttons, 0, sizeof(v->buttons));
    memset(&v->axis, 0, sizeof(v->axis));
    return v;
}

