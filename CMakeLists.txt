cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)


add_custom_command(
    OUTPUT background.bin background_pal.bin
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/img/png2bin.py
        bitmap 4 ${CMAKE_CURRENT_SOURCE_DIR}/img/background.png
        background.bin background_pal.bin
    DEPENDS img/png2bin.py img/background.png
    VERBATIM
)

add_custom_command(
    OUTPUT crosshair.bin crosshair_pal.bin
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/img/png2bin.py
        sprite 2 32x32 ${CMAKE_CURRENT_SOURCE_DIR}/img/crosshair.png
        crosshair.bin crosshair_pal.bin
    DEPENDS img/png2bin.py img/crosshair.png
    VERBATIM
)

add_custom_command(
    OUTPUT target.bin target_pal.bin
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/img/png2bin.py
        sprite 4 16x16 ${CMAKE_CURRENT_SOURCE_DIR}/img/target.png
        target.bin target_pal.bin
    DEPENDS img/png2bin.py img/target.png
    VERBATIM
)

add_custom_command(
    OUTPUT target_boom.bin target_boom_pal.bin
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/img/png2bin.py
        sprite 1 16x16 ${CMAKE_CURRENT_SOURCE_DIR}/img/target_boom.png
        target_boom.bin target_boom_pal.bin
    DEPENDS img/png2bin.py img/target_boom.png
    VERBATIM
)

add_custom_command(
    OUTPUT game_over.bin game_over_pal.bin
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/img/png2bin.py
        bitmap 4 ${CMAKE_CURRENT_SOURCE_DIR}/img/game_over.png
        game_over.bin game_over_pal.bin
    DEPENDS img/png2bin.py img/game_over.png
    VERBATIM
)

if(RP6502_BASIC)
    project(MY-RP6502-PROJECT BASIC)
    add_executable(hello)
    rp6502_asset(hello help src/help.txt)
    rp6502_asset(hello hello.bas src/main.bas)
    rp6502_basic(hello hello.bas)
    rp6502_web(hello)
else()
    project(MY-RP6502-PROJECT C CXX ASM)
    add_executable(hello)
    rp6502_map(hello src/xram.h "XRAM_.*")
    rp6502_asset(hello help src/help.txt)
    rp6502_asset(hello XRAM(XRAM_BACKGROUND_PALETTE) ${CMAKE_CURRENT_BINARY_DIR}/background_pal.bin)
    rp6502_asset(hello XRAM(XRAM_BACKGROUND) ${CMAKE_CURRENT_BINARY_DIR}/background.bin)
    rp6502_asset(hello XRAM(XRAM_CROSSHAIR_PALETTE) ${CMAKE_CURRENT_BINARY_DIR}/crosshair_pal.bin)
    rp6502_asset(hello XRAM(XRAM_CROSSHAIR) ${CMAKE_CURRENT_BINARY_DIR}/crosshair.bin)
    rp6502_asset(hello XRAM(XRAM_TARGET_PALETTE) ${CMAKE_CURRENT_BINARY_DIR}/target_pal.bin)
    rp6502_asset(hello XRAM(XRAM_TARGET) ${CMAKE_CURRENT_BINARY_DIR}/target.bin)
    rp6502_asset(hello XRAM(XRAM_TARGET_BOOM_PALETTE) ${CMAKE_CURRENT_BINARY_DIR}/target_boom_pal.bin)
    rp6502_asset(hello XRAM(XRAM_TARGET_BOOM) ${CMAKE_CURRENT_BINARY_DIR}/target_boom.bin)
    rp6502_asset(hello XRAM(XRAM_GAME_OVER_PALETTE) ${CMAKE_CURRENT_BINARY_DIR}/game_over_pal.bin)
    rp6502_asset(hello XRAM(XRAM_GAME_OVER) ${CMAKE_CURRENT_BINARY_DIR}/game_over.bin)
    rp6502_executable(hello DATA default RESET default)
    rp6502_web(hello)
    target_sources(hello PRIVATE
        src/main.c
    )
endif()
