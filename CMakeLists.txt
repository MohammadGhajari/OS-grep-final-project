cmake_minimum_required(VERSION 3.27)
project(OS_grep_final_project)

set(CMAKE_CXX_STANDARD 14)

add_executable(OS_grep_final_project
        node_modules/node-gyp/gyp/data/win/large-pdb-shim.cc
        node_modules/node-gyp/src/win_delay_load_hook.cc
        main.c)
