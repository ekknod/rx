# version define from cmake tutorials =)
cmake_minimum_required(VERSION 2.6)
project(rx)

set(OUT rxlib)

set(LINKTYPE STATIC)

add_library ( ${OUT} ${LINKTYPE}
    src/rx_handle.c src/rx_input.c src/rx_list.c src/rx_module.c src/rx_process.c )

set_target_properties(${OUT} PROPERTIES LINKER_LANGUAGE C)
set(CMAKE_C_FLAGS "-Wno-format-truncation")

