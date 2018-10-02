#ifndef RX_HANDLE_H
#define RX_HANDLE_H

#include "rx_types.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
typedef __SIZE_TYPE__ size_t;
typedef struct _rx_handle *rx_handle;

void
rx_close_handle(
    _in_  rx_handle handle
    ) ;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RX_HANDLE_H

