/*
 * =====================================================================================
 *
 *       Filename:  rx_handle.c
 *
 *    Description:  cleanup
 *
 *        Version:  1.0
 *        Created:  27.09.2018 16:25:44
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  github.com/ekknod
 *   Organization:  
 *
 * =====================================================================================
 */

#include "../include/rx_handle.h"

typedef struct _rx_handle {
    rx_handle_head head;
} *rx_handle ;

void rx_close_handle(
    _in_     rx_handle         handle
    )
{
    if (handle != 0) {
        handle->head.close(handle->head.self);
    }
}

