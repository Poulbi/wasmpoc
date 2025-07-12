#!/bin/sh
printf 'graphics.c\n'
clang \
    --target=wasm32 \
    -O2 \
    -nostdlib \
    -Wl,--no-entry \
    -Wl,--export-all \
    -o graphics.wasm \
    graphics.c
