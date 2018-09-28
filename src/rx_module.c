/*
 * =====================================================================================
 *
 *       Filename:  rx_module.c
 *
 *    Description:  library symbols and addresses
 *
 *        Version:  1.0
 *        Created:  27.09.2018 16:32:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  github.com/ekknod 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "../include/rx_types.h"
#include "../include/rx_handle.h"
#include "../include/rx_process.h"
#include <string.h>

typedef struct _rx_handle {
    rx_handle_head head;
    int            value[2];
    char           dir[17];
    uintptr_t      map;
} *rx_handle ;

extern struct rtld_global *_rtld_global;

static const char *nfp(const char *p);
static __ssize_t read_ptr(rx_handle handle, uintptr_t address, void *buffer, size_t length);

uintptr_t rx_current_module(void)
{
    return (uintptr_t)_rtld_global;
}

int rx_module_count(void)
{
    return *(int*)(&_rtld_global + 0x8);
}

uintptr_t rx_module_base(
    _in_     uintptr_t         module
    )
{
    return *(uintptr_t*)module;
}

uintptr_t rx_module_base_ex(
    _in_     rx_handle         process,
    _in_     uintptr_t         module
    )
{
    rx_read_process(process, module, &module, sizeof(module));
    return module;
}

const char *rx_module_path(
    _in_     uintptr_t         module
    )
{
    return *(const char **)(module + 0x8);
}

LONG_STRING rx_module_path_ex(
    _in_     rx_handle         process,
    _in_     uintptr_t         module
    )
{
    LONG_STRING v;
    read_ptr(process, module + 0x8, &v, sizeof(v));
    return v;
}

uintptr_t rx_find_module(
    _in_     const char        *name
    )
{
    uintptr_t a0;

    a0 = rx_current_module();
    while ((a0 = *(uintptr_t*)(a0 + 0x18))) {
        if (strcmp(nfp(*(const char**)(a0 + 8)), name) == 0) {
            return a0;
        }
    }
    return 0;
}

uintptr_t rx_find_module_ex(
    _in_     rx_handle         process,
    _in_     const char        *name
    )
{
    uintptr_t   a0;
    LONG_STRING a1;

    a0 = process->map;
    while (rx_read_process(process, a0 + 0x18, &a0, sizeof(a0)) != -1) {
        if (read_ptr(process, a0 + 0x08, &a1, sizeof(a1)) == -1)
            break;
        if (strcmp(nfp(a1.value), name) == 0) {
            return a0;
        }
    }
    return 0;
}

uintptr_t rx_find_export(
    _in_     uintptr_t         module,
    _in_     const char        *name
    )
{
    uintptr_t a0, a1;
    uint32_t  a2;


    if (module == 0)
        module = rx_current_module();

    a0 = *(uintptr_t*)(*(uintptr_t*)(module + 0x40 + 5 * 8) + 0x8);
    a1 = *(uintptr_t*)(*(uintptr_t*)(module + 0x40 + 6 * 8) + 0x8) + 0x18;
    a2 = 1;
    do {
        if (strcmp((const char*)(a0 + a2), name) == 0) {
            return rx_module_base(module) + *(uintptr_t*)(a1 + 8);
        }
        a1 += 0x18;
    } while ( (a2 = *(uint32_t*)(a1)) ) ;
    return 0;
}

uintptr_t rx_find_export_ex(
    _in_     rx_handle         process,
    _in_     uintptr_t         module,
    _in_     const char        *name
    )
{
    uintptr_t    a0, a1;
    uint32_t     a2;
    SHORT_STRING a3;


    if (module == 0)
        return 0;

    rx_read_process(process, module + 0x40 + 5 * 8, &a0, sizeof(a0));
    rx_read_process(process, module + 0x40 + 6 * 8, &a1, sizeof(a1));

    rx_read_process(process, a0 + 0x8, &a0, sizeof(a0));
    rx_read_process(process, a1 + 0x8, &a1, sizeof(a1));
    a1 += 0x18;
    a2 = 1;
    do {
        if (rx_read_process(process, a0 + a2, &a3, sizeof(a3)) == -1)
            break;
        if (strcmp(a3.value, name) == 0) {
            rx_read_process(process, a1 + 0x8, &a1, sizeof(a1));
            rx_read_process(process, module, &module, sizeof(module));
            return a1 + module;
        }
        a1 += 0x18;
    } while (rx_read_process(process, a1, &a2, sizeof(a2)) != -1);
    return 0;
}

static const char *nfp(const char *p)
{
    const char *n;
    n = strrchr(p, '/');
    return n == 0 ? (const char *)p : n + 1;
}

static __ssize_t read_ptr(rx_handle handle, uintptr_t address, void *buffer, size_t length)
{
    uintptr_t a0;
    rx_read_process(handle, address, &a0, sizeof(a0));
    return rx_read_process(handle, a0, buffer, length);
}

