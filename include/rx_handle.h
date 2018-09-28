#ifndef RX_HANDLE_H
#define RX_HANDLE_H

#include "rx_types.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum _RX_HANDLE_TYPE {
    RX_HANDLE_TYPE_NONE  = 0,
    RX_HANDLE_TYPE_DIR   = 1,
    RX_HANDLE_TYPE_FILE  = 2,
    RX_HANDLE_TYPE_INPUT = 3
} RX_HANDLE_TYPE ;

typedef struct _rx_handle *rx_handle;

typedef struct {
    void(*close)(rx_handle);
    void *self;
} rx_handle_head ;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // RX_HANDLE_H

