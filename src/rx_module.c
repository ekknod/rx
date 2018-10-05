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

#include "../include/rx_process.h"
#include <string.h>

uintptr_t rx_process_map_address(
    _in_      rx_handle        process
    ) ;

extern struct rtld_global *_rtld_global;
static __ssize_t read_ptr(rx_handle handle, uintptr_t address, void *buffer, size_t length);

uintptr_t rx_current_module(void)
{
    return (uintptr_t)_rtld_global;
}

int rx_module_count(void)
{
    return *(int*)(&_rtld_global + sizeof(void*));
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
    int len = rx_wow64_process(process) ? 4 : 8;

    rx_read_process(process, module, &module, len);
    return module;
}

const char *rx_module_path(
    _in_     uintptr_t         module
    )
{
    return *(const char **)(module + sizeof(void*));
}

LONG_STRING rx_module_path_ex(
    _in_     rx_handle         process,
    _in_     uintptr_t         module
    )
{
    LONG_STRING v;
    read_ptr(process, module + sizeof(void*), &v, sizeof(v));
    return v;
}

static const char *nfp(const char *p)
{
    const char *n = strrchr(p, '/');
    return n == 0 ? p : n + 1;
}

uintptr_t rx_find_module(
    _in_     const char        *name
    )
{
    uintptr_t a0 = rx_current_module();
    while ((a0 = *(uintptr_t*)(a0 + (sizeof(void*)*3)))) {
        if (strcmp(nfp(rx_module_path(a0)), name) == 0) {
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
    uintptr_t   map    = rx_process_map_address(process), temp = 0;
    int         offset = rx_wow64_process(process) ? 0x0C : 0x18;
    int         length = rx_wow64_process(process) ? 0x04 : 0x08;
    LONG_STRING path;

    while (rx_read_process(process, map + offset, &map, length) != -1) {
        if (rx_read_process(process, map + length, &temp, length) == -1)
            continue;
        if (rx_read_process(process, temp, &path, sizeof(path)) == -1)
            continue;
        if (strcmp(nfp(path.value), name) == 0) {
            return map;
        }
    }
    return 0;
}

#ifdef __x86_64__
#define OFFSET 0x40
#define ADD    0x18
#define LENGTH 8
#else
#define OFFSET 0x20
#define ADD    0x10
#define LENGTH 4
#endif

uintptr_t rx_find_export(
    _in_     uintptr_t         module,
    _in_     const char        *name
    )
{

    uintptr_t a0;
    uintptr_t a1;
    uint32_t  a2;


    if (module == 0)
        return 0;

    a0 = *(uintptr_t*)(*(uintptr_t*)(module + OFFSET + 5 * LENGTH) + LENGTH);
    a1 = *(uintptr_t*)(*(uintptr_t*)(module + OFFSET + 6 * LENGTH) + LENGTH) + ADD;
    a2 = 1;
    do {
        if (strcmp((const char*)(a0 + a2), name) == 0) {
            return rx_module_base(module) + *(uintptr_t*)(a1 + LENGTH);
        }
        a1 += ADD;
    } while ( (a2 = *(uint32_t*)(a1)) ) ;
    return 0;
}

uintptr_t rx_find_export_ex(
    _in_     rx_handle         process,
    _in_     uintptr_t         module,
    _in_     const char        *name
    )
{
    int          offset  = rx_wow64_process(process) ? 0x20 : 0x40;
    int          add     = rx_wow64_process(process) ? 0x10 : 0x18;
    int          length  = rx_wow64_process(process) ? 0x04 : 0x08;
    uintptr_t    str_tab = 0;
    uintptr_t    sym_tab = 0;
    uint32_t     st_name = 1;
    SHORT_STRING sym_name;


    if (module == 0)
        return 0;

    rx_read_process(process, module + offset + 5 * length, &str_tab, length);
    rx_read_process(process, str_tab + length, &str_tab, length);
    
    rx_read_process(process, module + offset + 6 * length, &sym_tab, length);
    rx_read_process(process, sym_tab + length, &sym_tab, length);

    sym_tab += add;
    do {
        if (rx_read_process(process, str_tab + st_name, &sym_name, sizeof(sym_name)) == -1)
            break;
        if (strcmp(sym_name.value, name) == 0) {
            rx_read_process(process, sym_tab + length, &sym_tab, length);
            rx_read_process(process, module, &module, length);
            return sym_tab + module;
        }
        sym_tab += add;
    } while (rx_read_process(process, sym_tab, &st_name, sizeof(st_name)) != -1) ;
    return 0;
}

static __ssize_t read_ptr(rx_handle handle, uintptr_t address, void *buffer, size_t length)
{
    uintptr_t a0;
    rx_read_process(handle, address, &a0, sizeof(a0));
    return rx_read_process(handle, a0, buffer, length);
}

