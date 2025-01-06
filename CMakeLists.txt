cmake_minimum_required(VERSION 3.27)
project(progetto_SO C)

set(CMAKE_C_STANDARD 99)

add_executable(progetto_SO main.c
        prato_tane.c
        prato_tane.h
        schermo.c
        schermo.h
        regole_gioco.c
        regole_gioco.h
        npc_rana.c
        npc_rana.h)
